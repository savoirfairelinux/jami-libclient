/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

//Parent
#include "call.h"

//C include
#include <time.h>

//Qt
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>

//DRing
#include <account_const.h>
#include <call_const.h>

//Ring library
#include "dbus/callmanager.h"

#include "collectioninterface.h"
#include "person.h"
#include "uri.h"
#include "account.h"
#include "accountmodel.h"
#include "availableaccountmodel.h"
#include "video/manager.h"
#include "historymodel.h"
#include "instantmessagingmodel.h"
#include "useractionmodel.h"
#include "callmodel.h"
#include "certificate.h"
#include "numbercategory.h"
#include "certificatemodel.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"
#include "video/renderer.h"
#include "tlsmethodmodel.h"
#include "audio/settings.h"
#include "personmodel.h"
#include "imconversationmanager.h"

//Track where state changes are performed on finished (over, error, failed) calls
//while not really problematic, it is technically wrong
#define Q_ASSERT_IS_IN_PROGRESS Q_ASSERT(m_CurrentState != Call::State::OVER);
#define FORCE_ERROR_STATE() {qDebug() << "Fatal error on " << this << __FILE__ << __LINE__;\
   d_ptr->changeCurrentState(Call::State::ERROR);}

#define FORCE_ERROR_STATE_P() {qDebug() << "Fatal error on " << this << __FILE__ << __LINE__;\
   changeCurrentState(Call::State::ERROR);}

#include "private/call_p.h"
#include "private/instantmessagingmodel_p.h"

const TypedStateMachine< TypedStateMachine< Call::State , Call::Action> , Call::State> CallPrivate::actionPerformedStateMap =
{{
//                           ACCEPT                      REFUSE                  TRANSFER                       HOLD                           RECORD              /**/
/*INCOMING     */  {{Call::State::INCOMING      , Call::State::INCOMING    , Call::State::ERROR        , Call::State::INCOMING     ,  Call::State::INCOMING     }},/**/
/*RINGING      */  {{Call::State::ERROR         , Call::State::RINGING     , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::RINGING      }},/**/
/*CURRENT      */  {{Call::State::ERROR         , Call::State::CURRENT     , Call::State::TRANSFERRED  , Call::State::CURRENT      ,  Call::State::CURRENT      }},/**/
/*DIALING      */  {{Call::State::INITIALIZATION, Call::State::OVER        , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
/*HOLD         */  {{Call::State::ERROR         , Call::State::HOLD        , Call::State::TRANSF_HOLD  , Call::State::HOLD         ,  Call::State::HOLD         }},/**/
/*FAILURE      */  {{Call::State::ERROR         , Call::State::OVER        , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
/*BUSY         */  {{Call::State::ERROR         , Call::State::BUSY        , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
/*TRANSFER     */  {{Call::State::TRANSFERRED   , Call::State::TRANSFERRED , Call::State::CURRENT      , Call::State::TRANSFERRED  ,  Call::State::TRANSFERRED  }},/**/
/*TRANSF_HOLD  */  {{Call::State::TRANSF_HOLD   , Call::State::TRANSF_HOLD , Call::State::HOLD         , Call::State::TRANSF_HOLD  ,  Call::State::TRANSF_HOLD  }},/**/
/*OVER         */  {{Call::State::ERROR         , Call::State::ERROR       , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
/*ERROR        */  {{Call::State::ERROR         , Call::State::ERROR       , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
/*CONF         */  {{Call::State::ERROR         , Call::State::CURRENT     , Call::State::TRANSFERRED  , Call::State::CURRENT      ,  Call::State::CURRENT      }},/**/
/*CONF_HOLD    */  {{Call::State::ERROR         , Call::State::HOLD        , Call::State::TRANSF_HOLD  , Call::State::HOLD         ,  Call::State::HOLD         }},/**/
/*INIT         */  {{Call::State::INITIALIZATION, Call::State::OVER        , Call::State::ERROR        , Call::State::ERROR        ,  Call::State::ERROR        }},/**/
}};//                                                                                                                                                                */

#define CP &CallPrivate
const TypedStateMachine< TypedStateMachine< function , Call::Action > , Call::State > CallPrivate::actionPerformedFunctionMap =
{{
//                      ACCEPT             REFUSE         TRANSFER             HOLD               RECORD            /**/
/*INCOMING       */  {{CP::accept     , CP::refuse   , CP::acceptTransf   , CP::acceptHold  ,  CP::toggleRecord  }},/**/
/*RINGING        */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::nothing     ,  CP::toggleRecord  }},/**/
/*CURRENT        */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::hold        ,  CP::toggleRecord  }},/**/
/*DIALING        */  {{CP::call       , CP::cancel   , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
/*HOLD           */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::unhold      ,  CP::toggleRecord  }},/**/
/*FAILURE        */  {{CP::nothing    , CP::remove   , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
/*BUSY           */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
/*TRANSFERT      */  {{CP::transfer   , CP::hangUp   , CP::transfer       , CP::hold        ,  CP::toggleRecord  }},/**/
/*TRANSFERT_HOLD */  {{CP::transfer   , CP::hangUp   , CP::transfer       , CP::unhold      ,  CP::toggleRecord  }},/**/
/*OVER           */  {{CP::nothing    , CP::nothing  , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
/*ERROR          */  {{CP::nothing    , CP::remove   , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
/*CONF           */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::hold        ,  CP::toggleRecord  }},/**/
/*CONF_HOLD      */  {{CP::nothing    , CP::hangUp   , CP::nothing        , CP::unhold      ,  CP::toggleRecord  }},/**/
/*INITIALIZATION */  {{CP::call       , CP::cancel   , CP::nothing        , CP::nothing     ,  CP::nothing       }},/**/
}};//                                                                                                                 */


const TypedStateMachine< TypedStateMachine< Call::State , CallPrivate::DaemonState> , Call::State> CallPrivate::stateChangedStateMap =
{{
//                        RINGING                   CURRENT                   BUSY                  HOLD                        HUNGUP                 FAILURE           /**/
/*INCOMING     */ {{Call::State::INCOMING    , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*RINGING      */ {{Call::State::RINGING     , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*CURRENT      */ {{Call::State::CURRENT     , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*DIALING      */ {{Call::State::RINGING     , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*HOLD         */ {{Call::State::HOLD        , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*FAILURE      */ {{Call::State::FAILURE     , Call::State::FAILURE    , Call::State::BUSY   , Call::State::FAILURE      ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*BUSY         */ {{Call::State::BUSY        , Call::State::CURRENT    , Call::State::BUSY   , Call::State::BUSY         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*TRANSFER     */ {{Call::State::TRANSFERRED , Call::State::TRANSFERRED, Call::State::BUSY   , Call::State::TRANSF_HOLD  ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*TRANSF_HOLD  */ {{Call::State::TRANSF_HOLD , Call::State::TRANSFERRED, Call::State::BUSY   , Call::State::TRANSF_HOLD  ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*OVER         */ {{Call::State::OVER        , Call::State::OVER       , Call::State::OVER   , Call::State::OVER         ,  Call::State::OVER  ,  Call::State::OVER     }},/**/
/*ERROR        */ {{Call::State::ERROR       , Call::State::ERROR      , Call::State::ERROR  , Call::State::ERROR        ,  Call::State::ERROR ,  Call::State::ERROR    }},/**/
/*CONF         */ {{Call::State::CURRENT     , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*CONF_HOLD    */ {{Call::State::HOLD        , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
/*INIT         */ {{Call::State::RINGING     , Call::State::CURRENT    , Call::State::BUSY   , Call::State::HOLD         ,  Call::State::OVER  ,  Call::State::FAILURE  }},/**/
}};//                                                                                                                                                                        */

const TypedStateMachine< TypedStateMachine< function , CallPrivate::DaemonState > , Call::State > CallPrivate::stateChangedFunctionMap =
{{
//                      RINGING                  CURRENT             BUSY              HOLD                    HUNGUP                  FAILURE       /**/
/*INCOMING       */  {{CP::nothing    , CP::start     , CP::startWeird     , CP::startWeird   ,  CP::startStop    , CP::failure }},/**/
/*RINGING        */  {{CP::nothing    , CP::start     , CP::start          , CP::start        ,  CP::startStop    , CP::failure }},/**/
/*CURRENT        */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*DIALING        */  {{CP::nothing    , CP::warning   , CP::warning        , CP::warning      ,  CP::stop         , CP::warning }},/**/
/*HOLD           */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*FAILURE        */  {{CP::nothing    , CP::warning   , CP::warning        , CP::warning      ,  CP::stop         , CP::nothing }},/**/
/*BUSY           */  {{CP::nothing    , CP::nothing   , CP::nothing        , CP::warning      ,  CP::stop         , CP::nothing }},/**/
/*TRANSFERT      */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*TRANSFERT_HOLD */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*OVER           */  {{CP::nothing    , CP::warning   , CP::warning        , CP::warning      ,  CP::stop         , CP::warning }},/**/
/*ERROR          */  {{CP::error      , CP::error     , CP::error          , CP::error        ,  CP::stop         , CP::error   }},/**/
/*CONF           */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*CONF_HOLD      */  {{CP::nothing    , CP::nothing   , CP::warning        , CP::nothing      ,  CP::stop         , CP::nothing }},/**/
/*INIT           */  {{CP::nothing    , CP::warning   , CP::warning        , CP::warning      ,  CP::stop         , CP::warning }},/**/
}};//                                                                                                                                */
#undef CP

const TypedStateMachine< Call::LifeCycleState , Call::State > CallPrivate::metaStateMap =
{{
/*               *        Life cycle meta-state              **/
/*INCOMING       */   Call::LifeCycleState::INITIALIZATION ,/**/
/*RINGING        */   Call::LifeCycleState::INITIALIZATION ,/**/
/*CURRENT        */   Call::LifeCycleState::PROGRESS       ,/**/
/*DIALING        */   Call::LifeCycleState::INITIALIZATION ,/**/
/*HOLD           */   Call::LifeCycleState::PROGRESS       ,/**/
/*FAILURE        */   Call::LifeCycleState::FINISHED       ,/**/
/*BUSY           */   Call::LifeCycleState::FINISHED       ,/**/
/*TRANSFERT      */   Call::LifeCycleState::PROGRESS       ,/**/
/*TRANSFERT_HOLD */   Call::LifeCycleState::PROGRESS       ,/**/
/*OVER           */   Call::LifeCycleState::FINISHED       ,/**/
/*ERROR          */   Call::LifeCycleState::FINISHED       ,/**/
/*CONF           */   Call::LifeCycleState::PROGRESS       ,/**/
/*CONF_HOLD      */   Call::LifeCycleState::PROGRESS       ,/**/
/*INIT           */   Call::LifeCycleState::INITIALIZATION ,/**/
}};/*                                                        **/

const TypedStateMachine< TypedStateMachine< bool , Call::LifeCycleState > , Call::State > CallPrivate::metaStateTransitionValidationMap =
{{
/*               *     INITIALIZATION    PROGRESS      FINISHED   **/
/*INCOMING       */  {{     true     ,    false    ,    false }},/**/
/*RINGING        */  {{     true     ,    false    ,    false }},/**/
/*CURRENT        */  {{     true     ,    true     ,    false }},/**/
/*DIALING        */  {{     true     ,    false    ,    false }},/**/
/*HOLD           */  {{     true     ,    true     ,    false }},/**/
/*FAILURE        */  {{     true     ,    true     ,    false }},/**/
/*BUSY           */  {{     true     ,    false    ,    false }},/**/
/*TRANSFERT      */  {{     false    ,    true     ,    false }},/**/
/*TRANSFERT_HOLD */  {{     false    ,    true     ,    false }},/**/
/*OVER           */  {{     true     ,    true     ,    true  }},/**/
/*ERROR          */  {{     true     ,    true     ,    false }},/**/
/*CONF           */  {{     true     ,    false    ,    false }},/**/
/*CONF_HOLD      */  {{     true     ,    false    ,    false }},/**/
/*INIT           */  {{     true     ,    false    ,    false }},/**/
}};/*                                                             **/
/*^^ A call _can_ be created on hold (conference) and as over (peer hang up before pickup)
 the progress->failure one is an implementation bug*/

QDebug LIB_EXPORT operator<<(QDebug dbg, const CallPrivate::DaemonState& c );

QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::State& c)
{
   dbg.nospace() << Call::toHumanStateName(c);
   return dbg.space();
}

QDebug LIB_EXPORT operator<<(QDebug dbg, const CallPrivate::DaemonState& c)
{
   dbg.nospace() << static_cast<int>(c);
   return dbg.space();
}

QDebug LIB_EXPORT operator<<(QDebug dbg, const Call::Action& c)
{
   switch (c) {
      case Call::Action::ACCEPT:
         dbg.nospace() << "ACCEPT";
         break;
      case Call::Action::REFUSE:
         dbg.nospace() << "REFUSE";
         break;
      case Call::Action::TRANSFER:
         dbg.nospace() << "TRANSFER";
         break;
      case Call::Action::HOLD:
         dbg.nospace() << "HOLD";
         break;
      case Call::Action::RECORD:
         dbg.nospace() << "RECORD";
         break;
      case Call::Action::COUNT__:
         dbg.nospace() << "COUNT";
         break;
   };
   dbg.space();
   dbg.nospace() << '(' << static_cast<int>(c) << ')';
   return dbg.space();
}

CallPrivate::CallPrivate(Call* parent) : QObject(parent),q_ptr(parent),
m_pStopTimeStamp(0),
m_pImModel(nullptr),m_pTimer(nullptr),m_Recording(false),m_Account(nullptr),
m_PeerName(),m_pPeerContactMethod(nullptr),m_HistoryConst(HistoryTimeCategoryModel::HistoryConst::Never),
m_pStartTimeStamp(0),m_pDialNumber(nullptr),m_pTransferNumber(nullptr),
m_History(false),m_Missed(false),m_Direction(Call::Direction::OUTGOING),m_Type(Call::Type::CALL),
m_pUserActionModel(nullptr), m_CurrentState(Call::State::ERROR),m_pCertificate(nullptr)
{
}

///Constructor
Call::Call(Call::State startState, const QString& peerName, ContactMethod* number, Account* account)
   : ItemBase<QObject>(CallModel::instance()),d_ptr(new CallPrivate(this))
{
   d_ptr->m_CurrentState     = startState;
   d_ptr->m_Type             = Call::Type::CALL;
   d_ptr->m_Account          = account;
   d_ptr->m_PeerName         = peerName;
   d_ptr->m_pPeerContactMethod = number;

   d_ptr->changeCurrentState(startState);

   emit changed();
   emit changed(this);
}

///Constructor
Call::Call(const QString& confId, const QString& account)
   : ItemBase<QObject>(CallModel::instance()),d_ptr(new CallPrivate(this))
{
   d_ptr->m_CurrentState = Call::State::CONFERENCE;
   d_ptr->m_Account      = AccountModel::instance()->getById(account.toLatin1());
   d_ptr->m_Type         = (!confId.isEmpty())?Call::Type::CONFERENCE:Call::Type::CALL;
   d_ptr->m_DringId      = confId;

   setObjectName("Conf:"+confId);

   if (type() == Call::Type::CONFERENCE) {
      time_t curTime;
      ::time(&curTime);
      d_ptr->setStartTimeStamp(curTime);
      d_ptr->initTimer();
      CallManagerInterface& callManager = DBus::CallManager::instance();
      MapStringString        details    = callManager.getConferenceDetails(dringId())  ;
      d_ptr->m_CurrentState             = d_ptr->confStatetoCallState(details[CallPrivate::ConfDetailsMapFields::CONF_STATE]);
      emit stateChanged();
   }
}

///Destructor
Call::~Call()
{
   if (d_ptr->m_pTimer) delete d_ptr->m_pTimer;
   this->disconnect();

   //m_pTransferNumber and m_pDialNumber are temporary, they are owned by the call
   if ( d_ptr->m_pTransferNumber ) delete d_ptr->m_pTransferNumber;
   if ( d_ptr->m_pDialNumber     ) delete d_ptr->m_pDialNumber;

   delete d_ptr;
}

/*****************************************************************************
 *                                                                           *
 *                               Call builder                                *
 *                                                                           *
 ****************************************************************************/

///Build a call from its ID
Call* CallPrivate::buildExistingCall(const QString& callId)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   MapStringString       details     = callManager.getCallDetails(callId);

   //Too noisy
   //qDebug() << "Constructing existing call with details : " << details;

   const QString peerNumber    = details[ CallPrivate::DetailsMapFields::PEER_NUMBER ];
   const QString peerName      = details[ CallPrivate::DetailsMapFields::PEER_NAME   ];
   const QString account       = details[ CallPrivate::DetailsMapFields::ACCOUNT_ID  ];
   Call::State   startState    = startStateFromDaemonCallState(details[CallPrivate::DetailsMapFields::STATE], details[CallPrivate::DetailsMapFields::TYPE]);
   Account*      acc           = AccountModel::instance()->getById(account.toLatin1());
   ContactMethod*  nb            = PhoneDirectoryModel::instance()->getNumber(peerNumber,acc);
   Call*         call          = new Call(startState, peerName, nb, acc);
   call->d_ptr->m_DringId      = callId;
   call->d_ptr->m_Recording    = callManager.getIsRecording(callId);

   if (!details[ CallPrivate::DetailsMapFields::TIMESTAMP_START ].isEmpty())
      call->d_ptr->setStartTimeStamp(details[ CallPrivate::DetailsMapFields::TIMESTAMP_START ].toInt());
   else {
      time_t curTime;
      ::time(&curTime);
      call->d_ptr->setStartTimeStamp(curTime);
   }

   call->d_ptr->initTimer();

   if (call->peerContactMethod()) {
      call->peerContactMethod()->addCall(call);
   }

   //Load the certificate if it's now available
   if (!call->certificate() && !details[DRing::Call::Details::TLS_PEER_CERT].isEmpty()) {
      call->d_ptr->m_pCertificate = CertificateModel::instance()->getCertificateFromContent(details[DRing::Call::Details::TLS_PEER_CERT].toLatin1(),call->account());
   }

   return call;
} //buildExistingCall

///Build a call from a dialing call (a call that is about to exist)
Call* CallPrivate::buildDialingCall(const QString & peerName, Account* account)
{
   Call* call = new Call(Call::State::DIALING, peerName, nullptr, account);
   call->d_ptr->m_Direction = Call::Direction::OUTGOING;
   if (Audio::Settings::instance()->isRoomToneEnabled()) {
      Audio::Settings::instance()->playRoomTone();
   }
   qDebug() << "Created dialing call" << call;
   return call;
}

///Build a call from a dbus event
Call* CallPrivate::buildIncomingCall(const QString& callId)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   MapStringString details = callManager.getCallDetails(callId);

   const QString from          = details[ CallPrivate::DetailsMapFields::PEER_NUMBER ];
   const QString account       = details[ CallPrivate::DetailsMapFields::ACCOUNT_ID  ];
   const QString peerName      = details[ CallPrivate::DetailsMapFields::PEER_NAME   ];
   Account*      acc           = AccountModel::instance()->getById(account.toLatin1());
   ContactMethod*  nb          = PhoneDirectoryModel::instance()->getNumber(from,acc);
   Call* call                  = new Call(Call::State::INCOMING, peerName, nb, acc);
   call->d_ptr->m_DringId      = callId;
   call->d_ptr->m_Direction    = Call::Direction::INCOMING;
   if (call->peerContactMethod()) {
      call->peerContactMethod()->addCall(call);
   }

   //Load the certificate if it's now available
   if (!call->certificate() && !details[DRing::Call::Details::TLS_PEER_CERT].isEmpty()) {
      call->d_ptr->m_pCertificate = CertificateModel::instance()->getCertificateFromContent(details[DRing::Call::Details::TLS_PEER_CERT].toLatin1(),call->account());
   }

   return call;
} //buildIncomingCall

///Build a ringing call (from dbus)
Call* CallPrivate::buildRingingCall(const QString & callId)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   MapStringString details = callManager.getCallDetails(callId);

   const QString from          = details[ CallPrivate::DetailsMapFields::PEER_NUMBER ];
   const QString account       = details[ CallPrivate::DetailsMapFields::ACCOUNT_ID  ];
   const QString peerName      = details[ CallPrivate::DetailsMapFields::PEER_NAME   ];
   Account*      acc           = AccountModel::instance()->getById(account.toLatin1());
   ContactMethod*  nb          = PhoneDirectoryModel::instance()->getNumber(from,acc);
   Call* call                  = new Call(Call::State::RINGING, peerName, nb, acc);
   call->d_ptr->m_DringId      = callId;
   call->d_ptr->m_Direction    = Call::Direction::OUTGOING;

   if (call->peerContactMethod()) {
      call->peerContactMethod()->addCall(call);
   }

   //Load the certificate if it's now available
   if (!call->certificate() && !details[DRing::Call::Details::TLS_PEER_CERT].isEmpty()) {
      call->d_ptr->m_pCertificate = CertificateModel::instance()->getCertificateFromContent(details[DRing::Call::Details::TLS_PEER_CERT].toLatin1(),call->account());
   }

   return call;
} //buildRingingCall


/*****************************************************************************
 *                                                                           *
 *                                  History                                  *
 *                                                                           *
 ****************************************************************************/

///Build a call that is already over
Call* Call::buildHistoryCall(const QMap<QString,QString>& hc)
{
   const QString& callId          = hc[ Call::HistoryMapFields::CALLID          ]          ;
   const QString& name            = hc[ Call::HistoryMapFields::DISPLAY_NAME    ]          ;
   const QString& number          = hc[ Call::HistoryMapFields::PEER_NUMBER     ]          ;
   const QString& type            = hc[ Call::HistoryMapFields::STATE           ]          ;
   const QString& direction       = hc[ Call::HistoryMapFields::DIRECTION       ]          ;
   const QString& cert_path       = hc[ Call::HistoryMapFields::CERT_PATH       ]          ;
   const bool     missed          = hc[ Call::HistoryMapFields::MISSED          ] == "1"   ;
   time_t         startTimeStamp  = hc[ Call::HistoryMapFields::TIMESTAMP_START ].toUInt() ;
   time_t         stopTimeStamp   = hc[ Call::HistoryMapFields::TIMESTAMP_STOP  ].toUInt() ;
   QByteArray accId               = hc[ Call::HistoryMapFields::ACCOUNT_ID      ].toLatin1();

   if (accId.isEmpty()) {
      qWarning() << "An history call has an invalid account identifier";
      accId = DRing::Account::ProtocolNames::IP2IP;
   }

   //Try to assiciate a contact now, the real contact object is probably not
   //loaded yet, but we can get a placeholder for now
//    const QString& contactUsed    = hc[ Call::HistoryMapFields::CONTACT_USED ]; //TODO
   const QString& contactUid     = hc[ Call::HistoryMapFields::CONTACT_UID  ];

   Person* ct = nullptr;
   if (!hc[ Call::HistoryMapFields::CONTACT_UID].isEmpty())
      ct = PersonModel::instance()->getPlaceHolder(contactUid.toLatin1());

   Account*        acc            = AccountModel::instance()->getById(accId);
   ContactMethod*  nb             = PhoneDirectoryModel::instance()->getNumber(number,ct,acc);

   Call*           call           = new Call(Call::State::OVER, (name == "empty")?QString():name, nb, acc );
   call->d_ptr->m_DringId         = callId;

   call->d_ptr->m_pStopTimeStamp  = stopTimeStamp ;
   call->d_ptr->setStartTimeStamp(startTimeStamp);
   call->d_ptr->m_History         = true;
   call->d_ptr->m_Account         = AccountModel::instance()->getById(accId);

   if (missed) {
      call->d_ptr->m_Missed = true;
   }
   if (!direction.isEmpty()) {
      if (direction == Call::HistoryStateName::INCOMING) {
         call->d_ptr->m_Direction    = Call::Direction::INCOMING         ;
      }
      else if (direction == Call::HistoryStateName::OUTGOING) {
         call->d_ptr->m_Direction    = Call::Direction::OUTGOING         ;
      }
   }
   else //Getting there is a bug. Pick one, even if it is the wrong one
      call->d_ptr->m_Direction    = Call::Direction::OUTGOING            ;

   call->setObjectName("History:"+call->d_ptr->m_DringId);

   if (call->peerContactMethod()) {
      call->peerContactMethod()->addCall(call);

      //Reload the glow and number colors
      connect(call->peerContactMethod(),SIGNAL(presentChanged(bool)),call->d_ptr,SLOT(updated()));

      //Change the display name and picture
      connect(call->peerContactMethod(),SIGNAL(rebased(ContactMethod*)),call->d_ptr,SLOT(updated()));
   }

   //Check the certificate
   if (!hc[Call::HistoryMapFields::CERT_PATH].isEmpty()) {
      call->d_ptr->m_pCertificate = CertificateModel::instance()->getCertificate(QUrl(hc[Call::HistoryMapFields::CERT_PATH]),acc);
   }

   return call;
}

/// aCall << Call::Action::HOLD
Call* Call::operator<<( Call::Action& c)
{
   performAction(c);
   return this;
}

Call* operator<<(Call* c, Call::Action action)
{
   return (!c) ? nullptr : (*c) << action;
}

///Get the start sate from the daemon state
Call::State CallPrivate::startStateFromDaemonCallState(const QString& daemonCallState, const QString& daemonCallType)
{
   if(daemonCallState      == CallPrivate::DaemonStateInit::CURRENT  )
      return Call::State::CURRENT  ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::HOLD     )
      return Call::State::HOLD     ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::BUSY     )
      return Call::State::BUSY     ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::INACTIVE && daemonCallType == CallPrivate::CallDirection::INCOMING )
      return Call::State::INCOMING ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::INACTIVE && daemonCallType == CallPrivate::CallDirection::OUTGOING )
      return Call::State::RINGING  ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::INCOMING )
      return Call::State::INCOMING ;
   else if(daemonCallState == CallPrivate::DaemonStateInit::RINGING  )
      return Call::State::RINGING  ;
   else
      return Call::State::FAILURE  ;
} //getStartStateFromDaemonCallState


/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///Transfer state from internal to daemon internal syntaz
CallPrivate::DaemonState CallPrivate::toDaemonCallState(const QString& stateName)
{
   if(stateName == CallPrivate::StateChange::HUNG_UP        )
      return CallPrivate::DaemonState::HUNG_UP ;
   if(stateName == CallPrivate::StateChange::RINGING        )
      return CallPrivate::DaemonState::RINGING ;
   if(stateName == CallPrivate::StateChange::CURRENT        )
      return CallPrivate::DaemonState::CURRENT ;
   if(stateName == CallPrivate::StateChange::UNHOLD_CURRENT )
      return CallPrivate::DaemonState::CURRENT ;
   if(stateName == CallPrivate::StateChange::HOLD           )
      return CallPrivate::DaemonState::HOLD    ;
   if(stateName == CallPrivate::StateChange::BUSY           )
      return CallPrivate::DaemonState::BUSY    ;
   if(stateName == CallPrivate::StateChange::FAILURE        )
      return CallPrivate::DaemonState::FAILURE ;

   qDebug() << "stateChanged signal received with unknown state.";
   return CallPrivate::DaemonState::FAILURE    ;
} //toDaemonCallState

///Transform a conference call state to a proper call state
Call::State CallPrivate::confStatetoCallState(const QString& stateName)
{
   if      ( stateName == CallPrivate::ConferenceStateChange::HOLD   )
      return Call::State::CONFERENCE_HOLD;
   else if ( stateName == CallPrivate::ConferenceStateChange::ACTIVE )
      return Call::State::CONFERENCE;
   else
      return Call::State::ERROR; //Well, this may bug a little
}

///Transform a backend state into a translated string
const QString Call::toHumanStateName(const Call::State cur)
{
   switch (cur) {
      case Call::State::INCOMING:
         return tr( "Ringing (in)"      );
         break;
      case Call::State::RINGING:
         return tr( "Ringing (out)"     );
         break;
      case Call::State::CURRENT:
         return tr( "Talking"           );
         break;
      case Call::State::DIALING:
         return tr( "Dialing"           );
         break;
      case Call::State::HOLD:
         return tr( "Hold"              );
         break;
      case Call::State::FAILURE:
         return tr( "Failed"            );
         break;
      case Call::State::BUSY:
         return tr( "Busy"              );
         break;
      case Call::State::TRANSFERRED:
         return tr( "Transfer"          );
         break;
      case Call::State::TRANSF_HOLD:
         return tr( "Transfer hold"     );
         break;
      case Call::State::OVER:
         return tr( "Over"              );
         break;
      case Call::State::ERROR:
         return tr( "Error"             );
         break;
      case Call::State::CONFERENCE:
         return tr( "Conference"        );
         break;
      case Call::State::CONFERENCE_HOLD:
         return tr( "Conference (hold)" );
      case Call::State::COUNT__:
         return tr( "ERROR"             );
      case Call::State::INITIALIZATION:
         return tr( "Initialization"    );
      default:
         return QString::number(static_cast<int>(cur));
   }
}

QString Call::toHumanStateName() const
{
   return toHumanStateName(state());
}

///Get the time (second from 1 jan 1970) when the call ended
time_t Call::stopTimeStamp() const
{
   return d_ptr->m_pStopTimeStamp;
}

///Get the time (second from 1 jan 1970) when the call started
time_t Call::startTimeStamp() const
{
   return d_ptr->m_pStartTimeStamp;
}

///Get the number where the call have been transferred
const QString Call::transferNumber() const
{
   return d_ptr->m_pTransferNumber?d_ptr->m_pTransferNumber->uri():QString();
}

///Get the call / peer number
const QString Call::dialNumber() const
{
   if (d_ptr->m_CurrentState != Call::State::DIALING) return QString();
   if (!d_ptr->m_pDialNumber) {
      d_ptr->m_pDialNumber = new TemporaryContactMethod();
   }
   return d_ptr->m_pDialNumber->uri();
}

///Return the call id
const QString Call::historyId() const
{
   return d_ptr->m_DringId;
}

///Return the call id
const QString Call::dringId() const
{
   Q_ASSERT(!d_ptr->m_DringId.isEmpty());
   return d_ptr->m_DringId;
}

ContactMethod* Call::peerContactMethod() const
{
   if (d_ptr->m_CurrentState == Call::State::DIALING) {
      if (!d_ptr->m_pTransferNumber) {
         d_ptr->m_pTransferNumber = new TemporaryContactMethod(d_ptr->m_pPeerContactMethod);
      }
      if (!d_ptr->m_pDialNumber)
         d_ptr->m_pDialNumber = new TemporaryContactMethod(d_ptr->m_pPeerContactMethod);
      return d_ptr->m_pDialNumber;
   }
   return d_ptr->m_pPeerContactMethod?d_ptr->m_pPeerContactMethod:const_cast<ContactMethod*>(ContactMethod::BLANK());
}

///Get the peer name
const QString Call::peerName() const
{
   return d_ptr->m_PeerName;
}

///Generate the best possible peer name
const QString Call::formattedName() const
{
   if (type() == Call::Type::CONFERENCE)
      return tr("Conference");
   else if (!peerContactMethod())
      return "Error";
   else if (peerContactMethod()->contact() && !peerContactMethod()->contact()->formattedName().isEmpty())
      return peerContactMethod()->contact()->formattedName();
   else if (!peerName().isEmpty())
      return d_ptr->m_PeerName;
   else if (peerContactMethod())
      return peerContactMethod()->uri();
   else
      return tr("Unknown");
}

///If the call have a valid record
bool Call::hasRecording() const
{
   return !recordingPath().isEmpty() && QFile::exists(recordingPath());
}

///If this call is encrypted, return the certificate associated with it
Certificate* Call::certificate() const
{
   return d_ptr->m_pCertificate;
}

///Generate an human readable string from the difference between StartTimeStamp and StopTimeStamp (or 'now')
QString Call::length() const
{
   if (d_ptr->m_pStartTimeStamp == d_ptr->m_pStopTimeStamp) return QString(); //Invalid
   int nsec =0;
   if (d_ptr->m_pStopTimeStamp)
      nsec = stopTimeStamp() - startTimeStamp();//If the call is over
   else { //Time to now
      time_t curTime;
      ::time(&curTime);
      nsec = curTime - d_ptr->m_pStartTimeStamp;
   }
   if (nsec/3600)
      return QString("%1:%2:%3 ").arg((nsec%(3600*24))/3600).arg(((nsec%(3600*24))%3600)/60,2,10,QChar('0')).arg(((nsec%(3600*24))%3600)%60,2,10,QChar('0'));
   else
      return QString("%1:%2 ").arg(nsec/60,2,10,QChar('0')).arg(nsec%60,2,10,QChar('0'));
}

///Is this call part of history
bool Call::isHistory() const
{
   if (lifeCycleState() == Call::LifeCycleState::FINISHED && !d_ptr->m_History)
      d_ptr->m_History = true;
   return d_ptr->m_History;
}

///Is this call missed
bool Call::isMissed() const
{
   return d_ptr->m_Missed;
}

///Is the call incoming or outgoing
Call::Direction Call::direction() const
{
   return d_ptr->m_Direction;
}

///Is the call a conference or something else
Call::Type Call::type() const
{
   return d_ptr->m_Type;
}


bool Call::hasRemote() const
{
   return !d_ptr->m_DringId.isEmpty();
}

///Does this call currently has video
bool Call::hasVideo() const
{
   #ifdef ENABLE_VIDEO
   return Video::Manager::instance()->getRenderer(this) != nullptr;
   #else
   return false;
   #endif
}


///Get the current state
Call::State Call::state() const
{
   return d_ptr->m_CurrentState;
}

///Translate the state into its life cycle equivalent
Call::LifeCycleState Call::lifeCycleState() const
{
   return d_ptr->metaStateMap[d_ptr->m_CurrentState];
}

///Get the call recording
bool Call::isRecording() const
{
   return d_ptr->m_Recording;
}

///Get the call account id
Account* Call::account() const
{
   return d_ptr->m_Account;
}

///Get the recording path
const QString Call::recordingPath() const
{
   return d_ptr->m_RecordingPath;
}

///This function could also be called mayBeSecure or haveChancesToBeEncryptedButWeCantTell.
bool Call::isSecure() const
{

   /*if (!d_ptr->m_Account) {
      qDebug() << "Account not set, can't check security";
      return false;
   }
   //BUG this doesn't work
   return d_ptr->m_Account && ((d_ptr->m_Account->isTlsEnabled()) || (d_ptr->m_Account->tlsMethod() != TlsMethodModel::Type::DEFAULT));*/

   return false; //No, it is not and cannot be
} //isSecure

///Return the renderer associated with this call or nullptr
Video::Renderer* Call::videoRenderer() const
{
   #ifdef ENABLE_VIDEO
   return Video::Manager::instance()->getRenderer(this);
   #else
   return nullptr;
   #endif
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Set the transfer number
void Call::setTransferNumber(const QString& number)
{
   if (!d_ptr->m_pTransferNumber) {
      d_ptr->m_pTransferNumber = new TemporaryContactMethod();
   }
   d_ptr->m_pTransferNumber->setUri(number);
}

///Set the call number
void Call::setDialNumber(const QString& number)
{
   //This is not supposed to happen, but this is not a serious issue if it does
   if (d_ptr->m_CurrentState != Call::State::DIALING) {
      qDebug() << "Trying to set a dial number to a non-dialing call, doing nothing";
      return;
   }

   if (!d_ptr->m_pDialNumber) {
      d_ptr->m_pDialNumber = new TemporaryContactMethod();
   }

   d_ptr->m_pDialNumber->setUri(number);
   emit dialNumberChanged(d_ptr->m_pDialNumber->uri());
   emit changed();
   emit changed(this);
}

///Set the dial number from a full phone number
void Call::setDialNumber(const ContactMethod* number)
{
   if (d_ptr->m_CurrentState == Call::State::DIALING && !d_ptr->m_pDialNumber) {
      d_ptr->m_pDialNumber = new TemporaryContactMethod(number);
   }
   if (d_ptr->m_pDialNumber && number)
      d_ptr->m_pDialNumber->setUri(number->uri());
   emit dialNumberChanged(d_ptr->m_pDialNumber->uri());
   emit changed();
   emit changed(this);
}

///Set the recording path
void Call::setRecordingPath(const QString& path)
{
   d_ptr->m_RecordingPath = path;
   if (!d_ptr->m_RecordingPath.isEmpty()) {
      CallManagerInterface& callManager = DBus::CallManager::instance();
      connect(&callManager,SIGNAL(recordPlaybackStopped(QString)), d_ptr, SLOT(stopPlayback(QString))  );
      connect(&callManager,SIGNAL(updatePlaybackScale(QString,int,int))  , d_ptr, SLOT(updatePlayback(QString,int,int)));
   }
}

///Set peer name
void Call::setPeerName(const QString& name)
{
   d_ptr->m_PeerName = name;
}

///Set the account (DIALING only, may be ignored)
void Call::setAccount( Account* account)
{
   if (state() == Call::State::DIALING)
      d_ptr->m_Account = account;
}

/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///The call state just changed (by the daemon)
Call::State CallPrivate::stateChanged(const QString& newStateName)
{
   const Call::State previousState = m_CurrentState;
   if (q_ptr->type() != Call::Type::CONFERENCE) {
      CallPrivate::DaemonState dcs = toDaemonCallState(newStateName);
      if (dcs == CallPrivate::DaemonState::COUNT__ || m_CurrentState == Call::State::COUNT__) {
         qDebug() << "Error: Invalid state change";
         return Call::State::FAILURE;
      }
//       if (previousState == stateChangedStateMap[m_CurrentState][dcs]) {
// #ifndef NDEBUG
//          qDebug() << "Trying to change state with the same state" << previousState;
// #endif
//          return previousState;
//       }

      try {
         //Validate if the transition respect the expected life cycle
         if (!metaStateTransitionValidationMap[stateChangedStateMap[m_CurrentState][dcs]][q_ptr->lifeCycleState()]) {
            qWarning() << "Unexpected state transition from" << q_ptr->state() << "to" << stateChangedStateMap[m_CurrentState][dcs];
            Q_ASSERT(false);
         }
         changeCurrentState(stateChangedStateMap[m_CurrentState][dcs]);
      }
      catch(Call::State& state) {
         qDebug() << "State change failed (stateChangedStateMap)" << state;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }
      catch(CallPrivate::DaemonState& state) {
         qDebug() << "State change failed (stateChangedStateMap)" << state;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }
      catch (...) {
         qDebug() << "State change failed (stateChangedStateMap) other";;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }

      CallManagerInterface & callManager = DBus::CallManager::instance();
      MapStringString details = callManager.getCallDetails(m_DringId);
      if (details[CallPrivate::DetailsMapFields::PEER_NAME] != m_PeerName)
         m_PeerName = details[CallPrivate::DetailsMapFields::PEER_NAME];

      //Load the certificate if it's now available
      if (!q_ptr->certificate() && !details[DRing::Call::Details::TLS_PEER_CERT].isEmpty()) {
         m_pCertificate = CertificateModel::instance()->getCertificateFromContent(details[DRing::Call::Details::TLS_PEER_CERT].toLatin1(),q_ptr->account());
      }

      try {
         (this->*(stateChangedFunctionMap[previousState][dcs]))();
      }
      catch(Call::State& state) {
         qDebug() << "State change failed (stateChangedFunctionMap)" << state;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }
      catch(CallPrivate::DaemonState& state) {
         qDebug() << "State change failed (stateChangedFunctionMap)" << state;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }
      catch (...) {
         qDebug() << "State change failed (stateChangedFunctionMap) other";;
         FORCE_ERROR_STATE_P()
         return m_CurrentState;
      }
   }
   else {
      //Until now, it does not worth using stateChangedStateMap, conferences are quite simple
      //update 2014: Umm... wrong
      m_CurrentState = confStatetoCallState(newStateName); //TODO don't do this
      emit q_ptr->stateChanged();
   }
   if (m_CurrentState != Call::State::DIALING && m_pDialNumber) {
      if (!m_pPeerContactMethod)
         m_pPeerContactMethod = PhoneDirectoryModel::instance()->fromTemporary(m_pDialNumber);
      m_pDialNumber->deleteLater();
      m_pDialNumber = nullptr;
   }
   emit q_ptr->changed();
   emit q_ptr->changed(q_ptr);
   qDebug() << "Calling stateChanged " << newStateName << " -> " << toDaemonCallState(newStateName) << " on call with state " << previousState << ". Become " << m_CurrentState;
   return m_CurrentState;
} //stateChanged

void CallPrivate::performAction(Call::State previousState, Call::Action action)
{
   changeCurrentState(actionPerformedStateMap[previousState][action]);
}

void CallPrivate::performActionCallback(Call::State previousState, Call::Action action)
{
   (this->*(actionPerformedFunctionMap[previousState][action]))();
}

///An account have been performed
Call::State Call::performAction(Call::Action action)
{
   const Call::State previousState = d_ptr->m_CurrentState;

//    if (actionPerformedStateMap[previousState][action] == previousState) {
// #ifndef NDEBUG
//       qDebug() << "Trying to change state with the same state" << previousState;
// #endif
//       return previousState;
//    }

   //update the state
   try {
      d_ptr->performAction(previousState, action);
   }
   catch(Call::State& state) {
      qDebug() << "State change failed (actionPerformedStateMap)" << state;
      FORCE_ERROR_STATE()
      return Call::State::ERROR;
   }
   catch (...) {
      qDebug() << "State change failed (actionPerformedStateMap) other";;
      FORCE_ERROR_STATE()
      return d_ptr->m_CurrentState;
   }

   //execute the action associated with this transition
   try {
      d_ptr->performActionCallback(previousState, action);
   }
   catch(Call::State& state) {
      qDebug() << "State change failed (actionPerformedFunctionMap)" << state;
      FORCE_ERROR_STATE()
      return Call::State::ERROR;
   }
   catch(Call::Action& action) {
      qDebug() << "State change failed (actionPerformedFunctionMap)" << action;
      FORCE_ERROR_STATE()
      return Call::State::ERROR;
   }
   catch (...) {
      qDebug() << "State change failed (actionPerformedFunctionMap) other";;
      FORCE_ERROR_STATE()
      return d_ptr->m_CurrentState;
   }
   qDebug() << "Calling action " << action << " on " << this << " with state " << previousState << ". Become " << d_ptr->m_CurrentState;
   return d_ptr->m_CurrentState;
} //actionPerformed

///Change the state, do not abuse of this, but it is necessary for error cases
void CallPrivate::changeCurrentState(Call::State newState)
{
   if (newState == Call::State::COUNT__) {
      qDebug() << "Error: Call reach invalid state";
      FORCE_ERROR_STATE_P()
      throw newState;
   }

   m_CurrentState = newState;

   emit q_ptr->stateChanged();
   emit q_ptr->changed();
   emit q_ptr->changed(q_ptr);

   initTimer();

   if (q_ptr->lifeCycleState() == Call::LifeCycleState::FINISHED)
      emit q_ptr->isOver(q_ptr);
}

///Set the start timestamp and update the cache
void CallPrivate::setStartTimeStamp(time_t stamp)
{
   m_pStartTimeStamp = stamp;
   //While the HistoryConst is not directly related to the call concept,
   //It is called to often to ignore
   m_HistoryConst = HistoryTimeCategoryModel::timeToHistoryConst(m_pStartTimeStamp);
}

///Send a text message
void Call::sendTextMessage(const QString& message)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.sendTextMessage(d_ptr->m_DringId,message);
   if (!d_ptr->m_pImModel) {
      d_ptr->m_pImModel = IMConversationManager::instance()->getModel(this);
   }
   d_ptr->m_pImModel->d_ptr->addOutgoingMessage(message);
}


/*****************************************************************************
 *                                                                           *
 *                              Automate function                            *
 *                                                                           *
 ****************************************************************************/
///@warning DO NOT TOUCH THAT, THEY ARE CALLED FROM AN AUTOMATE, HIGH FRAGILITY

///Do nothing (literally)
void CallPrivate::nothing()
{
   //nop
}

void CallPrivate::error()
{
   if (q_ptr->videoRenderer()) {
      //Well, in this case we have no choice, it still doesn't belong here
      q_ptr->videoRenderer()->stopRendering();
   }
   throw QString("There was an error handling your call, please restart Ring.Is you encounter this problem often, \
   please open Ring-KDE in a terminal and send this last 100 lines before this message in a bug report at \
   https://projects.savoirfairelinux.com/projects/sflphone/issues");
}

///Change history state to failure
void CallPrivate::failure()
{
   m_Missed = true;
   //This is how it always was done
   //The main point is to leave the call in the CallList
   start();
}

///Accept the call
void CallPrivate::accept()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Accepting call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   Q_NOREPLY callManager.accept(m_DringId);
   time_t curTime;
   ::time(&curTime);
   setStartTimeStamp(curTime);
   m_Direction = Call::Direction::INCOMING;
}

///Refuse the call
void CallPrivate::refuse()
{
   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Refusing call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   const bool ret = callManager.refuse(m_DringId);
   time_t curTime;
   ::time(&curTime);
   setStartTimeStamp(curTime);
   m_Missed = true;

   //If the daemon crashed then re-spawned when a call is ringing, this happen.
   if (!ret)
      FORCE_ERROR_STATE_P()
}

///Accept the transfer
void CallPrivate::acceptTransf()
{
   Q_ASSERT_IS_IN_PROGRESS

   if (!m_pTransferNumber) {
      qDebug() << "Trying to transfer to no one";
      return;
   }
   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Accepting call and transferring it to number : " << m_pTransferNumber->uri() << ". callId : " << q_ptr  << "ConfId:" << q_ptr;
   callManager.accept(m_DringId);
   Q_NOREPLY callManager.transfer(m_DringId, m_pTransferNumber->uri());
}

///Put the call on hold
void CallPrivate::acceptHold()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Accepting call and holding it. callId : " << q_ptr  << "ConfId:" << q_ptr;
   callManager.accept(m_DringId);
   Q_NOREPLY callManager.hold(m_DringId);
   m_Direction = Call::Direction::INCOMING;
}

///Hang up
void CallPrivate::hangUp()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface & callManager = DBus::CallManager::instance();
   time_t curTime;
   ::time(&curTime);
   m_pStopTimeStamp = curTime;
   qDebug() << "Hanging up call. callId : " << q_ptr << "ConfId:" << q_ptr;
   bool ret;
   if (q_ptr->videoRenderer()) { //TODO remove, cheap hack
      q_ptr->videoRenderer()->stopRendering();
   }
   if (q_ptr->type() != Call::Type::CONFERENCE)
      ret = callManager.hangUp(m_DringId);
   else
      ret = callManager.hangUpConference(m_DringId);
   if (!ret) { //Can happen if the daemon crash and open again
      qDebug() << "Error: Invalid call, the daemon may have crashed";
      changeCurrentState(Call::State::OVER);
   }
   if (m_pTimer)
      m_pTimer->stop();
}

///Remove the call without contacting the daemon
void CallPrivate::remove()
{
   if (q_ptr->lifeCycleState() != Call::LifeCycleState::FINISHED)
      FORCE_ERROR_STATE_P()

   CallManagerInterface & callManager = DBus::CallManager::instance();

   //HACK Call hang up again to make sure the busytone stop, this should
   //return true or false, both are valid, no point to check the result
   if (q_ptr->type() != Call::Type::CONFERENCE)
      callManager.hangUp(q_ptr->dringId());
   else
      callManager.hangUpConference(q_ptr->dringId());

   emit q_ptr->isOver(q_ptr);
   emit q_ptr->stateChanged();
   emit q_ptr->changed();
   emit q_ptr->changed(q_ptr);
}

///Cancel this call
void CallPrivate::cancel()
{
   //This one can be over if the peer server failed to comply with the correct sequence
   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Canceling call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   emit q_ptr->dialNumberChanged(QString());
//    Q_NOREPLY callManager.hangUp(m_DringId);
   if (!callManager.hangUp(m_DringId)) {
      qWarning() << "HangUp failed, the call was probably already over";
      changeCurrentState(Call::State::OVER);
   }
}

///Put on hold
void CallPrivate::hold()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Holding call. callId : " << q_ptr << "ConfId:" << q_ptr;
   if (q_ptr->type() != Call::Type::CONFERENCE)
      Q_NOREPLY callManager.hold(q_ptr->dringId());
   else
      Q_NOREPLY callManager.holdConference(q_ptr->dringId());
}

///Start the call
void CallPrivate::call()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface& callManager = DBus::CallManager::instance();
   qDebug() << "account = " << m_Account;
   if(!m_Account) {
      qDebug() << "Account is not set, taking the first registered.";
      m_Account = AvailableAccountModel::currentDefaultAccount(m_pDialNumber);
   }
   //Calls to empty URI should not be allowed, dring will go crazy
   if ((!m_pDialNumber) || m_pDialNumber->uri().isEmpty()) {
      qDebug() << "Trying to call an empty URI";
      changeCurrentState(Call::State::FAILURE);
      if (!m_pDialNumber) {
         emit q_ptr->dialNumberChanged(QString());
      }
      else {
         m_pDialNumber->deleteLater();
         m_pDialNumber = nullptr;
      }
      q_ptr->setPeerName(tr("Failure"));
      emit q_ptr->stateChanged();
      emit q_ptr->changed();
   }
   //Normal case
   else if(m_Account) {
      qDebug() << "Calling " << q_ptr->peerContactMethod()->uri() << " with account " << m_Account << ". callId : " << q_ptr  << "ConfId:" << q_ptr;

      this->m_pPeerContactMethod = PhoneDirectoryModel::instance()->getNumber(m_pDialNumber->uri(),q_ptr->account());

      //Warning: m_pDialNumber can become nullptr when linking directly
      m_DringId = callManager.placeCall(m_Account->id(), m_pDialNumber->uri());

      //This can happen when the daemon cannot allocate memory
      if (m_DringId.isEmpty()) {
         changeCurrentState(Call::State::FAILURE);
         qWarning() << "Creating the call to" << m_pDialNumber->uri() << "failed";
         m_DringId = "FAILED"; //TODO once the ABORTED state is implemented, use it
         return;
      }

      CallModel::instance()->registerCall(q_ptr);
      setObjectName("Call:"+m_DringId);

      if (PersonModel::instance()->hasCollections()) {
         if (q_ptr->peerContactMethod()->contact())
            m_PeerName = q_ptr->peerContactMethod()->contact()->formattedName();
      }
      connect(q_ptr->peerContactMethod(),SIGNAL(presentChanged(bool)),this,SLOT(updated()));
      time_t curTime;
      ::time(&curTime);
      setStartTimeStamp(curTime);
      m_Direction = Call::Direction::OUTGOING;
      if (q_ptr->peerContactMethod()) {
         q_ptr->peerContactMethod()->addCall(q_ptr);
      }
      if (m_pDialNumber)
         emit q_ptr->dialNumberChanged(QString());
      m_pDialNumber->deleteLater();
      m_pDialNumber = nullptr;
   }
   else {
      qDebug() << "Trying to call " << (m_pTransferNumber?QString(m_pTransferNumber->uri()):"ERROR")
         << " with no account registered . callId : " << q_ptr  << "ConfId:" << q_ptr;
      throw tr("No account registered!");
   }
}

///Trnasfer the call
void CallPrivate::transfer()
{
   Q_ASSERT_IS_IN_PROGRESS

   if (m_pTransferNumber) {
      CallManagerInterface & callManager = DBus::CallManager::instance();
      qDebug() << "Transferring call to number : " << m_pTransferNumber->uri() << ". callId : " << q_ptr;
      Q_NOREPLY callManager.transfer(m_DringId, m_pTransferNumber->uri());
      time_t curTime;
      ::time(&curTime);
      m_pStopTimeStamp = curTime;
   }
}

///Unhold the call
void CallPrivate::unhold()
{
   Q_ASSERT_IS_IN_PROGRESS

   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Unholding call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   if (q_ptr->type() != Call::Type::CONFERENCE)
      Q_NOREPLY callManager.unhold(q_ptr->dringId());
   else
      Q_NOREPLY callManager.unholdConference(q_ptr->dringId());
}

///Record the call
void CallPrivate::toggleRecord()
{
   CallManagerInterface & callManager = DBus::CallManager::instance();
   qDebug() << "Setting record " << !m_Recording << " for call. callId : " << q_ptr  << "ConfId:" << q_ptr;

   callManager.toggleRecording(q_ptr->dringId());
}

///Start the timer
void CallPrivate::start()
{
   qDebug() << "Starting call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   time_t curTime;
   ::time(&curTime);
   emit q_ptr->changed();
   emit q_ptr->changed(q_ptr);
   if (m_pDialNumber) {
      if (!m_pPeerContactMethod)
         m_pPeerContactMethod = PhoneDirectoryModel::instance()->fromTemporary(m_pDialNumber);
      m_pDialNumber->deleteLater();
      m_pDialNumber = nullptr;
   }
   setStartTimeStamp(curTime);
}

///Toggle the timer
void CallPrivate::startStop()
{
   qDebug() << "Starting and stoping call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   time_t curTime;
   ::time(&curTime);
   setStartTimeStamp(curTime);
   m_pStopTimeStamp  = curTime;
}

///Stop the timer
void CallPrivate::stop()
{
   qDebug() << "Stoping call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   if (q_ptr->videoRenderer()) { //TODO remove, cheap hack
      q_ptr->videoRenderer()->stopRendering();
   }
   time_t curTime;
   ::time(&curTime);
   m_pStopTimeStamp = curTime;
}

///Handle error instead of crashing
void CallPrivate::startWeird()
{
   qDebug() << "Starting call. callId : " << q_ptr  << "ConfId:" << q_ptr;
   time_t curTime;
   ::time(&curTime);
   setStartTimeStamp(curTime);
   qDebug() << "Warning : call " << q_ptr << " had an unexpected transition of state at its start.";
}

///Print a warning
void CallPrivate::warning()
{
   qWarning() << "Warning : call " << q_ptr << " had an unexpected transition of state.(" << m_CurrentState << ")";
   switch (m_CurrentState) {
      case Call::State::FAILURE        :
      case Call::State::ERROR          :
      case Call::State::COUNT__          :
         //If not stopped, then the counter will keep going
         //Getting here indicate something wrong happened
         //It can be normal, aka, an invalid URI such as '><'
         // or an Ring-KDE bug
         stop();
         break;
      case Call::State::TRANSFERRED    :
      case Call::State::TRANSF_HOLD    :
      case Call::State::DIALING        :
      case Call::State::INITIALIZATION:
      case Call::State::INCOMING       :
      case Call::State::RINGING        :
      case Call::State::CURRENT        :
      case Call::State::HOLD           :
      case Call::State::BUSY           :
      case Call::State::OVER           :
      case Call::State::CONFERENCE     :
      case Call::State::CONFERENCE_HOLD:
      default:
         break;
   }
}

/*****************************************************************************
 *                                                                           *
 *                             Keyboard handling                             *
 *                                                                           *
 ****************************************************************************/

///Input text on the call item
void Call::appendText(const QString& str)
{
   TemporaryContactMethod* editNumber = nullptr;

   switch (d_ptr->m_CurrentState) {
   case Call::State::TRANSFERRED :
   case Call::State::TRANSF_HOLD :
      editNumber = d_ptr->m_pTransferNumber;
      break;
   case Call::State::DIALING     :
      editNumber = d_ptr->m_pDialNumber;
      break;
   case Call::State::INITIALIZATION:
   case Call::State::INCOMING:
   case Call::State::RINGING:
   case Call::State::CURRENT:
   case Call::State::HOLD:
   case Call::State::FAILURE:
   case Call::State::BUSY:
   case Call::State::OVER:
   case Call::State::ERROR:
   case Call::State::CONFERENCE:
   case Call::State::CONFERENCE_HOLD:
   case Call::State::COUNT__:
   default:
      qDebug() << "Backspace on call not editable. Doing nothing.";
      return;
   }

   if (editNumber) {
      editNumber->setUri(editNumber->uri()+str);
      if (state() == Call::State::DIALING)
         emit dialNumberChanged(editNumber->uri());
   }
   else
      qDebug() << "TemporaryContactMethod not defined";


   emit changed();
   emit changed(this);
}

///Remove the last character
void Call::backspaceItemText()
{
   TemporaryContactMethod* editNumber = nullptr;

   switch (d_ptr->m_CurrentState) {
      case Call::State::TRANSFERRED      :
      case Call::State::TRANSF_HOLD      :
         editNumber = d_ptr->m_pTransferNumber;
         break;
      case Call::State::DIALING          :
         editNumber = d_ptr->m_pDialNumber;
         break;
      case Call::State::INITIALIZATION:
      case Call::State::INCOMING:
      case Call::State::RINGING:
      case Call::State::CURRENT:
      case Call::State::HOLD:
      case Call::State::FAILURE:
      case Call::State::BUSY:
      case Call::State::OVER:
      case Call::State::ERROR:
      case Call::State::CONFERENCE:
      case Call::State::CONFERENCE_HOLD:
      case Call::State::COUNT__:
      default                          :
         qDebug() << "Backspace on call not editable. Doing nothing.";
         return;
   }
   if (editNumber) {
      QString text = editNumber->uri();
      const int textSize = text.size();
      if(textSize > 0) {
         editNumber->setUri(text.remove(textSize-1, 1));
         emit changed();
         emit changed(this);
      }
      else {
         d_ptr->changeCurrentState(Call::State::OVER);
      }
   }
   else
      qDebug() << "TemporaryContactMethod not defined";
}

///Reset the string a dialing or transfer call
void Call::reset()
{
   TemporaryContactMethod* editNumber = nullptr;

   switch (d_ptr->m_CurrentState) {
      case Call::State::TRANSFERRED      :
      case Call::State::TRANSF_HOLD      :
         editNumber = d_ptr->m_pTransferNumber;
         break;
      case Call::State::DIALING          :
         editNumber = d_ptr->m_pDialNumber;
         break;
      case Call::State::INITIALIZATION   :
      case Call::State::INCOMING         :
      case Call::State::RINGING          :
      case Call::State::CURRENT          :
      case Call::State::HOLD             :
      case Call::State::FAILURE          :
      case Call::State::BUSY             :
      case Call::State::OVER             :
      case Call::State::ERROR            :
      case Call::State::CONFERENCE       :
      case Call::State::CONFERENCE_HOLD  :
      case Call::State::COUNT__:
      default                            :
         qDebug() << "Cannot reset" << d_ptr->m_CurrentState << "calls";
         return;
   }
   if (editNumber) {
      editNumber->setUri(QString());
   }
}

/*****************************************************************************
 *                                                                           *
 *                                   SLOTS                                   *
 *                                                                           *
 ****************************************************************************/

void CallPrivate::updated()
{
   emit q_ptr->changed();
   emit q_ptr->changed(q_ptr);
}

///Play the record, if any
void Call::playRecording()
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   const bool retval = callManager.startRecordedFilePlayback(recordingPath());
   if (retval)
      emit playbackStarted();
}

///Stop the record, if any
void Call::stopRecording()
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.stopRecordedFilePlayback(recordingPath());
   emit playbackStopped(); //TODO remove this, it is a workaround for bug #11942
}

///seek the record, if any
void Call::seekRecording(double position)
{
   CallManagerInterface& callManager = DBus::CallManager::instance();
   Q_NOREPLY callManager.recordPlaybackSeek(position);
}

///Daemon record playback stopped
void CallPrivate::stopPlayback(const QString& filePath)
{
   if (filePath == q_ptr->recordingPath()) {
      emit q_ptr->playbackStopped();
   }
}

///Daemon playback position chnaged
void CallPrivate::updatePlayback(const QString& path, int position,int size)
{
   if (path == m_RecordingPath) {
      emit q_ptr->playbackPositionChanged(position,size);
   }
}

UserActionModel* Call::userActionModel() const
{
   if (!d_ptr->m_pUserActionModel)
      d_ptr->m_pUserActionModel = new UserActionModel(const_cast<Call*>(this));
   return d_ptr->m_pUserActionModel;
}

///Check if creating a timer is necessary
void CallPrivate::initTimer()
{
   if (q_ptr->lifeCycleState() == Call::LifeCycleState::PROGRESS) {
      if (!m_pTimer) {
         m_pTimer = new QTimer(this);
         m_pTimer->setInterval(1000);
         connect(m_pTimer,SIGNAL(timeout()),this,SLOT(updated()));
      }
      if (!m_pTimer->isActive())
         m_pTimer->start();
   }
   else if (m_pTimer && q_ptr->lifeCycleState() != Call::LifeCycleState::PROGRESS) {
      m_pTimer->stop();
      delete m_pTimer;
      m_pTimer = nullptr;
   }
}

QVariant Call::roleData(Call::Role role) const
{
   return roleData(static_cast<int>(role));
}

///Common source for model data roles
QVariant Call::roleData(int role) const
{
   const Person* ct = peerContactMethod()?peerContactMethod()->contact():nullptr;
   switch (role) {
      case static_cast<int>(Call::Role::Name):
      case Qt::DisplayRole:
         if (type() == Call::Type::CONFERENCE)
            return tr("Conference");
         else if (state() == Call::State::DIALING)
            return dialNumber();
         else if (d_ptr->m_PeerName.isEmpty())
            return ct?ct->formattedName():peerContactMethod()?peerContactMethod()->uri():dialNumber();
         else
            return formattedName();
         break;
      case Qt::ToolTipRole:
         return tr("Account: ") + (account()?account()->alias():QString());
         break;
      case Qt::EditRole:
         return dialNumber();
      case static_cast<int>(Call::Role::Number):
         return peerContactMethod()->uri();
         break;
      case static_cast<int>(Call::Role::Direction):
         return QVariant::fromValue(d_ptr->m_Direction);
         break;
      case static_cast<int>(Call::Role::Date):
         return (int)startTimeStamp();
         break;
      case static_cast<int>(Call::Role::Length):
         return length();
         break;
      case static_cast<int>(Call::Role::FormattedDate):
         return QDateTime::fromTime_t(startTimeStamp()).toString();
         break;
      case static_cast<int>(Call::Role::HasRecording):
         return hasRecording();
         break;
      case static_cast<int>(Call::Role::Filter): {
         QString normStripppedC;
         foreach(QChar char2,(static_cast<int>(direction())+'\n'+roleData(Call::Role::Name).toString()+'\n'+
            roleData(Call::Role::Number).toString()).toLower().normalized(QString::NormalizationForm_KD) ) {
            if (!char2.combiningClass())
               normStripppedC += char2;
         }
         return normStripppedC;
         }
         break;
      case static_cast<int>(Call::Role::FuzzyDate):
         return QVariant::fromValue(d_ptr->m_HistoryConst);
         break;
      case static_cast<int>(Call::Role::IsBookmark):
         return false;
         break;
      case static_cast<int>(Call::Role::Security):
         return isSecure();
         break;
      case static_cast<int>(Call::Role::Department):
         return ct?ct->department():QVariant();
         break;
      case static_cast<int>(Call::Role::Email):
         return ct?ct->preferredEmail():QVariant();
         break;
      case static_cast<int>(Call::Role::Organisation):
         return ct?ct->organization():QVariant();
         break;
      case static_cast<int>(Call::Role::Object):
         return QVariant::fromValue(const_cast<Call*>(this));
         break;
      case static_cast<int>(Call::Role::ContactMethod):
         return QVariant::fromValue(peerContactMethod());
         break;
      case static_cast<int>(Call::Role::Photo):
         return ct?ct->photo():QVariant();
         break;
      case static_cast<int>(Call::Role::State):
         return QVariant::fromValue(state());
         break;
      case static_cast<int>(Call::Role::StartTime):
         return (int) d_ptr->m_pStartTimeStamp;
      case static_cast<int>(Call::Role::StopTime):
         return (int) d_ptr->m_pStopTimeStamp;
      case static_cast<int>(Call::Role::IsRecording):
         return isRecording();
      case static_cast<int>(Call::Role::IsPresent):
         return peerContactMethod()->isPresent();
      case static_cast<int>(Call::Role::IsTracked):
         return peerContactMethod()->isTracked();
      case static_cast<int>(Call::Role::SupportPresence):
         return peerContactMethod()->supportPresence();
      case static_cast<int>(Call::Role::CategoryIcon):
         return peerContactMethod()->category()->icon(peerContactMethod()->isTracked(),peerContactMethod()->isPresent());
      case static_cast<int>(Call::Role::CallCount):
         return peerContactMethod()->callCount();
      case static_cast<int>(Call::Role::TotalSpentTime):
         return peerContactMethod()->totalSpentTime();
      case static_cast<int>(Call::Role::Certificate):
         return QVariant::fromValue(certificate());
         break;
      case static_cast<int>(Call::Role::DropState):
         return property("dropState");
         break;
      case static_cast<int>(Call::Role::Missed):
         return isMissed();
      case static_cast<int>(Call::Role::LifeCycleState):
         return static_cast<int>(lifeCycleState()); //TODO Qt5, use the Q_ENUM
      case static_cast<int>(Call::Role::DTMFAnimState):
         return property("DTMFAnimState");
         break;
      case static_cast<int>(Call::Role::LastDTMFidx):
         return property("latestDtmfIdx");
         break;
      case static_cast<int>(Call::Role::DropPosition):
         return property("dropPosition");
         break;
      default:
         break;
   };
   return QVariant();
}


void Call::playDTMF(const QString& str)
{
   Q_NOREPLY DBus::CallManager::instance().playDTMF(str);
   emit dtmfPlayed(str);
}

#undef Q_ASSERT_IS_IN_PROGRESS
#undef FORCE_ERROR_STATE
#undef FORCE_ERROR_STATE_P

#include <call.moc>

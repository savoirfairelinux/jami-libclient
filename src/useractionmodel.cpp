/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#include "useractionmodel.h"

//Qt
#include <QtCore/QItemSelection>
#include <QtCore/QSortFilterProxyModel>

//Ring
#include "call.h"
#include "callmodel.h"
#include "account.h"
#include "accountmodel.h"
#include "availableaccountmodel.h"
#include "interfaces/instances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "private/useractions.h"
#include "private/matrixutils.h"

#define UAM UserActionModel

class ActiveUserActionModel : public QSortFilterProxyModel
{
public:
   ActiveUserActionModel(QAbstractItemModel* parent) : QSortFilterProxyModel(parent)
   {
      setSourceModel(parent);
   }
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

class UserActionModelPrivate final : public QObject
{
   Q_OBJECT
public:
   enum class SelectionState {
      NONE  , /*!< No Item is selected         */
      UNIQUE, /*!< A single item is selected   */
      MULTI , /*!< Multiple items are selected */
      COUNT__,
   };

   enum class UserActionModelMode {
      CALL     , /*!< Model the react on a single call element    */
      CALLMODEL, /*!< Model that react on the callmodel selection */
   };

   //Availability matrices
   UserActionModelPrivate(UserActionModel* parent, const FlagPack<UAM::Context>& c);
   static const Matrix1D< UAM::Action                            , bool  > heterogenous_call_options ;
   static const Matrix2D< UAM::Action, Call::State               , bool  > availableActionMap        ;
   static const Matrix2D< UAM::Action, Account::RegistrationState, bool  > availableAccountActionMap ;
   static const Matrix2D< UAM::Action, SelectionState            , bool  > multi_call_options        ;
   static const Matrix2D< UAM::Action, Account::Protocol         , bool  > availableProtocolActions  ;
   static const Matrix1D< UAM::Action, FlagPack<UAM::Context>> actionContext             ;
   static const Matrix1D< UAM::Action, FlagPack<UAM::Asset>  > availableByAsset          ;

   static const Matrix2D< UAM::Action, SelectionState, UAM::ActionStatfulnessLevel > actionStatefulness;

   //Helpers
   bool updateAction   (UAM::Action action                          );
   bool updateByCall   (UAM::Action action, const Call* c           );
   void updateCheckMask(int& ret, UAM::Action action, const Call* c );

   //Attributes
   Call*                                                             m_pCall              ;
   UserActionModelMode                                               m_Mode               ;
   SelectionState                                                    m_SelectionState     ;
   TypedStateMachine< bool, UAM::Action>                 m_CurrentActions     ;
   Matrix1D< UAM::Action, Qt::CheckState>                m_CurrentActionsState;
   Matrix1D< UAM::Action, QString>                       m_ActionNames        ;
   ActiveUserActionModel*                                            m_pActiveModel       ;
   FlagPack<UAM::Context>                                m_fContext           ;

private:
   UserActionModel* q_ptr;

public Q_SLOTS:
   //Single call mode
   void slotStateChanged(); //TODO

   //CallModel mode
   void updateActions();
};


/*
 * This algorithm implement a matrix multiplication of the 4 sources of relevant states.
 * The result of the multiplication indicate is the option is enabled in a given scenario.
 */

#define UAMA UserActionModel::Action
#define CS Call::State
//Enabled actions
static EnumClassReordering<Call::State> co = {
   CS::NEW            , /* >----------|                                                                                                                       */
   CS::INCOMING       , /* -----------|-------|                                                                                                               */
   CS::RINGING        , /* -----------|-------|-------|                                                                                                       */
   CS::CURRENT        , /* -----------|-------|-------|------|                                                                                                */
   CS::DIALING        , /* -----------|-------|-------|------|-----|                                                                                          */
   CS::HOLD           , /* -----------|-------|-------|------|-----|-------|                                                                                  */
   CS::FAILURE        , /* -----------|-------|-------|------|-----|-------|------|                                                                           */
   CS::BUSY           , /* -----------|-------|-------|------|-----|-------|------|------|                                                                    */
   CS::TRANSFERRED    , /* -----------|-------|-------|------|-----|-------|------|------|-----|                                                              */
   CS::TRANSF_HOLD    , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|                                                      */
   CS::OVER           , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|                                               */
   CS::ERROR          , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|                                         */
   CS::CONFERENCE     , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|                                  */
   CS::CONFERENCE_HOLD, /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|                           */
   CS::INITIALIZATION , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|                   */
   CS::ABORTED        , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|------|            */
   CS::CONNECTED      , /* -----------|-------|-------|------|-----|-------|------|------|-----|-------|------|-----|------|------|-------|------|-----|      */
};                      /*            |       |       |      |     |       |      |      |     |       |      |     |      |      |       |      |     |      */
                        /*            \/      \/      \/     \/    \/      \/     \/     \/    \/      \/     \/    \/     \/     \/      \/     \/    \/     */
const Matrix2D< UAM::Action, Call::State, bool > UserActionModelPrivate::availableActionMap = {
 { UAMA::ACCEPT            , {{co, { false, true  , false, false, true , false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::HOLD              , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, true , false, false, false, false }}}},
 { UAMA::MUTE_AUDIO        , {{co, { false, false , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::MUTE_VIDEO        , {{co, { false, false , true , true , false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::SERVER_TRANSFER   , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::RECORD            , {{co, { false, false , true , true , false, true , false, false, true , true , false, false, true , true , false, false, false }}}},
 { UAMA::HANGUP            , {{co, { true , true  , true , true , true , true , true , true , true , true , false, true , true , true , true , false, true  }}}},
 { UAMA::JOIN              , {{co, { false, false , true , true , false, true , false, false, true , true , false, false, true , true , false, false, false }}}},
 { UAMA::ADD_NEW           , {{co, { false, false , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false }}}},
 { UAMA::TOGGLE_VIDEO      , {{co, { false, false , false, true , false, true , false, false, false, false, false, false, true , false, false, false, false }}}},
 { UAMA::ADD_CONTACT       , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::ADD_TO_CONTACT    , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::DELETE_CONTACT    , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::EMAIL_CONTACT     , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::COPY_CONTACT      , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::BOOKMARK          , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::VIEW_CHAT_HISTORY , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::ADD_CONTACT_METHOD, {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::CALL_CONTACT      , {{co, { false, true  , true , true , false, true , true , true , true , true , true , true , false, false, true , true , true  }}}},
 { UAMA::REMOVE_HISTORY    , {{co, { false, false , false, false, false, false, true , true , false, false, true , false, false, false, false, false, false }}}},
};
#undef CS

/**
 * Assuming a call is in progress, the communication can still be valid if the account is down, however,
 * this will impact the available actions
 */
const Matrix2D< UAMA, Account::RegistrationState, bool > UserActionModelPrivate::availableAccountActionMap = {
   /*                             READY  UNREGISTERED  TRYING    ERROR   */
   { UAMA::ACCEPT            , {{ true ,    false,     false,    false  }}},
   { UAMA::HOLD              , {{ true ,    false,     false,    false  }}},
   { UAMA::MUTE_AUDIO        , {{ true ,    true ,     true ,    true   }}},
   { UAMA::MUTE_VIDEO        , {{ true ,    true ,     true ,    true   }}},
   { UAMA::SERVER_TRANSFER   , {{ true ,    false,     false,    false  }}},
   { UAMA::RECORD            , {{ true ,    true ,     true ,    true   }}},
   { UAMA::HANGUP            , {{ true ,    true ,     true ,    true   }}},

   { UAMA::JOIN              , {{ true ,    true ,     true ,    true   }}},

   { UAMA::ADD_NEW           , {{ true ,    false,     true ,    true   }}},
   { UAMA::TOGGLE_VIDEO      , {{ true ,    true ,     true ,    true   }}},
   { UAMA::ADD_CONTACT       , {{ true ,    true ,     true ,    true   }}},
   { UAMA::ADD_TO_CONTACT    , {{ true ,    true ,     true ,    true   }}},
   { UAMA::DELETE_CONTACT    , {{ true ,    true ,     true ,    true   }}},
   { UAMA::EMAIL_CONTACT     , {{ true ,    true ,     true ,    true   }}},
   { UAMA::COPY_CONTACT      , {{ true ,    true ,     true ,    true   }}},
   { UAMA::BOOKMARK          , {{ true ,    true ,     true ,    true   }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ true ,    true ,     true ,    true   }}},
   { UAMA::ADD_CONTACT_METHOD, {{ true ,    true ,     true ,    true   }}},
   { UAMA::CALL_CONTACT      , {{ true ,    true ,     true ,    true   }}},
   { UAMA::REMOVE_HISTORY    , {{ true ,    true ,     true ,    true   }}},
};

/**
 * This matrix define if an option is available depending on the number of selection elements
 */
const Matrix2D< UAMA, UserActionModelPrivate::SelectionState, bool > UserActionModelPrivate::multi_call_options = {
   /*                             NONE   UNIQUE   MULTI  */
   { UAMA::ACCEPT            , {{ false,  true ,  true  }}},
   { UAMA::HOLD              , {{ false,  true ,  false }}},
   { UAMA::MUTE_AUDIO        , {{ false,  true ,  true  }}},
   { UAMA::MUTE_VIDEO        , {{ false,  true ,  true  }}},
   { UAMA::SERVER_TRANSFER   , {{ false,  true ,  true  }}},
   { UAMA::RECORD            , {{ false,  true ,  true  }}},
   { UAMA::HANGUP            , {{ false,  true ,  true  }}},

   { UAMA::JOIN              , {{ false,  false,  true  }}},

   { UAMA::ADD_NEW           , {{ true ,  false,  false }}},
   { UAMA::TOGGLE_VIDEO      , {{ false,  true ,  false }}},
   { UAMA::ADD_CONTACT       , {{ false,  true ,  false }}},
   { UAMA::ADD_TO_CONTACT    , {{ false,  true ,  false }}},
   { UAMA::DELETE_CONTACT    , {{ false,  true ,  false }}},
   { UAMA::EMAIL_CONTACT     , {{ false,  true ,  false }}},
   { UAMA::COPY_CONTACT      , {{ false,  true ,  false }}},
   { UAMA::BOOKMARK          , {{ false,  true ,  false }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ false,  true ,  false }}},
   { UAMA::ADD_CONTACT_METHOD, {{ false,  true ,  false }}},
   { UAMA::CALL_CONTACT      , {{ false,  true ,  false }}},
   { UAMA::REMOVE_HISTORY    , {{ false,  true ,  true  }}},
};

/**
 * This matrix define if an option is available when multiple elements with mismatching CheckState are selected
 */
const Matrix1D< UAMA, bool > UserActionModelPrivate::heterogenous_call_options = {
   { UAMA::ACCEPT            , true  }, /* N/A                                       */
   { UAMA::HOLD              , false }, /* Do not allow to set a state               */
   { UAMA::MUTE_AUDIO        , true  }, /* Force mute on all calls                   */
   { UAMA::MUTE_VIDEO        , true  }, /* Mute all calls                            */
   { UAMA::SERVER_TRANSFER   , false }, /* It make no sense to let the user set this */
   { UAMA::RECORD            , true  }, /* Force "record" on all calls               */
   { UAMA::HANGUP            , true  }, /* N/A                                       */

   { UAMA::JOIN              , true  }, /* N/A                                       */

   { UAMA::ADD_NEW           , false }, /* N/A                                       */
   { UAMA::TOGGLE_VIDEO      , false },
   { UAMA::ADD_CONTACT       , false },
   { UAMA::ADD_TO_CONTACT    , false },
   { UAMA::DELETE_CONTACT    , false },
   { UAMA::EMAIL_CONTACT     , false },
   { UAMA::COPY_CONTACT      , false },
   { UAMA::BOOKMARK          , false },
   { UAMA::VIEW_CHAT_HISTORY , false },
   { UAMA::ADD_CONTACT_METHOD, false },
   { UAMA::CALL_CONTACT      , false },
   { UAMA::REMOVE_HISTORY    , false },
};

/**
 * This matrix allow to enable/disable actions depending on the call protocol
 */
const Matrix2D< UAMA, Account::Protocol, bool > UserActionModelPrivate::availableProtocolActions = {
   /*                              SIP    IAX    DHT   */
   { UAMA::ACCEPT            , {{ true , true , true  }}},
   { UAMA::HOLD              , {{ true , true , true  }}},
   { UAMA::MUTE_AUDIO        , {{ true , true , true  }}},
   { UAMA::MUTE_VIDEO        , {{ true , true , true  }}},
   { UAMA::SERVER_TRANSFER   , {{ true , false, false }}},
   { UAMA::RECORD            , {{ true , true , true  }}},
   { UAMA::HANGUP            , {{ true , true , true  }}},

   { UAMA::JOIN              , {{ true , true , true  }}},

   { UAMA::ADD_NEW           , {{ true , true , true  }}},

   { UAMA::TOGGLE_VIDEO      , {{ true,  true , true  }}},
   { UAMA::ADD_CONTACT       , {{ true,  true , true  }}},
   { UAMA::ADD_TO_CONTACT    , {{ true,  true , true  }}},
   { UAMA::DELETE_CONTACT    , {{ true,  true , true  }}},
   { UAMA::EMAIL_CONTACT     , {{ true,  true , true  }}},
   { UAMA::COPY_CONTACT      , {{ true,  true , true  }}},
   { UAMA::BOOKMARK          , {{ true,  true , true  }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ true,  true , true  }}},
   { UAMA::ADD_CONTACT_METHOD, {{ true,  true , true  }}},
   { UAMA::CALL_CONTACT      , {{ true,  true , true  }}},
   { UAMA::REMOVE_HISTORY    , {{ true,  true , true  }}},
};

/**
 * This matrix define if an action is stateless or stateful. The only state
 * supported is "checked", but when multiple items are selected, this can
 * cause a heterogenous bunch of checked and unchecked elements, this is
 * called "TRISTATE".
 */
#define ST UserActionModel::ActionStatfulnessLevel::
const Matrix2D< UAMA, UserActionModelPrivate::SelectionState, UserActionModel::ActionStatfulnessLevel > UserActionModelPrivate::actionStatefulness = {
   /*                                NONE          UNIQUE             MULTI       */
   { UAMA::ACCEPT            , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::HOLD              , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::MUTE_AUDIO        , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::MUTE_VIDEO        , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::SERVER_TRANSFER   , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::RECORD            , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::HANGUP            , {{ ST UNISTATE,  ST UNISTATE     ,  ST TRISTATE  }}},

   { UAMA::JOIN              , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},

   { UAMA::ADD_NEW           , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::TOGGLE_VIDEO      , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::ADD_CONTACT       , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::ADD_TO_CONTACT    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::DELETE_CONTACT    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::EMAIL_CONTACT     , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::COPY_CONTACT      , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::BOOKMARK          , {{ ST UNISTATE,  ST CHECKABLE    ,  ST TRISTATE  }}},
   { UAMA::VIEW_CHAT_HISTORY , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::ADD_CONTACT_METHOD, {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::CALL_CONTACT      , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
   { UAMA::REMOVE_HISTORY    , {{ ST UNISTATE,  ST UNISTATE     ,  ST UNISTATE  }}},
};
#undef ST

/**
 * This matrix categorize each actions. Action can be enabled in a given context
 * while being hidden in another. This allow, for example, to use the
 * UserActionModel to fill context menu.
 */
const Matrix1D< UAMA, FlagPack<UAM::Context>> UserActionModelPrivate::actionContext = {
   { UAMA::ACCEPT            , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::HOLD              , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::MUTE_AUDIO        , UAM::Context::RECOMMANDED },

   { UAMA::MUTE_VIDEO        , UAM::Context::RECOMMANDED },

   { UAMA::SERVER_TRANSFER   , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::RECORD            , UAM::Context::RECOMMANDED },

   { UAMA::HANGUP            , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::JOIN              , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::ADD_NEW           , UAM::Context::MINIMAL     |
                               UAM::Context::RECOMMANDED },

   { UAMA::TOGGLE_VIDEO      , UAM::Context::ADVANCED    },
   { UAMA::ADD_CONTACT       , UAM::Context::MANAGEMENT  },
   { UAMA::ADD_TO_CONTACT    , UAM::Context::MANAGEMENT  },
   { UAMA::DELETE_CONTACT    , UAM::Context::MANAGEMENT  },
   { UAMA::EMAIL_CONTACT     , UAM::Context::CONTACT     },
   { UAMA::COPY_CONTACT      , UAM::Context::MANAGEMENT  },
   { UAMA::BOOKMARK          , UAM::Context::MANAGEMENT  },
   { UAMA::VIEW_CHAT_HISTORY , UAM::Context::MANAGEMENT  },
   { UAMA::ADD_CONTACT_METHOD, UAM::Context::MANAGEMENT  },
   { UAMA::CALL_CONTACT      , UAM::Context::CONTACT     },
   { UAMA::REMOVE_HISTORY    , UAM::Context::MANAGEMENT  },
};

const Matrix1D< UAMA, FlagPack<UAM::Asset>> UserActionModelPrivate::availableByAsset = {
   { UAMA::ACCEPT            , UAM::Asset::CALL           },
   { UAMA::HOLD              , UAM::Asset::CALL           },
   { UAMA::MUTE_AUDIO        , UAM::Asset::CALL           },
   { UAMA::MUTE_VIDEO        , UAM::Asset::CALL           },
   { UAMA::SERVER_TRANSFER   , UAM::Asset::CALL           },
   { UAMA::RECORD            , UAM::Asset::CALL           },
   { UAMA::HANGUP            , UAM::Asset::CALL           },
   { UAMA::JOIN              , UAM::Asset::CALL           },
   { UAMA::ADD_NEW           , UAM::Asset::CALL           },
   { UAMA::TOGGLE_VIDEO      , UAM::Asset::CALL           },
   { UAMA::ADD_CONTACT       , UAM::Asset::PERSON         },
   { UAMA::ADD_TO_CONTACT    , UAM::Asset::CONTACT_METHOD },
   { UAMA::DELETE_CONTACT    , UAM::Asset::PERSON         },
   { UAMA::EMAIL_CONTACT     , UAM::Asset::PERSON         },
   { UAMA::COPY_CONTACT      , UAM::Asset::PERSON         },
   { UAMA::BOOKMARK          , UAM::Asset::CONTACT_METHOD },
   { UAMA::VIEW_CHAT_HISTORY , UAM::Asset::CONTACT_METHOD },
   { UAMA::ADD_CONTACT_METHOD, UAM::Asset::PERSON         },
   { UAMA::CALL_CONTACT      , UAM::Asset::PERSON         },
   { UAMA::REMOVE_HISTORY    , UAM::Asset::CALL           },
};

UserActionModelPrivate::UserActionModelPrivate(UserActionModel* parent, const FlagPack<UAM::Context>& c) : QObject(parent),q_ptr(parent),
m_pCall(nullptr), m_pActiveModel(nullptr), m_fContext(c)
{
   //Init the default names
   m_ActionNames = {
      { UAMA::ACCEPT            , QObject::tr("Accept"                 )},
      { UAMA::HOLD              , QObject::tr("Hold"                   )},
      { UAMA::MUTE_AUDIO        , QObject::tr("Mute audio"             )},
      { UAMA::MUTE_VIDEO        , QObject::tr("Mute video"             )},
      { UAMA::SERVER_TRANSFER   , QObject::tr("Server transfer"        )},
      { UAMA::RECORD            , QObject::tr("Record"                 )},
      { UAMA::HANGUP            , QObject::tr("Hangup"                 )},
      { UAMA::JOIN              , QObject::tr("Join"                   )},
      { UAMA::ADD_NEW           , QObject::tr("Add new"                )},
      { UAMA::TOGGLE_VIDEO      , QObject::tr("Toggle video"           )},
      { UAMA::ADD_CONTACT       , QObject::tr("Add a contact"          )},
      { UAMA::ADD_TO_CONTACT    , QObject::tr("Add to existing contact")},
      { UAMA::DELETE_CONTACT    , QObject::tr("Delete contact"         )},
      { UAMA::EMAIL_CONTACT     , QObject::tr("Email contact"          )},
      { UAMA::COPY_CONTACT      , QObject::tr("Copy contact"           )},
      { UAMA::BOOKMARK          , QObject::tr("Add bookmark"           )},
      { UAMA::VIEW_CHAT_HISTORY , QObject::tr("View chat history"      )},
      { UAMA::ADD_CONTACT_METHOD, QObject::tr("Add phone number"       )},
      { UAMA::CALL_CONTACT      , QObject::tr("Call again"             )},
      { UAMA::REMOVE_HISTORY    , QObject::tr("Remove"                 )},
   };

   m_CurrentActionsState = {
      { UAMA::ACCEPT            , Qt::Unchecked},
      { UAMA::HOLD              , Qt::Unchecked},
      { UAMA::MUTE_AUDIO        , Qt::Unchecked},
      { UAMA::MUTE_VIDEO        , Qt::Unchecked},
      { UAMA::SERVER_TRANSFER   , Qt::Unchecked},
      { UAMA::RECORD            , Qt::Unchecked},
      { UAMA::HANGUP            , Qt::Unchecked},
      { UAMA::JOIN              , Qt::Unchecked},
      { UAMA::ADD_NEW           , Qt::Unchecked},
      { UAMA::TOGGLE_VIDEO      , Qt::Unchecked},
      { UAMA::ADD_CONTACT       , Qt::Unchecked},
      { UAMA::ADD_TO_CONTACT    , Qt::Unchecked},
      { UAMA::DELETE_CONTACT    , Qt::Unchecked},
      { UAMA::EMAIL_CONTACT     , Qt::Unchecked},
      { UAMA::COPY_CONTACT      , Qt::Unchecked},
      { UAMA::BOOKMARK          , Qt::Unchecked},
      { UAMA::VIEW_CHAT_HISTORY , Qt::Unchecked},
      { UAMA::ADD_CONTACT_METHOD, Qt::Unchecked},
      { UAMA::CALL_CONTACT      , Qt::Unchecked},
      { UAMA::REMOVE_HISTORY    , Qt::Unchecked},
   };
}

#undef UAMA
#undef UAM

/**
 * Create an UserActionModel around a single call. This wont take advantage
 * of the multiselection feature.
 */
UserActionModel::UserActionModel(Call* parent, const FlagPack<UserActionModel::Context> c) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this,c))
{
   Q_ASSERT(parent != nullptr);
   Q_ASSERT(parent->state() != Call::State::OVER);
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALL;
   d_ptr->m_pCall = parent;

   connect(AccountModel::instance(), SIGNAL(accountStateChanged(Account*,Account::RegistrationState)), d_ptr.data(), SLOT(slotStateChanged()));
   d_ptr->updateActions();
}

/**
 * Create an UserActionModel around the CallModel selected call(s)
 */
UserActionModel::UserActionModel(CallModel* parent, const FlagPack<UserActionModel::Context> c) : QAbstractListModel(parent),d_ptr(new UserActionModelPrivate(this,c))
{
   Q_ASSERT(parent != nullptr);
   d_ptr->m_Mode = UserActionModelPrivate::UserActionModelMode::CALLMODEL;
   d_ptr->m_SelectionState = UserActionModelPrivate::SelectionState::UNIQUE;

   connect(parent->selectionModel(), &QItemSelectionModel::currentRowChanged , d_ptr.data(), &UserActionModelPrivate::updateActions);
   connect(parent->selectionModel(), &QItemSelectionModel::selectionChanged  , d_ptr.data(), &UserActionModelPrivate::updateActions);
   connect(parent,                   &CallModel::callStateChanged            , d_ptr.data(), &UserActionModelPrivate::updateActions);
   connect(parent,                   &CallModel::mediaStateChanged           , d_ptr.data(), &UserActionModelPrivate::updateActions);
   connect(AccountModel::instance(), &AccountModel::accountStateChanged      , d_ptr.data(), &UserActionModelPrivate::updateActions);
   d_ptr->updateActions();
}

UserActionModel::~UserActionModel()
{

}

QHash<int,QByteArray> UserActionModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

QVariant UserActionModel::data(const QModelIndex& idx, int role ) const
{
   if ((!idx.isValid()) || !(idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
      return QVariant();

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx.row());

   switch(role) {
      case Qt::DisplayRole:
         return d_ptr->m_ActionNames[action];
      case Qt::CheckStateRole:
         if (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] != UserActionModel::ActionStatfulnessLevel::UNISTATE)
            return d_ptr->m_CurrentActionsState[action];
         break;
      case Qt::DecorationRole: {
         UserActionElement state;
         state.action     = action       ;
         state.calls      = {}           ;
         state.checkState = Qt::Unchecked;
         return Interfaces::pixmapManipulator().userActionIcon(state);
      }
      case UserActionModel::Role::ACTION:
         return QVariant::fromValue(static_cast<Action>(idx.row()));
   };

   return QVariant();
}

int UserActionModel::rowCount(const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : static_cast<int>(Action::COUNT__);
}

///For now, this model probably wont be used that way
Qt::ItemFlags UserActionModel::flags(const QModelIndex& idx ) const
{
   if ((!idx.isValid()) || !(idx.row()>=0 && idx.row() < enum_class_size<UserActionModel::Action>()))
      return Qt::NoItemFlags;

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx.row());

   return (d_ptr->m_CurrentActions[action] ? (Qt::ItemIsEnabled | Qt::ItemIsSelectable) : Qt::NoItemFlags)
      | (d_ptr->actionStatefulness[action][d_ptr->m_SelectionState] != UserActionModel::ActionStatfulnessLevel::UNISTATE
      ? Qt::ItemIsUserCheckable : Qt::NoItemFlags);
}

// ///This model is read only
bool UserActionModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED( index )
   Q_UNUSED( value )
   Q_UNUSED( role  )
   return false;
}

bool UserActionModel::isActionEnabled( UserActionModel::Action action ) const
{
   return d_ptr->availableActionMap[action][d_ptr->m_pCall->state()];
}

void UserActionModelPrivate::slotStateChanged()
{
   emit q_ptr->actionStateChanged();
}

void UserActionModelPrivate::updateCheckMask(int& ret, UserActionModel::Action action, const Call* c)
{
   switch(action) {
      case UserActionModel::Action::ACCEPT          :
         ret += 0;
         break;
      case UserActionModel::Action::HOLD            :
         ret += c->state() == Call::State::HOLD? 100 : 1;
         break;
      case UserActionModel::Action::MUTE_AUDIO      : {
         auto a = c->firstMedia<Media::Audio>(Media::Media::Direction::OUT);
         ret += a && a->state() == Media::Media::State::MUTED ? 100 : 1;
         }
         break;
      case UserActionModel::Action::MUTE_VIDEO      : {
         auto v = c->firstMedia<Media::Video>(Media::Media::Direction::OUT);
         ret += v && v->state() == Media::Media::State::MUTED ? 100 : 1;
         }
         break;
      case UserActionModel::Action::SERVER_TRANSFER :
         ret += c->state() == Call::State::TRANSFERRED? 100 : 1;
         break;
      case UserActionModel::Action::RECORD          :
         ret += c->isRecording(Media::Media::Type::AUDIO,Media::Media::Direction::OUT) ? 100 : 1;
         break;
      case UserActionModel::Action::HANGUP          :
         ret += 0;
         break;
      case UserActionModel::Action::JOIN            :
         ret += 0;
         break;
      case UserActionModel::Action::ADD_NEW         :
         ret += 0;
         break;
      case UserActionModel::Action::TOGGLE_VIDEO      :
      case UserActionModel::Action::ADD_CONTACT       :
      case UserActionModel::Action::ADD_TO_CONTACT    :
      case UserActionModel::Action::DELETE_CONTACT    :
      case UserActionModel::Action::EMAIL_CONTACT     :
      case UserActionModel::Action::COPY_CONTACT      :
      case UserActionModel::Action::BOOKMARK          :
      case UserActionModel::Action::VIEW_CHAT_HISTORY :
      case UserActionModel::Action::ADD_CONTACT_METHOD:
      case UserActionModel::Action::CALL_CONTACT      :
      case UserActionModel::Action::REMOVE_HISTORY    :
      case UserActionModel::Action::COUNT__:
         break;
   };

   //Avoid the noise
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wswitch-enum"
   //Update the labels
   switch (action) {
      case UserActionModel::Action::ACCEPT          :
         switch(c->state()) {
            case Call::State::DIALING        :
               m_ActionNames.setAt(UserActionModel::Action::ACCEPT, QObject::tr("Call"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::ACCEPT,  QObject::tr("Accept"));
               break;
         }
         break;
      case UserActionModel::Action::HOLD            :
         switch(c->state()) {
            case Call::State::HOLD           :
            case Call::State::CONFERENCE_HOLD:
            case Call::State::TRANSF_HOLD    :
               m_ActionNames.setAt(UserActionModel::Action::HOLD, QObject::tr("Unhold"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::HOLD, QObject::tr("Hold"));
               break;
         }
         break;
      case UserActionModel::Action::HANGUP          :
         switch(c->state()) {
            case Call::State::DIALING        :
            case Call::State::NEW            :
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Cancel"));
               break;
            case Call::State::FAILURE        :
            case Call::State::ERROR          :
            case Call::State::COUNT__        :
            case Call::State::INITIALIZATION :
            case Call::State::BUSY           :
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Remove"));
               break;
            default:
               m_ActionNames.setAt(UserActionModel::Action::HANGUP, QObject::tr("Hangup"));
               break;
         }
         break;
      case UserActionModel::Action::JOIN            :
      case UserActionModel::Action::ADD_NEW         :
      case UserActionModel::Action::COUNT__         :
      case UserActionModel::Action::MUTE_AUDIO      :
      case UserActionModel::Action::MUTE_VIDEO      :
      case UserActionModel::Action::SERVER_TRANSFER :
      case UserActionModel::Action::RECORD          :
      case UserActionModel::Action::TOGGLE_VIDEO      :
      case UserActionModel::Action::ADD_CONTACT       :
      case UserActionModel::Action::ADD_TO_CONTACT    :
      case UserActionModel::Action::DELETE_CONTACT    :
      case UserActionModel::Action::EMAIL_CONTACT     :
      case UserActionModel::Action::COPY_CONTACT      :
      case UserActionModel::Action::BOOKMARK          :
      case UserActionModel::Action::VIEW_CHAT_HISTORY :
      case UserActionModel::Action::ADD_CONTACT_METHOD:
      case UserActionModel::Action::CALL_CONTACT      :
      case UserActionModel::Action::REMOVE_HISTORY    :
         break;
   }
   #pragma GCC diagnostic pop
}

bool UserActionModelPrivate::updateByCall(UserActionModel::Action action, const Call* c)
{
   Account* a = c->account() ? c->account() : AvailableAccountModel::instance()->currentDefaultAccount();
   return (!c) ? false : (
      availableActionMap        [action] [c->state()             ] &&
      availableAccountActionMap [action] [a->registrationState() ] &&
      multi_call_options        [action] [m_SelectionState       ] &&
      availableProtocolActions  [action] [a->protocol()          ] &&
      actionContext             [action] & m_fContext              //
   );
}

bool UserActionModelPrivate::updateAction(UserActionModel::Action action)
{
   int state = 0;
   switch(m_Mode) {
      case UserActionModelMode::CALL:
         updateCheckMask(state,action,m_pCall);
         m_CurrentActionsState.setAt(action,  state / 100 ? Qt::Checked : Qt::Unchecked);

         return updateByCall(action, m_pCall);
      case UserActionModelMode::CALLMODEL: {
         bool ret = true;

         m_SelectionState = CallModel::instance()->selectionModel()->selectedRows().size() > 1 ? SelectionState::MULTI : SelectionState::UNIQUE;

         //Aggregate and reduce the action state for each selected calls
         if (CallModel::instance()->selectionModel()->selectedRows().size()) {
            for (const QModelIndex& idx : CallModel::instance()->selectionModel()->selectedRows()) {
               const Call* c = qvariant_cast<Call*>(idx.data(static_cast<int>(Call::Role::Object)));
               updateCheckMask    ( state ,action, c );
               ret &= updateByCall( action       , c );
            }
         }
         else {
            Account* a = AvailableAccountModel::instance()->currentDefaultAccount();
            ret = multi_call_options[action][UserActionModelPrivate::SelectionState::NONE]
               && (a?availableAccountActionMap[action][a->registrationState()]:false);
         }

         //Detect if the multiple selection has mismatching item states, disable it if necessary
         m_CurrentActionsState.setAt(action, (state % 100 && state / 100) ? Qt::PartiallyChecked : (state / 100 ? Qt::Checked : Qt::Unchecked));
         return ret && (m_CurrentActionsState[action] != Qt::PartiallyChecked || heterogenous_call_options[action]);
      }
   };
   return false;
}

void UserActionModelPrivate::updateActions()
{
   for (UserActionModel::Action action : EnumIterator<UserActionModel::Action>())
      m_CurrentActions[action] = updateAction(action);
   emit q_ptr->dataChanged(q_ptr->index(0,0),q_ptr->index(enum_class_size<UserActionModel::Action>()-1,0));
}

uint UserActionModel::relativeIndex( UserActionModel::Action action ) const
{
   int i(0),ret(0);
   while (i != static_cast<int>(action) && i < enum_class_size<UserActionModel::Action>()) {
      ret += isActionEnabled(static_cast<UserActionModel::Action>(i))?1:0;
      i++;
   }
   return ret;
}

bool UserActionModel::execute(const UserActionModel::Action action) const
{
   /*
    * The rational behind not expanding Call::Actions to handle this are:
    *
    * 1) There is some actions that apply _only_ to multiple calls
    * 2) There is still a need to abstract the multi call use case
    */

   //1) Build the list of all selected calls
   QList<Call*> selected;
   switch(d_ptr->m_Mode) {
      case UserActionModelPrivate::UserActionModelMode::CALL:
         selected << d_ptr->m_pCall;
         break;
      case UserActionModelPrivate::UserActionModelMode::CALLMODEL: {
         for (const QModelIndex& idx : CallModel::instance()->selectionModel()->selectedRows()) {
            Call* c = qvariant_cast<Call*>(idx.data(static_cast<int>(Call::Role::Object)));
            if (c)
               selected << c;
         }
         break;
      }
   };

   //2) Perform the actions
   switch(action) {
      case UserActionModel::Action::ACCEPT          :
         if (UserActions::accept(selected))
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::HOLD            :
         switch(d_ptr->m_CurrentActionsState[UserActionModel::Action::HOLD]) {
            case Qt::Checked:
               if (UserActions::unhold(selected))
                  d_ptr->updateActions();
               break;
            case Qt::Unchecked:
               if (UserActions::hold(selected))
                  d_ptr->updateActions();
               break;
            case Qt::PartiallyChecked:
               //Invalid
               break;
         };
         break;
      case UserActionModel::Action::MUTE_AUDIO      :
         {
            bool mute = d_ptr->m_CurrentActionsState[UserActionModel::Action::MUTE_AUDIO] != Qt::Checked;
            UserActions::muteAudio(selected, mute);
            d_ptr->updateActions();
         }
         break;
      case UserActionModel::Action::MUTE_VIDEO      :
         {
            bool mute = d_ptr->m_CurrentActionsState[UserActionModel::Action::MUTE_VIDEO] != Qt::Checked;
            UserActions::muteVideo(selected, mute);
            d_ptr->updateActions();
         }
         break;
      case UserActionModel::Action::SERVER_TRANSFER :
         UserActions::transfer(selected);
         break;
      case UserActionModel::Action::RECORD          :
         if (UserActions::recordAudio(selected)) //TODO handle other recording types
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::HANGUP          :
         if (UserActions::hangup(selected))
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::JOIN            :
         //TODO unimplemented
         break;
      case UserActionModel::Action::ADD_NEW         :
         if (UserActions::addNew())
            d_ptr->updateActions();
         break;
      case UserActionModel::Action::TOGGLE_VIDEO      :
      case UserActionModel::Action::ADD_CONTACT       :
      case UserActionModel::Action::ADD_TO_CONTACT    :
      case UserActionModel::Action::DELETE_CONTACT    :
      case UserActionModel::Action::EMAIL_CONTACT     :
      case UserActionModel::Action::COPY_CONTACT      :
      case UserActionModel::Action::BOOKMARK          :
      case UserActionModel::Action::VIEW_CHAT_HISTORY :
      case UserActionModel::Action::ADD_CONTACT_METHOD:
      case UserActionModel::Action::CALL_CONTACT      :
      case UserActionModel::Action::REMOVE_HISTORY    :
      case UserActionModel::Action::COUNT__:
         break;
   };

   return true; //TODO handle errors
}

UserActionModel* UserActionModel::operator<<(UserActionModel::Action& action)
{
   execute(action);
   return this;
}


UserActionModel* operator<<(UserActionModel* m,UserActionModel::Action action)
{
   return (!m)? nullptr : (*m) << action;
}

/**
 * Execute an action
 * @param idx A model index. It can be from proxies.
 */
bool UserActionModel::execute(const QModelIndex& idx) const
{
   QModelIndex idx2 = idx;

   //Make this API a little more user friendly and unwind the proxies
   QAbstractProxyModel* m = nullptr;
   while (idx2.model() != this && idx2.isValid()) {
      m = qobject_cast<QAbstractProxyModel*>(const_cast<QAbstractItemModel*>(idx2.model()));
      if (m)
         idx2 = m->mapToSource(idx2);
      else
         break;
   }

   if (!idx2.isValid())
      return false;

   UserActionModel::Action action = static_cast<UserActionModel::Action>(idx2.row());
   return execute(action);
}

///Get a model filter with only the available actions
QSortFilterProxyModel* UserActionModel::activeActionModel() const
{
   if (!d_ptr->m_pActiveModel)
      d_ptr->m_pActiveModel = new ActiveUserActionModel(const_cast<UserActionModel*>(this));
   return d_ptr->m_pActiveModel;
}

//Do not display disabled account
bool ActiveUserActionModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   return sourceModel()->index(source_row,0,source_parent).flags() & Qt::ItemIsEnabled;
}

#include <useractionmodel.moc>

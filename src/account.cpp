/****************************************************************************
 *   Copyright (C) 2009-2018 Savoir-faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *            Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
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
#include "account.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>

//Ring daemon
#include <account_const.h>
#include <presence_const.h>

//Ring lib
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "dbus/videomanager.h"
#include "dbus/presencemanager.h"
#include "globalinstances.h"
#include "interfaces/accountlistcolorizeri.h"
#include "certificate.h"
#include "certificatemodel.h"
#include "accountmodel.h"
#include "private/certificatemodel_p.h"
#include "private/account_p.h"
#include "private/accountmodel_p.h"
#include "credentialmodel.h"
#include "ciphermodel.h"
#include "protocolmodel.h"
#include "bootstrapmodel.h"
#include "ringdevicemodel.h"
#include "contactrequest.h"
#include "person.h"
#include "profile.h"
#include "profilemodel.h"
#include "pendingcontactrequestmodel.h"
#include "private/pendingcontactrequestmodel_p.h"
#include "accountstatusmodel.h"
#include "codecmodel.h"
#include "networkinterfacemodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"
#include "presencestatusmodel.h"
#include "uri.h"
#include "private/vcardutils.h"
#include "mime.h"
#include "namedirectory.h"
#include "securityevaluationmodel.h"
#include "daemoncertificatecollection.h"
#include "private/securityevaluationmodel_p.h"
#include "extensions/securityevaluationextension.h"
#include "bannedcontactmodel.h"

// define
#define TO_BOOL ?"true":"false"
#define IS_TRUE == "true"


#define AP &AccountPrivate
#define EA Account::EditAction
#define ES Account::EditState

static EnumClassReordering<Account::EditAction> co =
{                                 EA::NOTHING,  EA::EDIT  , EA::RELOAD ,  EA::SAVE  , EA::REMOVE , EA::MODIFY   , EA::CANCEL     };
const Matrix2D<Account::EditState, Account::EditAction, account_function> AccountPrivate::stateMachineActionsOnState = {
{ES::READY               ,{{co, { AP::nothing, AP::edit   , AP::reload , AP::nothing, AP::remove , AP::modify   , AP::nothing }}}},
{ES::EDITING             ,{{co, { AP::nothing, AP::nothing, AP::outdate, AP::nothing, AP::remove , AP::modify   , AP::cancel  }}}},
{ES::OUTDATED            ,{{co, { AP::nothing, AP::nothing, AP::nothing, AP::nothing, AP::remove , AP::reloadMod, AP::reload  }}}},
{ES::NEW                 ,{{co, { AP::nothing, AP::nothing, AP::nothing, AP::save   , AP::remove , AP::nothing  , AP::nothing }}}},
{ES::MODIFIED_INCOMPLETE ,{{co, { AP::nothing, AP::nothing, AP::nothing, AP::save   , AP::remove , AP::modify   , AP::reload  }}}},
{ES::MODIFIED_COMPLETE   ,{{co, { AP::nothing, AP::nothing, AP::nothing, AP::save   , AP::remove , AP::modify   , AP::reload  }}}},
{ES::REMOVED             ,{{co, { AP::nothing, AP::nothing, AP::nothing, AP::nothing, AP::nothing, AP::nothing  , AP::cancel  }}}}
};

#undef ES
#undef EA
#undef AP

//Host the current highest interal identifier. The internal id is used for some bitmasks
//when objects have a different status for each account
static uint p_sAutoIncrementId = 0;

AccountPrivate::AccountPrivate(Account* acc) : QObject(acc),q_ptr(acc),m_pCredentials(nullptr),m_pCodecModel(nullptr),
m_LastErrorCode(-1),m_VoiceMailCount(0),m_CurrentState(Account::EditState::READY),
m_pAccountNumber(nullptr),m_pKeyExchangeModel(nullptr),m_pSecurityEvaluationModel(nullptr),m_pTlsMethodModel(nullptr),
m_pCaCert(nullptr),m_pTlsCert(nullptr),m_isLoaded(true),m_pCipherModel(nullptr),
m_pStatusModel(nullptr),m_LastTransportCode(0),m_RegistrationState(Account::RegistrationState::UNREGISTERED),
m_UseDefaultPort(false),m_pProtocolModel(nullptr),m_pBootstrapModel(nullptr),m_RemoteEnabledState(false),
m_pKnownCertificates(nullptr),
m_pBannedCertificates(nullptr), m_pAllowedCertificates(nullptr),m_InternalId(++p_sAutoIncrementId),
m_pNetworkInterfaceModel(nullptr),m_pAllowedCerts(nullptr),m_pBannedCerts(nullptr),m_pPendingContactRequestModel(nullptr),
m_pRingDeviceModel(nullptr)
{
}

void AccountPrivate::changeState(Account::EditState state) {
   const Account::EditState previous = m_CurrentState;
   m_CurrentState = state;

   if (state != previous)
      emit q_ptr->editStateChanged(state, previous);

   emit q_ptr->changed(q_ptr);
}

///Constructors
Account::Account():ItemBase(&AccountModel::instance()),d_ptr(new AccountPrivate(this))
{
}

///Build an account from it'id
Account* Account::buildExistingAccountFromId(const QByteArray& _accountId)
{
//    qDebug() << "Building an account from id: " << _accountId;
   Account* a = new Account();
   a->d_ptr->m_AccountId = _accountId;
   a->d_ptr->setObjectName(_accountId);
   a->d_ptr->m_RemoteEnabledState = true;

   a->performAction(Account::EditAction::RELOAD);

   //If a placeholder exist for this account, upgrade it
   if (auto place_holder = AccountModel::instance().findPlaceHolder(_accountId))
       place_holder->d_ptr->merge(a);

   //Load the pending trust requests
   if (a->protocol() == Account::Protocol::RING) {
      const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(a->id())};
      for (const auto& tr_info : pending_tr) {
         auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();
         auto ringID = tr_info[DRing::Account::TrustRequest::FROM];
         auto timeReceived = tr_info[DRing::Account::TrustRequest::RECEIVED].toInt();

         auto contactMethod = PhoneDirectoryModel::instance().getNumber(ringID, a);
         auto person = VCardUtils::mapToPersonFromReceivedProfile(contactMethod, payload);

         auto contactRequest = new ContactRequest(a, person, ringID, timeReceived);
         a->pendingContactRequestModel()->d_ptr->addRequest(contactRequest);
      }
   }

   // Connects the account to the signal of the model.
   connect(a->pendingContactRequestModel() , &PendingContactRequestModel::requestAccepted, [a] (ContactRequest* r) {
      emit a->contactRequestAccepted(r);
   });

   // Load the contacts associated from the daemon and create the cms.
   const auto account_contacts = static_cast<QVector<QMap<QString, QString>>>(ConfigurationManager::instance()
                                                                                          .getContacts(a->id().data()));

   if (a->protocol() == Account::Protocol::RING) {
      for (auto contact_info : account_contacts) {
          auto cm = PhoneDirectoryModel::instance().getNumber(contact_info["id"], a);
          if (contact_info["banned"] IS_TRUE) {
             a->bannedContactModel()->add(cm);
          } else {
             cm->setConfirmed(contact_info["confirmed"] IS_TRUE);
             a->d_ptr->m_NumbersFromDaemon << cm;
          }
      }
   }

   //Load the tracked buddies
   const VectorMapStringString subscriptions = PresenceManager::instance().getSubscriptions(a->id());
   foreach(auto subscription, subscriptions){
       ContactMethod* tracked_buddy = PhoneDirectoryModel::instance().getNumber(subscription[DRing::Presence::BUDDY_KEY], a);
       bool tracked_buddy_present = subscription[DRing::Presence::STATUS_KEY].compare(DRing::Presence::ONLINE_KEY) == 0;
       tracked_buddy->setTracked(true);
       tracked_buddy->setPresent(tracked_buddy_present);
   }

   return a;
} //buildExistingAccountFromId

///Build an account from it's name / alias
Account* Account::buildNewAccountFromAlias(Account::Protocol proto, const QString& alias)
{
   qDebug() << "Building an account from alias: " << alias;
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   Account* a = new Account();
   a->setProtocol(proto);
   a->d_ptr->m_hAccountDetails.clear();
   a->d_ptr->m_hAccountDetails[DRing::Account::ConfProperties::ENABLED] = "false";
   a->d_ptr->m_pAccountNumber = nullptr;
   MapStringString tmp;
   switch (proto) {
      case Account::Protocol::SIP:
         tmp = configurationManager.getAccountTemplate(DRing::Account::ProtocolNames::SIP);
         break;
      case Account::Protocol::RING:
         tmp = configurationManager.getAccountTemplate(DRing::Account::ProtocolNames::RING);
         break;
      case Account::Protocol::COUNT__:
         break;
   }
   QMutableMapIterator<QString, QString> iter(tmp);
   while (iter.hasNext()) {
      iter.next();
      a->d_ptr->m_hAccountDetails[iter.key()] = iter.value();
   }

   if (proto == Account::Protocol::RING)
   {
       /* Set LRC-provided bootstrap servers */
       a->bootstrapModel() << BootstrapModel::EditAction::RESET;
       a->bootstrapModel() << BootstrapModel::EditAction::SAVE;
   }
   else
   {
       a->setHostname(a->d_ptr->m_hAccountDetails[DRing::Account::ConfProperties::HOSTNAME]);
   }

   a->d_ptr->setAccountProperty(DRing::Account::ConfProperties::ALIAS,alias);
   a->d_ptr->m_RemoteEnabledState = a->isEnabled();
   //a->setObjectName(a->id());
   return a;
}

///Destructor
Account::~Account()
{
   disconnect();
   if (d_ptr->m_pCredentials) delete d_ptr->m_pCredentials ;
   if (d_ptr->m_pCodecModel) delete d_ptr->m_pCodecModel   ;
}

/*****************************************************************************
 *                                                                           *
 *                                   Slots                                   *
 *                                                                           *
 ****************************************************************************/

void AccountPrivate::slotPresentChanged(bool present)
{
   Q_UNUSED(present)
   emit q_ptr->changed(q_ptr);
}

void AccountPrivate::slotPresenceMessageChanged(const QString& message)
{
   Q_UNUSED(message)
   emit q_ptr->changed(q_ptr);
}

void AccountPrivate::slotUpdateCertificate()
{
   Certificate* cert = qobject_cast<Certificate*>(sender());
   if (cert) {
      switch (cert->type()) {
         case Certificate::Type::AUTHORITY:
            if (accountDetail(DRing::Account::ConfProperties::TLS::CA_LIST_FILE) != cert->path())
               setAccountProperty(DRing::Account::ConfProperties::TLS::CA_LIST_FILE, cert->path());
            break;
         case Certificate::Type::USER:
            if (accountDetail(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE) != cert->path())
               setAccountProperty(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE, cert->path());
            break;
         case Certificate::Type::PRIVATE_KEY:
            if (accountDetail(DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE) != cert->path())
               setAccountProperty(DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE, cert->path());
            break;
         case Certificate::Type::NONE:
         case Certificate::Type::CALL:
            break;
      };
   }
}

/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

///IS this account new
bool Account::isNew() const
{
   return (d_ptr->m_AccountId == nullptr) || d_ptr->m_AccountId.isEmpty();
}

///Is this an IP2IP account
bool Account::isIp2ip() const
{
    return protocol() == Protocol::SIP && hostname().isEmpty();
}

///Get this account ID
const QByteArray Account::id() const
{
   if (isNew()) {
      qDebug() << "Error : getting AccountId of a new account.";
      return QByteArray(); //WARNING May explode
   }

   return d_ptr->m_AccountId;
}

///Get the device ID
QString Account::deviceId() const
{
    return d_ptr->accountDetail(DRing::Account::ConfProperties::RING_DEVICE_ID);
}

///Get current state
const QString Account::toHumanStateName() const
{
   const QString s = d_ptr->m_hAccountDetails[DRing::Account::ConfProperties::Registration::STATUS];

                                                 //: Account state
   static const QString ready                  = tr("Ready"                    );
                                                 //: Account state
   static const QString registered             = tr("Registered"               );
                                                 //: Account state
   static const QString notRegistered          = tr("Not Registered"           );
                                                 //: Account state
   static const QString initializing           = tr("Initializing"             );
                                                 //: Account state
   static const QString trying                 = tr("Trying..."                );
                                                 //: Account state
   static const QString error                  = tr("Error"                    );
                                                 //: Account state
   static const QString authenticationFailed   = tr("Authentication Failed"    );
                                                 //: Account state
   static const QString networkUnreachable     = tr("Network unreachable"      );
                                                 //: Account state
   static const QString hostUnreachable        = tr("Host unreachable"         );
                                                 //: Account state
   static const QString stunConfigurationError = tr("Stun configuration error" );
                                                 //: Account state
   static const QString stunServerInvalid      = tr("Stun server invalid"      );
                                                 //: Account state
   static const QString serviceUnavailable     = tr("Service unavailable"      );
                                                 //: Account state
   static const QString notAcceptable          = tr("Unacceptable"             );
                                                 //: Account state
   static const QString invalid                = tr("Invalid"                  );
                                                 //: Account state
   static const QString requestTimeout         = tr("Request Timeout"          );

   if(s == DRing::Account::States::READY            )
      return ready                  ;
   if(s == DRing::Account::States::REGISTERED       )
      return registered             ;
   if(s == DRing::Account::States::INITIALIZING     )
      return initializing           ;
   if(s == DRing::Account::States::UNREGISTERED     )
      return notRegistered          ;
   if(s == DRing::Account::States::TRYING           )
      return trying                 ;
   if(s == DRing::Account::States::ERROR            )
      return d_ptr->m_LastErrorMessage.isEmpty()?error:d_ptr->m_LastErrorMessage;
   if(s == DRing::Account::States::ERROR_AUTH       )
      return authenticationFailed   ;
   if(s == DRing::Account::States::ERROR_NETWORK    )
      return networkUnreachable     ;
   if(s == DRing::Account::States::ERROR_HOST       )
      return hostUnreachable        ;
   if(s == DRing::Account::States::ERROR_CONF_STUN  )
      return stunConfigurationError ;
   if(s == DRing::Account::States::ERROR_EXIST_STUN )
      return stunServerInvalid      ;
   if(s == DRing::Account::States::ERROR_SERVICE_UNAVAILABLE )
      return serviceUnavailable     ;
   if(s == DRing::Account::States::ERROR_NOT_ACCEPTABLE      )
      return notAcceptable          ;
   if(s == DRing::Account::States::REQUEST_TIMEOUT           )
      return requestTimeout         ;
   return invalid                   ;
}

///Get an account detail
QString AccountPrivate::accountDetail(const QString& param) const
{
   if (!m_hAccountDetails.size()) {
      qDebug() << "The account details is not set";
      return QString(); //May crash, but better than crashing now
   }
   if (m_hAccountDetails.find(param) != m_hAccountDetails.end()) {
      return m_hAccountDetails[param];
   }
   else if (m_hAccountDetails.count() > 0) {
      if (param == DRing::Account::ConfProperties::ENABLED) //If an account is invalid, at least does not try to register it
         return AccountPrivate::RegistrationEnabled::NO;
      if (param == DRing::Account::ConfProperties::Registration::STATUS) { //If an account is new, then it is unregistered
         return DRing::Account::States::UNREGISTERED;
      }
      static QHash<QString,bool> alreadyWarned;
      if (!alreadyWarned[param]) {
         alreadyWarned[param] = true;
         qDebug() << "Account parameter \"" << param << "\" not found";
      }
      return QString();
   }
   else {
      qDebug() << "Account details not found, there is " << m_hAccountDetails.count() << " details available";
      return QString();
   }
} //accountDetail

///Get an account detail
QString Account::accountDetail(const QString& param) const
{
    return d_ptr->accountDetail(param);
}

///Get the alias
const QString Account::alias() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ALIAS);
}

///Return the model index of this item
QModelIndex Account::index() const
{
   //There is usually < 5 accounts, the loop may be faster than a hash for most users
   for (int i=0;i < AccountModel::instance().size();i++) {
      if (this == AccountModel::instance()[i]) {
         return AccountModel::instance().index(i,0);
      }
   }
   return QModelIndex();
}

///Return status color name
QString Account::stateColorName() const
{
   switch(registrationState()) {
      case RegistrationState::READY:
         return "darkGreen";
      case RegistrationState::UNREGISTERED:
         return "black";
      case RegistrationState::TRYING:
      case RegistrationState::INITIALIZING:
         return "orange";
      case RegistrationState::ERROR:
         return "red";
      case RegistrationState::COUNT__:
         break;
   };
   return QString();
}

///I
bool Account::isLoaded() const
{
   return d_ptr->m_isLoaded;
}

///Return status Qt color, QColor is not part of QtCore, use using the global variant
QVariant Account::stateColor() const
{
   return GlobalInstances::accountListColorizer().color(this);
}

///Create and return the credential model
CredentialModel* Account::credentialModel() const
{
   if (!d_ptr->m_pCredentials) {
      d_ptr->m_pCredentials = new CredentialModel(const_cast<Account*>(this));
      connect(d_ptr->m_pCredentials, &CredentialModel::primaryCredentialChanged,[this]() {
         Account* a = const_cast<Account*>(this);
         emit a->changed(a);
      });
   }
   return d_ptr->m_pCredentials;
}

///Create and return the audio codec model
CodecModel* Account::codecModel() const
{
   if (!d_ptr->m_pCodecModel) {
      d_ptr->m_pCodecModel = new CodecModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pCodecModel;
}

KeyExchangeModel* Account::keyExchangeModel() const
{
   if (!d_ptr->m_pKeyExchangeModel) {
      d_ptr->m_pKeyExchangeModel = new KeyExchangeModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pKeyExchangeModel;
}

CipherModel* Account::cipherModel() const
{
   if (!d_ptr->m_pCipherModel) {
      d_ptr->m_pCipherModel = new CipherModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pCipherModel;
}

AccountStatusModel* Account::statusModel() const
{
   if (!d_ptr->m_pStatusModel) {
      d_ptr->m_pStatusModel = new AccountStatusModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pStatusModel;
}

SecurityEvaluationModel* Account::securityEvaluationModel() const
{
   if (!d_ptr->m_pSecurityEvaluationModel) {
      d_ptr->m_pSecurityEvaluationModel = new SecurityEvaluationModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pSecurityEvaluationModel;
}

TlsMethodModel* Account::tlsMethodModel() const
{
   if (!d_ptr->m_pTlsMethodModel ) {
      d_ptr->m_pTlsMethodModel  = new TlsMethodModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pTlsMethodModel;
}

ProtocolModel* Account::protocolModel() const
{
   if (!d_ptr->m_pProtocolModel ) {
      d_ptr->m_pProtocolModel  = new ProtocolModel(const_cast<Account*>(this));
   }
   return d_ptr->m_pProtocolModel;
}

BootstrapModel* Account::bootstrapModel() const
{
   if (protocol() != Account::Protocol::RING)
      return nullptr;

   if (!d_ptr->m_pBootstrapModel ) {
      d_ptr->m_pBootstrapModel  = new BootstrapModel(const_cast<Account*>(this));
   }

   return d_ptr->m_pBootstrapModel;
}

RingDeviceModel* Account::ringDeviceModel() const
{
    if (!d_ptr->m_pRingDeviceModel)
      d_ptr->m_pRingDeviceModel = new RingDeviceModel(const_cast<Account*>(this));

   return d_ptr->m_pRingDeviceModel;
}

QAbstractItemModel* Account::knownCertificateModel() const
{
   if (!d_ptr->m_pKnownCertificates) {
      d_ptr->m_pKnownCertificates = CertificateModel::instance().d_ptr->createKnownList(this);
   }

   return d_ptr->m_pKnownCertificates;
}

QAbstractItemModel* Account::bannedCertificatesModel() const
{
   if (protocol() != Account::Protocol::RING || isNew())
      return nullptr;

   if (!d_ptr->m_pBannedCerts) {
      d_ptr->m_pBannedCerts = CertificateModel::instance().addCollection<DaemonCertificateCollection,Account*,DaemonCertificateCollection::Mode>(
         const_cast<Account*>(this),
         DaemonCertificateCollection::Mode::BANNED
      );
      d_ptr->m_pBannedCerts->load();
   }

   if (!d_ptr->m_pBannedCertificates) {
      d_ptr->m_pBannedCertificates = CertificateModel::instance().d_ptr->createBannedList(this);
   }

   return d_ptr->m_pBannedCertificates;
}

QAbstractItemModel* Account::allowedCertificatesModel() const
{
   if (protocol() != Account::Protocol::RING || isNew())
      return nullptr;

   if (!d_ptr->m_pAllowedCerts) {
      d_ptr->m_pAllowedCerts = CertificateModel::instance().addCollection<DaemonCertificateCollection,Account*,DaemonCertificateCollection::Mode>(
         const_cast<Account*>(this),
         DaemonCertificateCollection::Mode::ALLOWED
      );
      d_ptr->m_pAllowedCerts->load();
   }

   if (!d_ptr->m_pAllowedCertificates) {
      d_ptr->m_pAllowedCertificates = CertificateModel::instance().d_ptr->createAllowedList(this);
   }

   return d_ptr->m_pAllowedCertificates;
}

PendingContactRequestModel* Account::pendingContactRequestModel() const
{
   if (!d_ptr->m_pPendingContactRequestModel)
      d_ptr->m_pPendingContactRequestModel = new PendingContactRequestModel(const_cast<Account*>(this));

   return d_ptr->m_pPendingContactRequestModel;
}

BannedContactModel*
Account::bannedContactModel() const
{
   if (!d_ptr->m_pBannedContactModel)
      d_ptr->m_pBannedContactModel = new BannedContactModel(const_cast<Account*>(this));

   return d_ptr->m_pBannedContactModel;
}

NetworkInterfaceModel* Account::networkInterfaceModel() const
{
   if (!d_ptr->m_pNetworkInterfaceModel) {
      d_ptr->m_pNetworkInterfaceModel = new NetworkInterfaceModel(const_cast<Account*>(this));
   }

   return d_ptr->m_pNetworkInterfaceModel;
}

bool Account::isUsedForOutgogingCall() const
{
   return usageStats.haveCalled();
}

uint Account::totalCallCount() const
{
   return usageStats.totalCount();
}

uint Account::weekCallCount() const
{
   return usageStats.lastWeekCount();
}

uint Account::trimesterCallCount() const
{
   return usageStats.lastTrimCount();
}

time_t Account::lastUsed() const
{
   return usageStats.lastUsed();
}

/*******************************************************************************
 *                                                                             *
 *                                  Setters                                    *
 *                                                                             *
 ******************************************************************************/

void Account::setAlias(const QString& detail)
{
   const bool accChanged = detail != alias();
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ALIAS,detail);

   if (accChanged)
      emit aliasChanged(detail);
}

///Return the account hostname
QString Account::hostname() const
{
   return d_ptr->m_HostName;
}

///Return if the account is enabled
bool Account::isEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ENABLED) IS_TRUE;
}

///Return if the account should auto answer
bool Account::isAutoAnswer() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::AUTOANSWER) IS_TRUE;
}

//Return if the accounts needs to migrate
bool Account::needsMigration() const
{
    const MapStringString details = ConfigurationManager::instance().getVolatileAccountDetails(id());
    const QString status = details[DRing::Account::VolatileProperties::Registration::STATUS];
    return status == DRing::Account::States::ERROR_NEED_MIGRATION;
}

/**
 * return all ContactsMethod from @this account
 * @return Account::ContactMethods a.k.a. QVector<ContactMethod*>
 */
Account::ContactMethods&
Account::getContacts() const
{
    return d_ptr->m_NumbersFromDaemon;
}

///Return the account user name
QString Account::username() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::USERNAME);
}

//Return the account registered name
QString Account::registeredName() const
{
    const MapStringString details = ConfigurationManager::instance().getVolatileAccountDetails(id());
    const QString registeredName = details[DRing::Account::VolatileProperties::REGISTERED_NAME];
    return registeredName;
}

///Return the account mailbox address
QString Account::mailbox() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::MAILBOX);
}

///Return the account mailbox address
QString Account::proxy() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ROUTE);
}

///Return the name service URL
QString Account::nameServiceURL() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::RingNS::URI);
}

QString Account::password() const
{
   switch (protocol()) {
      case Account::Protocol::SIP:
         if (credentialModel()->primaryCredential(Credential::Type::SIP))
            return credentialModel()->primaryCredential(Credential::Type::SIP)->password();
         break;
      case Account::Protocol::RING:
         return tlsPassword();
      case Account::Protocol::COUNT__:
         break;
   };
   return "";
}

///Return the account security fallback
bool Account::isSrtpRtpFallback() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::SRTP::RTP_FALLBACK) IS_TRUE;
}

//Return if SRTP is enabled or not
bool Account::isSrtpEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::SRTP::ENABLED) IS_TRUE;
}

///Return if the account is using a STUN server
bool Account::isSipStunEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::STUN::ENABLED) IS_TRUE;
}

///Return the account STUN server
QString Account::sipStunServer() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::STUN::SERVER);
}

///Return when the account expire (require renewal)
int Account::registrationExpire() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Registration::EXPIRE).toInt();
}

///Return if the published address is the same as the local one
bool Account::isPublishedSameAsLocal() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::PUBLISHED_SAMEAS_LOCAL) IS_TRUE;
}

///Return the account published address
QString Account::publishedAddress() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::PUBLISHED_ADDRESS);
}

///Return the account published port
int Account::publishedPort() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::PUBLISHED_PORT).toUInt();
}

///Return the account tls password
QString Account::tlsPassword() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::PASSWORD);
}

///Return the account TLS port
int Account::bootstrapPort() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::DHT::PORT).toInt();
}

///Return the account TLS certificate authority list file
Certificate* Account::tlsCaListCertificate() const
{
   if (!d_ptr->m_pCaCert) {
      const QString& path = d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::CA_LIST_FILE);
      if (path.isEmpty())
         return nullptr;
      d_ptr->m_pCaCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::AUTHORITY);
      connect(d_ptr->m_pCaCert,SIGNAL(changed()),d_ptr.data(),SLOT(slotUpdateCertificate()));
   }
   return d_ptr->m_pCaCert;
}

///Return the account TLS certificate
Certificate* Account::tlsCertificate() const
{
   if (!d_ptr->m_pTlsCert) {
      const QString& path = d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE);
      if (path.isEmpty())
         return nullptr;
      d_ptr->m_pTlsCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::USER);
      connect(d_ptr->m_pTlsCert,SIGNAL(changed()),d_ptr.data(),SLOT(slotUpdateCertificate()));
   }
   return d_ptr->m_pTlsCert;
}

///Return the account private key
QString Account::tlsPrivateKey() const
{
   return tlsCertificate() ? tlsCertificate()->privateKeyPath() : QString();
}

///Return the account TLS server name
QString Account::tlsServerName() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::SERVER_NAME);
}

///Return the account negotiation timeout in seconds
int Account::tlsNegotiationTimeoutSec() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC).toInt();
}

///Return the account TLS verify server
bool Account::isTlsVerifyServer() const
{
   return (d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_SERVER) IS_TRUE);
}

///Return the account TLS verify client
bool Account::isTlsVerifyClient() const
{
   return (d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT) IS_TRUE);
}

///Return if it is required for the peer to have a certificate
bool Account::isTlsRequireClientCertificate() const
{
   return (d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE) IS_TRUE);
}

///Return the account TLS security is enabled
bool Account::isTlsEnabled() const
{
   return protocol() == Account::Protocol::RING || (d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::ENABLED) IS_TRUE);
}

///Return if the ringtone are enabled
bool Account::isRingtoneEnabled() const
{
   return (d_ptr->accountDetail(DRing::Account::ConfProperties::Ringtone::ENABLED) IS_TRUE);
}

///Return the account ringtone path
QString Account::ringtonePath() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Ringtone::PATH);
}

///Return the last error message received
QString Account::lastErrorMessage() const
{
   return statusModel()->lastErrorMessage();
}

///Return the last error code (useful for debugging)
int Account::lastErrorCode() const
{
   return statusModel()->lastErrorCode();
}

///Get the last transport error code, this is used to debug why registration failed
int Account::lastTransportErrorCode() const
{
   return d_ptr->m_LastTransportCode;
}

///Get the last transport error message, this is used to debug why registration failed
QString Account::lastTransportErrorMessage() const
{
   return d_ptr->m_LastTransportMessage;
}

///Return the account local port
int Account::localPort() const
{
   switch (protocol()) {
      case Account::Protocol::SIP:
         if (isTlsEnabled())
            return d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::LISTENER_PORT).toInt();
         else
            return d_ptr->accountDetail(DRing::Account::ConfProperties::LOCAL_PORT).toInt();
      case Account::Protocol::RING:
         return d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::LISTENER_PORT).toInt();
      case Account::Protocol::COUNT__:
         break;
   };
   return 0;
}

///Return the number of voicemails
int Account::voiceMailCount() const
{
   return d_ptr->m_VoiceMailCount;
}

///Return the account registration status
Account::RegistrationState Account::registrationState() const
{
   return d_ptr->m_RegistrationState;
}

///Return the last account SIP registration status
QString Account::lastSipRegistrationStatus() const
{
   return d_ptr->m_LastSipRegistrationStatus;
}

///Return the account type
Account::Protocol Account::protocol() const
{
   const QString str = d_ptr->accountDetail(DRing::Account::ConfProperties::TYPE);

   if (str.isEmpty() || str == DRing::Account::ProtocolNames::SIP)
      return Account::Protocol::SIP;
   else if (str == DRing::Account::ProtocolNames::RING)
      return Account::Protocol::RING;
   qDebug() << "Warning: unhandled protocol name" << str << ", defaulting to SIP";
   return Account::Protocol::SIP;
}

///Return the contact method associated with this account
ContactMethod* Account::contactMethod() const
{
   return d_ptr->m_pAccountNumber;
}

///Return the DTMF type
DtmfType Account::DTMFType() const
{
   QString type = d_ptr->accountDetail(DRing::Account::ConfProperties::DTMF_TYPE);
   return (type == "overrtp" || type.isEmpty())? DtmfType::OverRtp:DtmfType::OverSip;
}

bool Account::presenceStatus() const
{
   return d_ptr->m_pAccountNumber->isPresent();
}

QString Account::presenceMessage() const
{
   return d_ptr->m_pAccountNumber->presenceMessage();
}

bool Account::supportPresencePublish() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Presence::SUPPORT_PUBLISH) IS_TRUE;
}

bool Account::supportPresenceSubscribe() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Presence::SUPPORT_SUBSCRIBE) IS_TRUE;
}

bool Account::presenceEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Presence::ENABLED) IS_TRUE;
}

bool Account::isVideoEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::ENABLED) IS_TRUE;
}

int Account::videoPortMax() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::PORT_MAX).toInt();
}

int Account::videoPortMin() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Video::PORT_MIN).toInt();
}

int Account::audioPortMin() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Audio::PORT_MIN).toInt();
}

int Account::audioPortMax() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::Audio::PORT_MAX).toInt();
}

bool Account::isUpnpEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::UPNP_ENABLED) IS_TRUE;
}

bool Account::hasCustomUserAgent() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::HAS_CUSTOM_USER_AGENT) IS_TRUE;
}

QString Account::userAgent() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::USER_AGENT);
}

bool Account::useDefaultPort() const
{
   return d_ptr->m_UseDefaultPort;
}

bool Account::isTurnEnabled() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TURN::ENABLED) IS_TRUE;
}

QString Account::turnServer() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TURN::SERVER);
}

QString Account::turnServerUsername() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_UNAME);
}

QString Account::turnServerPassword() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_PWD);
}

QString Account::turnServerRealm() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::TURN::SERVER_REALM);
}

bool Account::hasProxy() const
{
   return proxy().size();
}

QString Account::displayName() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::DISPLAYNAME);
}

bool Account::archiveHasPassword() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ARCHIVE_HAS_PASSWORD) == "true";
}


QString Account::archivePassword() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ARCHIVE_PASSWORD);
}

QString Account::archivePin() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ARCHIVE_PIN);
}

QString Account::archivePath() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ARCHIVE_PATH);
}

bool Account::changePassword(const QString& currentPassword, const QString newPassword) const
{
    return ConfigurationManager::instance().changeAccountPassword(id(), currentPassword, newPassword);
}

bool Account::allowIncomingFromUnknown() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::DHT::PUBLIC_IN_CALLS) IS_TRUE;
}

bool Account::allowIncomingFromHistory() const
{
   if (protocol() != Account::Protocol::RING)
      return false;

   return d_ptr->accountDetail(DRing::Account::ConfProperties::ALLOW_CERT_FROM_HISTORY) IS_TRUE;
}

bool Account::allowIncomingFromContact() const
{
   if (protocol() != Account::Protocol::RING)
      return false;

   return d_ptr->accountDetail(DRing::Account::ConfProperties::ALLOW_CERT_FROM_CONTACT) IS_TRUE;
}

int Account::activeCallLimit() const
{
   return d_ptr->accountDetail(DRing::Account::ConfProperties::ACTIVE_CALL_LIMIT).toInt();
}

bool Account::hasActiveCallLimit() const
{
   return activeCallLimit() > -1;
}

bool Account::exportOnRing(const QString& password) const
{
    return ConfigurationManager::instance().exportOnRing(id(), password);
}


#define CAST(item) static_cast<int>(item)
QVariant Account::roleData(int role) const
{
   switch(role) {
      //Generic
      case Qt::DisplayRole:
      case Qt::EditRole:
      case CAST(Ring::Role::Name):
         return alias();
      case Qt::CheckStateRole:
         return QVariant(isEnabled() ? Qt::Checked : Qt::Unchecked);
      case Qt::BackgroundRole:
         return stateColor();
      case Qt::DecorationRole:
         return GlobalInstances::accountListColorizer().icon(this);

      //Specialized
      case CAST(Account::Role::Alias):
         return alias();
      case CAST(Account::Role::Proto):
         return CAST(protocol());
      case CAST(Account::Role::Hostname):
         return hostname();
      case CAST(Account::Role::Username):
         return username();
      case CAST(Account::Role::Mailbox):
         return mailbox();
      case CAST(Account::Role::Proxy):
         return proxy();
//       case Password:
//          return accountPassword();
      case CAST(Account::Role::TlsPassword):
         return tlsPassword();
      case CAST(Account::Role::TlsCaListCertificate):
         return tlsCaListCertificate()?tlsCaListCertificate()->path():QVariant();
      case CAST(Account::Role::TlsCertificate):
         return tlsCertificate()?tlsCertificate()->path():QVariant();
      case CAST(Account::Role::TlsPrivateKey):
        return tlsPrivateKey();
      case CAST(Account::Role::TlsServerName):
         return tlsServerName();
      case CAST(Account::Role::SipStunServer):
         return sipStunServer();
      case CAST(Account::Role::PublishedAddress):
         return publishedAddress();
      case CAST(Account::Role::RingtonePath):
         return ringtonePath();
      case CAST(Account::Role::RegistrationExpire):
         return registrationExpire();
      case CAST(Account::Role::TlsNegotiationTimeoutSec):
         return tlsNegotiationTimeoutSec();
      case CAST(Account::Role::LocalPort):
         return localPort();
      case CAST(Account::Role::BootstrapPort):
         return bootstrapPort();
      case CAST(Account::Role::PublishedPort):
         return publishedPort();
      case CAST(Account::Role::Enabled):
         return isEnabled();
      case CAST(Account::Role::AutoAnswer):
         return isAutoAnswer();
      case CAST(Account::Role::TlsVerifyServer):
         return isTlsVerifyServer();
      case CAST(Account::Role::TlsVerifyClient):
         return isTlsVerifyClient();
      case CAST(Account::Role::TlsRequireClientCertificate):
         return isTlsRequireClientCertificate();
      case CAST(Account::Role::TlsEnabled):
         return isTlsEnabled();
      case CAST(Account::Role::SrtpRtpFallback):
         return isSrtpRtpFallback();
      case CAST(Account::Role::SipStunEnabled):
         return isSipStunEnabled();
      case CAST(Account::Role::PublishedSameAsLocal):
         return isPublishedSameAsLocal();
      case CAST(Account::Role::RingtoneEnabled):
         return isRingtoneEnabled();
      case CAST(Account::Role::dTMFType):
         return DTMFType();
      case CAST(Account::Role::Id):
         return id();
      case CAST(Ring::Role::Object):
      case CAST(Account::Role::Object): {
         QVariant var;
         var.setValue(const_cast<Account*>(this));
         return var;
      }
      case CAST(Account::Role::TypeName):
         return CAST(protocol());
      case CAST(Account::Role::PresenceStatus):
         return PresenceStatusModel::instance().currentStatus();
      case CAST(Account::Role::PresenceMessage):
         return PresenceStatusModel::instance().currentMessage();
      case CAST(Account::Role::RegistrationState):
         return QVariant::fromValue(registrationState());
      case CAST(Account::Role::UsedForOutgogingCall):
         return isUsedForOutgogingCall();
      case CAST(Account::Role::TotalCallCount):
         return totalCallCount();
      case CAST(Account::Role::WeekCallCount):
         return weekCallCount();
      case CAST(Account::Role::TrimesterCallCount):
         return trimesterCallCount();
      case CAST(Ring::Role::LastUsed):
      case CAST(Account::Role::LastUsed):
         return (int)lastUsed();
      case CAST(Account::Role::UserAgent):
         return userAgent();
      case CAST(Account::Role::Password):
         return password();
      case CAST(Account::Role::SupportPresencePublish   ):
         return supportPresencePublish();
      case CAST(Account::Role::SupportPresenceSubscribe ):
         return supportPresenceSubscribe();
      case CAST(Account::Role::PresenceEnabled          ):
         return presenceEnabled();
      case CAST(Account::Role::IsVideoEnabled           ):
         return isVideoEnabled();
      case CAST(Account::Role::VideoPortMax             ):
         return videoPortMax();
      case CAST(Account::Role::VideoPortMin             ):
         return videoPortMin();
      case CAST(Account::Role::AudioPortMin             ):
         return audioPortMin();
      case CAST(Account::Role::AudioPortMax             ):
         return audioPortMax();
      case CAST(Account::Role::IsUpnpEnabled            ):
         return isUpnpEnabled();
      case CAST(Account::Role::HasCustomUserAgent       ):
         return hasCustomUserAgent();
      case CAST(Account::Role::LastTransportErrorCode   ):
         return lastTransportErrorCode();
      case CAST(Account::Role::LastTransportErrorMessage):
         return lastTransportErrorMessage();
      case CAST(Account::Role::TurnServer               ):
         return turnServer();
      case CAST(Account::Role::TurnServerEnabled        ):
         return isTurnEnabled();
      case CAST(Account::Role::TurnServerUsername       ):
         return turnServerUsername();
      case CAST(Account::Role::TurnServerPassword       ):
         return turnServerPassword();
      case CAST(Account::Role::TurnServerRealm          ):
         return turnServerRealm();
      case CAST(Account::Role::HasProxy                 ):
         return hasProxy();
      case CAST(Account::Role::DisplayName              ):
         return displayName();
      case CAST(Account::Role::SrtpEnabled              ):
         return isSrtpEnabled();
      case CAST(Account::Role::HasCustomBootstrap       ):
         //Do not create the model for nothing
         return protocol() == Account::Protocol::RING ? bootstrapModel()->isCustom() : false;
      case CAST(Account::Role::CredentialModel            ):
         return QVariant::fromValue(credentialModel());
      case CAST(Account::Role::CodecModel                 ):
         return QVariant::fromValue(codecModel());
      case CAST(Account::Role::KeyExchangeModel           ):
         return QVariant::fromValue(keyExchangeModel());
      case CAST(Account::Role::CipherModel                ):
         return QVariant::fromValue(cipherModel());
      case CAST(Account::Role::StatusModel                ):
         return QVariant::fromValue(statusModel());
      case CAST(Account::Role::SecurityEvaluationModel    ):
         return QVariant::fromValue(securityEvaluationModel());
      case CAST(Account::Role::TlsMethodModel             ):
         return QVariant::fromValue(tlsMethodModel());
      case CAST(Account::Role::ProtocolModel              ):
         return QVariant::fromValue(protocolModel());
      case CAST(Account::Role::BootstrapModel             ):
         return QVariant::fromValue(bootstrapModel());
      case CAST(Account::Role::NetworkInterfaceModel      ):
         return QVariant::fromValue(networkInterfaceModel());
      case CAST(Account::Role::KnownCertificateModel      ):
         return QVariant::fromValue(knownCertificateModel());
      case CAST(Account::Role::BannedCertificatesModel    ):
         return QVariant::fromValue(bannedCertificatesModel());
      case CAST(Account::Role::AllowedCertificatesModel   ):
         return QVariant::fromValue(allowedCertificatesModel());
      case CAST(Account::Role::AllowIncomingFromHistory ):
         return allowIncomingFromHistory();
      case CAST(Account::Role::AllowIncomingFromContact ):
         return allowIncomingFromContact();
      case CAST(Account::Role::AllowIncomingFromUnknown ):
         return allowIncomingFromUnknown();
      case CAST(Account::Role::ActiveCallLimit):
         return activeCallLimit();
      case CAST(Account::Role::HasActiveCallLimit):
         return hasActiveCallLimit();
      case CAST(Account::Role::SecurityLevel):
         if (extension<SecurityEvaluationExtension>()) {
            return QVariant::fromValue(
               extension<SecurityEvaluationExtension>()->securityLevel(this)
            );
         };
         break;
      case CAST(Account::Role::SecurityLevelIcon):
         if (extension<SecurityEvaluationExtension>()) {
            return extension<SecurityEvaluationExtension>()->securityLevelIcon(this);
         }
         break;
      case CAST(Account::Role::LastStatusChangeTimeStamp):
         return QVariant::fromValue(statusModel()->lastTimeStamp());
      case CAST(Account::Role::RegisteredName):
         return registeredName();
      default:
         return QVariant();
   }
   return QVariant();
}
#undef CAST


bool Account::supportScheme( URI::SchemeType type ) const
{
   switch(type) {
      case URI::SchemeType::NONE :
         if (protocol() == Account::Protocol::RING)
            /* the URIs which are supported by accounts of type RING are well
             * defined and should always be identified correctly, thus URIs
             * which are not identified to be of a specific type cannot possibly
             * be of type RING and thus should never be used to make a RING call
             */
            return false;
         return true;
         break;
      case URI::SchemeType::SIP  :
      case URI::SchemeType::SIPS :
         if (protocol() == Account::Protocol::SIP)
            return true;
         break;
      case URI::SchemeType::RING :
         if (protocol() == Account::Protocol::RING)
            return true;
         break;
      case URI::SchemeType::COUNT__:
         break;
   }
   return false;
}

bool Account::allowCertificate(Certificate* c)
{
   if (protocol() != Account::Protocol::RING)
      return false;

   return CertificateModel::instance().d_ptr->allowCertificate(c, this);
}

bool Account::banCertificate(Certificate* c)
{
   if (protocol() != Account::Protocol::RING)
      return false;

   return CertificateModel::instance().d_ptr->banCertificate(c, this);
}

///Ask the certificate owner (peer) to trust you
bool Account::sendContactRequest( const URI& uri )
{
   if (uri.isEmpty())
       return false;

   QByteArray payload;

   // Send our VCard as payload
   payload = profile()->person()->toVCard();

   ConfigurationManager::instance().sendTrustRequest(id(), uri, payload);

   return true;
}

bool Account::sendContactRequest(const ContactMethod* c)
{
    if (!c)
        return false;

    return sendContactRequest(c->uri());
}

bool Account::sendContactRequest( Certificate* c )
{
   if ((!c) || (c->remoteId().isEmpty()))
      return false;

   QByteArray payload;

   // Send our VCard as payload
   payload = profile()->person()->toVCard();

   ConfigurationManager::instance().sendTrustRequest(id(),c->remoteId(), payload);

   return true;
}

bool Account::removeContact( const URI& uri )
{
   if (uri.isEmpty())
       return false;

   ConfigurationManager::instance().removeContact(id(), uri, false);

   return true;
}

bool Account::removeContact(const ContactMethod* c)
{
    if (!c)
        return false;

    return removeContact(c->uri());
}

bool Account::removeContact( Certificate* c  )
{
    if ((!c) || (c->remoteId().isEmpty()))
        return false;

    ConfigurationManager::instance().removeContact(id(), c->remoteId(), false);
    return true;
}

bool Account::addContact( const URI& uri )
{
   if (uri.isEmpty())
       return false;

   ConfigurationManager::instance().addContact(id(), uri);

   return true;
}

bool Account::addContact(const ContactMethod* c)
{
    if (!c)
        return false;

    return addContact(c->uri());
}

bool Account::addContact( Certificate* c  )
{
    if ((!c) || (c->remoteId().isEmpty()))
        return false;

    ConfigurationManager::instance().addContact(id(), c->remoteId());
    return true;
}

bool Account::hasContact(ContactMethod* c )
{
    auto contacts = getContacts();
    return contacts.indexOf(c) != -1;
}

uint Account::internalId() const
{
   return d_ptr->m_InternalId;
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Set account details
void AccountPrivate::setAccountProperties(const QHash<QString,QString>& m)
{
   m_hAccountDetails.clear();
   m_hAccountDetails = m;
   m_HostName = m[DRing::Account::ConfProperties::HOSTNAME];
}

///Set a specific detail
bool AccountPrivate::setAccountProperty(const QString& param, const QString& val)
{
   const QString buf = m_hAccountDetails[param];
   const bool accChanged = buf != val;
   //Status can be changed regardless of the EditState
   //TODO make this more generic for volatile properties
   if (param == DRing::Account::ConfProperties::Registration::STATUS) {
      m_hAccountDetails[param] = val;
      if (accChanged) {
         emit q_ptr->changed(q_ptr);
         emit q_ptr->propertyChanged(q_ptr,param,val,buf);
      }
   }
   else if (accChanged) {

      m_hAccountDetails[param] = val;
      emit q_ptr->changed(q_ptr);
      emit q_ptr->propertyChanged(q_ptr,param,val,buf);

      q_ptr->performAction(Account::EditAction::MODIFY);
   }
   return m_CurrentState == Account::EditState::MODIFIED_COMPLETE
    || m_CurrentState == Account::EditState::MODIFIED_INCOMPLETE
    || m_CurrentState == Account::EditState::NEW;
}

bool Account::setAccountProperty(const QString& param, const QString& val)
{
    return d_ptr->setAccountProperty(param, val);
}

///Set the account id
void Account::setId(const QByteArray& id)
{
   if (! isNew())
      qDebug() << "Error : setting AccountId of an existing account" << d_ptr->m_AccountId;
   d_ptr->m_AccountId = id;
}

///Set the account type, SIP or RING
void Account::setProtocol(Account::Protocol proto)
{
   //TODO prevent this if the protocol has been saved
   switch (proto) {
      case Account::Protocol::SIP:
         d_ptr->setAccountProperty(DRing::Account::ConfProperties::TYPE ,DRing::Account::ProtocolNames::SIP );
         break;
      case Account::Protocol::RING:
         d_ptr->setAccountProperty(DRing::Account::ConfProperties::TYPE ,DRing::Account::ProtocolNames::RING);
         break;
      case Account::Protocol::COUNT__:
         break;
   };
}

///The set account hostname, it can be an hostname or an IP address
void Account::setHostname(const QString& detail)
{
   if (d_ptr->m_HostName != detail) {
      d_ptr->m_HostName = detail;
      if (protocol() == Account::Protocol::RING)
      {
          bootstrapModel() << BootstrapModel::EditAction::RELOAD;
      }
      d_ptr->setAccountProperty(DRing::Account::ConfProperties::HOSTNAME, detail);
   }
}


///Set the account registeredName
bool Account::registerName(const QString& password, const QString& name) const
{
    return NameDirectory::instance().registerName(this, password, name);
}

//Lookup a name
bool Account::lookupName(const QString& name) const
{
    return NameDirectory::instance().lookupName(this, QString(), name);
}

//Lookup an address
bool Account::lookupAddress(const QString& address) const
{
    return NameDirectory::instance().lookupAddress(this, QString(), address);
}

///Set the account username, everything is valid, some might be rejected by the PBX server
void Account::setUsername(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::USERNAME, detail);
   switch (protocol()) {
      case Account::Protocol::RING:
      case Account::Protocol::COUNT__:
         //nothing to do
         break;
      case Account::Protocol::SIP:
         if (credentialModel()->primaryCredential(Credential::Type::SIP)) {
            credentialModel()->primaryCredential(Credential::Type::SIP)->setUsername(detail);
            credentialModel() << CredentialModel::EditAction::MODIFY;
         }
         else {
            const QModelIndex idx = credentialModel()->addCredentials(Credential::Type::SIP);
            credentialModel()->setData(idx,detail,CredentialModel::Role::NAME);
         }
         break;
   };
}

///Set the account mailbox, usually a number, but can be anything
void Account::setMailbox(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::MAILBOX, detail);
}

///Set the account mailbox, usually a number, but can be anything
void Account::setProxy(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ROUTE, detail);
}

//Set the name service URL
void Account::setNameServiceURL(const QString& detail)
{
    d_ptr->setAccountProperty(DRing::Account::ConfProperties::RingNS::URI, detail);
}

///Set the main credential password
void Account::setPassword(const QString& detail)
{
   switch (protocol()) {
      case Account::Protocol::SIP:
         if (credentialModel()->primaryCredential(Credential::Type::SIP)) {
            credentialModel()->primaryCredential(Credential::Type::SIP)->setPassword(detail);
            credentialModel() << CredentialModel::EditAction::MODIFY;
         }
         else {
            const QModelIndex idx = credentialModel()->addCredentials(Credential::Type::SIP);
            credentialModel()->setData(idx,detail,CredentialModel::Role::PASSWORD);
         }
         break;
      case Account::Protocol::RING:
         setTlsPassword(detail);
         break;
      case Account::Protocol::COUNT__:
         break;
   };
}

///Set the TLS (encryption) password
void Account::setTlsPassword(const QString& detail)
{
   auto cert = tlsCertificate();
   if (!cert)
      return;
   cert->setPrivateKeyPassword(detail);
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::PASSWORD, detail);
   d_ptr->regenSecurityValidation();
}

///Set the certificate authority list file
void Account::setTlsCaListCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCaListCertificate(cert);
}

///Set the certificate
void Account::setTlsCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCertificate(cert);
}

///Set the private key
void Account::setTlsPrivateKey(const QString& path)
{
    auto cert = tlsCertificate();
    if (!cert)
        return;

    cert->setPrivateKeyPath(path);
    d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE, cert?path:QString());
    d_ptr->regenSecurityValidation();
}

///Set the certificate authority list file
void Account::setTlsCaListCertificate(Certificate* cert)
{
   //FIXME it can be a list of multiple certificates
   //this code currently only handle the case where is there is exactly one

   cert->setRequireStrictPermission(false);

   //All calls from the same top level CA are always accepted
   allowCertificate(cert);

   d_ptr->m_pCaCert = cert;
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::CA_LIST_FILE, cert?cert->path():QString());
   d_ptr->regenSecurityValidation();

   if (d_ptr->m_cTlsCaCert)
      disconnect(d_ptr->m_cTlsCaCert);

   if (cert) {
      d_ptr->m_cTlsCaCert = connect(cert, &Certificate::changed,[this]() {
         d_ptr->regenSecurityValidation();
      });
   }

}

///Set the certificate
void Account::setTlsCertificate(Certificate* cert)
{
   //The private key will be required for this certificate
   cert->setRequirePrivateKey(true);

   d_ptr->m_pTlsCert = cert;
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE, cert?cert->path():QString());
   d_ptr->regenSecurityValidation();
}

///Set the TLS server
void Account::setTlsServerName(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::SERVER_NAME, detail);
   d_ptr->regenSecurityValidation();
}

///Set the stun server
void Account::setSipStunServer(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::STUN::SERVER, detail);
}

///Set the published address
void Account::setPublishedAddress(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::PUBLISHED_ADDRESS, detail);
}

///Set the ringtone path, it have to be a valid absolute path
void Account::setRingtonePath(const QString& detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Ringtone::PATH, detail);
}

///Set the number of voice mails
void Account::setVoiceMailCount(int count)
{
   d_ptr->m_VoiceMailCount = count;
}

///Set the account timeout, it will be renegotiated when that timeout occur
void Account::setRegistrationExpire(int detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Registration::EXPIRE, QString::number(detail));
}

///Set TLS negotiation timeout in second
void Account::setTlsNegotiationTimeoutSec(int detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC, QString::number(detail));
   d_ptr->regenSecurityValidation();
}

///Set the local port for SIP/RING communications
void Account::setLocalPort(unsigned short detail)
{
   switch (protocol()) {
      case Account::Protocol::SIP:
         if (isTlsEnabled())
            d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::LISTENER_PORT, QString::number(detail));
         else
            d_ptr->setAccountProperty(DRing::Account::ConfProperties::LOCAL_PORT, QString::number(detail));
      case Account::Protocol::RING:
         d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::LISTENER_PORT, QString::number(detail));
         break;
      case Account::Protocol::COUNT__:
         break;
   };
}

///Set the TLS listener port (0-2^16)
void Account::setBootstrapPort(unsigned short detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::DHT::PORT, QString::number(detail));
}

///Set the published port (0-2^16)
void Account::setPublishedPort(unsigned short detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::PUBLISHED_PORT, QString::number(detail));
}

///Set if the account is enabled or not
void Account::setEnabled(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ENABLED, (detail)TO_BOOL);
}

///Set if the account should auto answer
void Account::setAutoAnswer(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::AUTOANSWER, (detail)TO_BOOL);
}

///Set the TLS verification server
void Account::setTlsVerifyServer(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_SERVER, (detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

///Set the TLS verification client
void Account::setTlsVerifyClient(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT, (detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

///Set if the peer need to be providing a certificate
void Account::setTlsRequireClientCertificate(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE ,(detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

///Set if the security settings are enabled
void Account::setTlsEnabled(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::ENABLED ,(detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

void Account::setSrtpRtpFallback(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::SRTP::RTP_FALLBACK, (detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

void Account::setSrtpEnabled(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::SRTP::ENABLED, (detail)TO_BOOL);
   d_ptr->regenSecurityValidation();
}

void Account::setSipStunEnabled(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::STUN::ENABLED, (detail)TO_BOOL);
}

/**
 * Set if the published address is the same as the local IP address
 * @see Account::setPublishedAddress
 * @see Account::publishedAddress
 */
void Account::setPublishedSameAsLocal(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::PUBLISHED_SAMEAS_LOCAL, (detail)TO_BOOL);
}

///Set if custom ringtone are enabled
void Account::setRingtoneEnabled(bool detail)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Ringtone::ENABLED, (detail)TO_BOOL);
}

/**
 * Set if the account broadcast its presence data
 *
 * @note This only works when the account support presence
 * @see Account::supportPresencePublish()
 */
void Account::setPresenceEnabled(bool enable)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Presence::ENABLED, (enable)TO_BOOL);
   emit presenceEnabledChanged(enable);
}

///Use video by default when available
void Account::setVideoEnabled(bool enable)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::ENABLED, (enable)TO_BOOL);
}

/**Set the maximum audio port
 * This can be used when some routers without UPnP support open a narrow range
 * of ports to allow the stream to go through.
 */
void Account::setAudioPortMax(int port )
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Audio::PORT_MAX, QString::number(port));
}

/**Set the minimum audio port
 * This can be used when some routers without UPnP support open a narrow range
 * of ports to allow the stream to go through.
 */
void Account::setAudioPortMin(int port )
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Audio::PORT_MIN, QString::number(port));
}

/**Set the maximum video port
 * This can be used when some routers without UPnP support open a narrow range
 * of ports to allow the stream to go through.
 */
void Account::setVideoPortMax(int port )
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::PORT_MAX, QString::number(port));
}

/**Set the minimum video port
 * This can be used when some routers without UPnP support open a narrow range
 * of ports to allow the stream to go through.
 */
void Account::setVideoPortMin(int port )
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::Video::PORT_MIN, QString::number(port));
}

void Account::setUpnpEnabled(bool enable)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::UPNP_ENABLED, (enable)TO_BOOL);
}

void Account::setHasCustomUserAgent(bool enable)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::HAS_CUSTOM_USER_AGENT, (enable)TO_BOOL);
}

///TODO implement the "use default" logic correctly
/**Set the user agent
 * If the string is unchanged, the daemon should upgrade it automatically
 */
void Account::setUserAgent(const QString& agent)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::USER_AGENT, agent);
}

void Account::setUseDefaultPort(bool value)
{
   if (value) {
      switch (protocol()) {
         case Account::Protocol::SIP:
            setLocalPort(5060); //FIXME check is TLS is used
            break;
         case Account::Protocol::RING:
            setLocalPort(5061);
            break;
         case Account::Protocol::COUNT__:
            break;
      };
   }
   d_ptr->m_UseDefaultPort = value;
}

void Account::setTurnEnabled(bool value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TURN::ENABLED, (value)TO_BOOL);
}

void Account::setTurnServer(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER, value);
}

void Account::setTurnServerUsername(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_UNAME, value);
}

void Account::setTurnServerPassword(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_PWD, value);
}

void Account::setTurnServerRealm(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::TURN::SERVER_REALM, value);
}

void Account::setDisplayName(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::DISPLAYNAME, value);
}

void Account::setArchivePassword(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ARCHIVE_PASSWORD, value);
}

void Account::setArchivePin(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ARCHIVE_PIN, value);
}

void Account::setArchivePath(const QString& value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ARCHIVE_PATH, value);
}

void Account::setAllowIncomingFromUnknown(bool value)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::DHT::PUBLIC_IN_CALLS, (value)TO_BOOL);
}

void Account::setAllowIncomingFromHistory(bool value)
{
   if (protocol() != Account::Protocol::RING)
      return;

   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ALLOW_CERT_FROM_HISTORY, value TO_BOOL);
   performAction(Account::EditAction::MODIFY);
}

void Account::setAllowIncomingFromContact(bool value)
{
   if (protocol() != Account::Protocol::RING)
      return;

   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ALLOW_CERT_FROM_CONTACT, value TO_BOOL);
   performAction(Account::EditAction::MODIFY);
}

void Account::setActiveCallLimit(int value )
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::ACTIVE_CALL_LIMIT, QString::number(value));
}

void Account::setHasActiveCallLimit(bool value )
{
   if ((!value) && activeCallLimit() != -1)
      return;

   setActiveCallLimit(value?1:-1);
}

///Set the DTMF type
void Account::setDTMFType(DtmfType type)
{
   d_ptr->setAccountProperty(DRing::Account::ConfProperties::DTMF_TYPE,(type==OverRtp)?"overrtp":"oversip");
}

void Account::setProfile(Profile* p)
{
    if (!p) {
        qWarning() << "Cannot set profile to null as all accounts must belong to a profile";
        return;
    }

    if (p == d_ptr->m_pProfile)
        return; // nothing to do

    if (d_ptr->m_pProfile)
        d_ptr->m_pProfile->removeAccount(this);

    if (p->addAccount(this))
        p->save();

    d_ptr->m_pProfile = p;

    emit changed(this);
}

Profile* Account::profile() const
{
   // Make sure all accounts belong to a profile
   if (!d_ptr->m_pProfile) {
      Profile* p = ProfileModel::instance().selectedProfile();

      if (!p) // for now default to the first profile
         p = ProfileModel::instance().getProfile(ProfileModel::instance().index(0,0));

      if (!p)
         return nullptr;

      // Use a const cast rather than a mutable to make sure the logic is the
      // same between "automatic" default profile" and the setProfile
      // implementation.
      const_cast<Account*>(this)->setProfile(p);
   }

   return d_ptr->m_pProfile;
}

void Account::setLastSipRegistrationStatus(const QString& value )
{
    d_ptr->m_LastSipRegistrationStatus = value;
}

void Account::setLastTransportCode(int value)
{
    d_ptr->m_LastTransportCode = value;
}

void Account::setLastTransportMessage(const QString& value)
{
    d_ptr->m_LastTransportMessage = value;
}

void Account::setRegistrationState(const RegistrationState& value)
{
    d_ptr->m_RegistrationState = value;
}

#define CAST(item) static_cast<int>(item)
///Proxy for AccountModel::setData
void Account::setRoleData(int role, const QVariant& value)
{
   switch(role) {
      case CAST(Account::Role::Alias):
         setAlias(value.toString());
         break;
      case CAST(Account::Role::Proto): {
         const int proto = value.toInt();
         setProtocol((proto>=0&&proto<=1)?static_cast<Account::Protocol>(proto):Account::Protocol::SIP);
         break;
      }
      case CAST(Account::Role::Hostname):
         setHostname(value.toString());
         break;
      case CAST(Account::Role::Username):
         setUsername(value.toString());
         break;
      case CAST(Account::Role::Mailbox):
         setMailbox(value.toString());
         break;
      case CAST(Account::Role::Proxy):
         setProxy(value.toString());
         break;
//       case Password:
//          accountPassword();
      case CAST(Account::Role::TlsPassword):
         setTlsPassword(value.toString());
         break;
      case CAST(Account::Role::TlsCaListCertificate): {
         setTlsCaListCertificate(value.toString());
         break;
      }
      case CAST(Account::Role::TlsCertificate): {
         setTlsCertificate(value.toString());
      }
         break;
      case CAST(Account::Role::TlsServerName):
         setTlsServerName(value.toString());
         break;
      case CAST(Account::Role::SipStunServer):
         setSipStunServer(value.toString());
         break;
      case CAST(Account::Role::PublishedAddress):
         setPublishedAddress(value.toString());
         break;
      case CAST(Account::Role::RingtonePath):
         setRingtonePath(value.toString());
         break;
      case CAST(Account::Role::RegistrationExpire):
         setRegistrationExpire(value.toInt());
         break;
      case CAST(Account::Role::TlsNegotiationTimeoutSec):
         setTlsNegotiationTimeoutSec(value.toInt());
         break;
      case CAST(Account::Role::LocalPort):
         setLocalPort(value.toInt());
         break;
      case CAST(Account::Role::BootstrapPort):
         setBootstrapPort(value.toInt());
         break;
      case CAST(Account::Role::PublishedPort):
         setPublishedPort(value.toInt());
         break;
      case CAST(Account::Role::Enabled):
         setEnabled(value.toBool());
         break;
      case CAST(Account::Role::AutoAnswer):
         setAutoAnswer(value.toBool());
         break;
      case CAST(Account::Role::TlsVerifyServer):
         setTlsVerifyServer(value.toBool());
         break;
      case CAST(Account::Role::TlsVerifyClient):
         setTlsVerifyClient(value.toBool());
         break;
      case CAST(Account::Role::TlsRequireClientCertificate):
         setTlsRequireClientCertificate(value.toBool());
         break;
      case CAST(Account::Role::TlsEnabled):
         setTlsEnabled(value.toBool());
         break;
      case CAST(Account::Role::SrtpRtpFallback):
         setSrtpRtpFallback(value.toBool());
         break;
      case CAST(Account::Role::SipStunEnabled):
         setSipStunEnabled(value.toBool());
         break;
      case CAST(Account::Role::PublishedSameAsLocal):
         setPublishedSameAsLocal(value.toBool());
         break;
      case CAST(Account::Role::RingtoneEnabled):
         setRingtoneEnabled(value.toBool());
         break;
      case CAST(Account::Role::dTMFType):
         setDTMFType((DtmfType)value.toInt());
         break;
      case CAST(Account::Role::Id):
         setId(value.toByteArray());
         break;
      case CAST(Account::Role::UserAgent):
         setUserAgent(value.toString());
         break;
      case CAST(Account::Role::Password):
         setPassword(value.toString());
         break;
      case CAST(Account::Role::SupportPresencePublish   ):
         break;
      case CAST(Account::Role::SupportPresenceSubscribe ):
         break;
      case CAST(Account::Role::PresenceEnabled          ):
         setPresenceEnabled(value.toBool());
         break;
      case CAST(Account::Role::IsVideoEnabled           ):
         setVideoEnabled(value.toBool());
         break;
      case CAST(Account::Role::VideoPortMax             ):
         setVideoPortMax(value.toInt());
         break;
      case CAST(Account::Role::VideoPortMin             ):
         setVideoPortMin(value.toInt());
         break;
      case CAST(Account::Role::AudioPortMin             ):
         setAudioPortMin(value.toInt());
         break;
      case CAST(Account::Role::AudioPortMax             ):
         setAudioPortMax(value.toInt());
         break;
      case CAST(Account::Role::IsUpnpEnabled            ):
         setUpnpEnabled(value.toBool());
         break;
      case CAST(Account::Role::HasCustomUserAgent       ):
         setHasCustomUserAgent(value.toBool());
         break;
      case CAST(Account::Role::LastTransportErrorCode   ):
         break;
      case CAST(Account::Role::LastTransportErrorMessage):
         break;
      case CAST(Account::Role::TurnServer               ):
         setTurnServer(value.toString());
         break;
      case CAST(Account::Role::DisplayName              ):
         setDisplayName(value.toString());
         break;
      case CAST(Account::Role::SrtpEnabled              ):
         setSrtpEnabled(value.toBool());
         break;
      case CAST(Account::Role::HasCustomBootstrap       ):
         //Do not create the model for nothing
         if (protocol() == Account::Protocol::RING && value.toBool())
            bootstrapModel()->reset();
         break;
      //Read-only
      case CAST(Account::Role::CredentialModel          ):
      case CAST(Account::Role::CodecModel               ):
      case CAST(Account::Role::KeyExchangeModel         ):
      case CAST(Account::Role::CipherModel              ):
      case CAST(Account::Role::StatusModel              ):
      case CAST(Account::Role::SecurityEvaluationModel  ):
      case CAST(Account::Role::TlsMethodModel           ):
      case CAST(Account::Role::ProtocolModel            ):
      case CAST(Account::Role::BootstrapModel           ):
      case CAST(Account::Role::NetworkInterfaceModel    ):
      case CAST(Account::Role::KnownCertificateModel    ):
      case CAST(Account::Role::BannedCertificatesModel  ):
      case CAST(Account::Role::AllowedCertificatesModel ):
         break;
      case CAST(Account::Role::AllowIncomingFromHistory ):
         setAllowIncomingFromHistory(value.toBool());
         break;
      case CAST(Account::Role::AllowIncomingFromContact ):
         setAllowIncomingFromContact(value.toBool());
         break;
      case CAST(Account::Role::AllowIncomingFromUnknown ):
         setAllowIncomingFromUnknown(value.toBool());
         break;
      case CAST(Account::Role::ActiveCallLimit):
         return setActiveCallLimit(value.toInt());
         break;
      case CAST(Account::Role::HasActiveCallLimit):
         return setHasActiveCallLimit(value.toBool());
         break;
      //Read-only
      case CAST(Account::Role::SecurityLevel):
      case CAST(Account::Role::SecurityLevelIcon):
         break;
      case CAST(Account::Role::TurnServerPassword):
       setTurnServerPassword(value.toString());
       break;
      case CAST(Account::Role::TurnServerRealm):
       setTurnServerRealm(value.toString());
       break;
      case CAST(Account::Role::TurnServerUsername):
       setTurnServerUsername(value.toString());
       break;
      case CAST(Account::Role::TurnServerEnabled):
       setTurnEnabled(value.toBool());
       break;
   }
}
#undef CAST


/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

void AccountPrivate::performAction(const Account::EditAction action)
{
   (this->*(stateMachineActionsOnState[m_CurrentState][action]))();
}

/// anAccount << Call::EditAction::SAVE
Account* Account::operator<<(Account::EditAction& action)
{
   performAction(action);
   return this;
}

Account* operator<<(Account* a, Account::EditAction action)
{
   return (!a)?nullptr : (*a) << action;
}

///Change the current edition state
bool Account::performAction(const Account::EditAction action)
{
   Account::EditState curState = d_ptr->m_CurrentState;
   d_ptr->performAction(action);
   return curState != d_ptr->m_CurrentState;
}

///Get the current account edition state
Account::EditState Account::editState() const
{
   return d_ptr->m_CurrentState;
}

/**
 * This method can be used to query if a field is available or not in current
 * context. This could be extended over time. For now, it only handle the fields
 * related to SDES.
 *
 * @todo Support of the private key password is necessary
 *
 * @param role An SDES related Account::Role
 * @return if the field is available in the current context
 */
Account::RoleState Account::roleState(Account::Role role) const
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wswitch-enum"
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wswitch"

   //Hide unsupported fields by protocol
   switch(protocol()) {
      case Account::Protocol::RING:
         switch(role) {
            case Account::Role::Password          :
            case Account::Role::RegistrationExpire:
            case Account::Role::Mailbox           :
            case Account::Role::UserAgent         :
            case Account::Role::HasCustomUserAgent:
            case Account::Role::HasProxy          :
            case Account::Role::Proxy             :
            case Account::Role::CipherModel       :
            case Account::Role::DisplayName       :
               return Account::RoleState::UNAVAILABLE;
            case Account::Role::Username                :
            case Account::Role::TlsCaListCertificate    :
            case Account::Role::TlsCertificate          :
            case Account::Role::SrtpEnabled             :
            case Account::Role::TlsEnabled              :
               return Account::RoleState::READ_ONLY;
            default:
               break;
         }

         // The registered name cannot be changed once the account is created
         if (!isNew() && role == Account::Role::RegisteredName)
             return Account::RoleState::READ_ONLY;

         break;
      case Account::Protocol::SIP     :
         switch(role) {
            case Account::Role::PresenceEnabled:
               return supportPresenceSubscribe() || supportPresencePublish() ?
                  Account::RoleState::READ_WRITE : Account::RoleState::UNAVAILABLE;
         }
         [[clang::fallthrough]];
      case Account::Protocol::COUNT__ :
         switch(role) {
            case Account::Role::BannedCertificatesModel :
            case Account::Role::AllowedCertificatesModel:
            case Account::Role::AllowIncomingFromHistory:
            case Account::Role::AllowIncomingFromContact:
            case Account::Role::AllowIncomingFromUnknown:
            case Account::Role::RegisteredName          :
            case Account::Role::HasCustomBootstrap      :
               return Account::RoleState::UNAVAILABLE;
         }
         break;
   }

   //Supported security fields
   enum class Fields {
      SDES_FALLBACK_RTP  ,
      COUNT__
   };

   //Mapping between the roles and the fields
   Fields f = Fields::COUNT__;
   switch(role) {
      case Account::Role::SrtpRtpFallback   :
         f = Fields::SDES_FALLBACK_RTP  ;
         break;
   }
   #pragma GCC diagnostic pop
   #pragma GCC diagnostic pop

   //The field is not possible to disable
   if (f == Fields::COUNT__)
      return Account::RoleState::READ_WRITE;

   constexpr static const Account::RoleState rw = Account::RoleState::READ_WRITE ;
   constexpr static const Account::RoleState un = Account::RoleState::UNAVAILABLE;

   //Matrix used to define if a field is enabled
   static const Matrix2D<KeyExchangeModel::Type, Fields, Account::RoleState>
   enabledFields={{
      /*                 ______________________> SDES_FALLBACK_RTP  */
      /*                /                                           */
      /*               \/                                           */
      /*Type::SDES*/ {{rw}},
      /*Type::NONE*/ {{un}},
   }};

   const QModelIndex idx = keyExchangeModel()->selectionModel()->currentIndex();

   if (!idx.isValid())
      return Account::RoleState::UNAVAILABLE;

   const KeyExchangeModel::Type type = qvariant_cast<KeyExchangeModel::Type>(
      idx.data(static_cast<int>(KeyExchangeModel::Role::TYPE))
   );

   return enabledFields[type][f];
}

/**
 * Get the **current** status of the role based on its content. ::roleState
 * check the field state based on protocol and context with ::roleStatus check
 * the content.
 */
Account::RoleStatus Account::roleStatus(Account::Role role) const
{
   //The validations are currently performed by the state machine
   return d_ptr->m_hRoleStatus[(int)role];
}

/**Update the account
 * @return if the state changed
 */
bool AccountPrivate::updateState()
{
   if(! q_ptr->isNew()) {
      ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
      const MapStringString details        = configurationManager.getVolatileAccountDetails(q_ptr->id());
      const QString         status         = details[DRing::Account::VolatileProperties::Registration::STATUS];
      const Account::RegistrationState cst = q_ptr->registrationState();
      const Account::RegistrationState st  = Account::fromDaemonName(status);

      setAccountProperty(DRing::Account::ConfProperties::Registration::STATUS, status); //Update -internal- object state
      m_RegistrationState = st;

      if (st != cst)
         emit q_ptr->stateChanged(q_ptr->registrationState());

      return st == cst;
   }
   return true;
}

bool Account::updateState()
{
    return d_ptr->updateState();
}

///Save the current account to the daemon
void AccountPrivate::save()
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   if (q_ptr->isNew()) {
      MapStringString details;
      QMutableHashIterator<QString,QString> iter(m_hAccountDetails);

      while (iter.hasNext()) {
         iter.next();
         details[iter.key()] = iter.value();
      }

      //Clear the password
      q_ptr->setArchivePassword("");

      const QString currentId = configurationManager.addAccount(details);

      q_ptr->codecModel() << CodecModel::EditAction::RELOAD;

      q_ptr->setId(currentId.toLatin1());
   } //New account
   else { //Existing account
      MapStringString tmp;
      QMutableHashIterator<QString,QString> iter(m_hAccountDetails);

      while (iter.hasNext()) {
         iter.next();
         tmp[iter.key()] = iter.value();
      }
      configurationManager.setAccountDetails(q_ptr->id(), tmp);
      if (m_RemoteEnabledState != q_ptr->isEnabled()) {
         m_RemoteEnabledState = q_ptr->isEnabled();
         emit q_ptr->enabled(m_RemoteEnabledState);
      }
   }

   //Save the credentials if they changed
   q_ptr->credentialModel() << CredentialModel::EditAction::SAVE;

   if (!q_ptr->id().isEmpty()) {
      Account* acc =  AccountModel::instance().getById(q_ptr->id());
      if (acc != q_ptr) {
         qDebug() << "Adding the new account to the account list (" << q_ptr->id() << ")";
         AccountModel::instance().add(q_ptr);
      }

      q_ptr->performAction(Account::EditAction::RELOAD);
      updateState();

      changeState(Account::EditState::READY);
   }

   q_ptr->codecModel() << CodecModel::EditAction::SAVE;

   emit q_ptr->changed(q_ptr);
}

///sync with the daemon, this need to be done manually to prevent reloading the account while it is being edited
void AccountPrivate::reload()
{
   if (!q_ptr->isNew()) {
      if (m_hAccountDetails.size())
         qDebug() << "Reloading" << q_ptr->id() << q_ptr->alias();
      else
         qDebug() << "Loading" << q_ptr->id();
      ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
      QMap<QString,QString> aDetails = configurationManager.getAccountDetails(q_ptr->id());

      if (!aDetails.count()) {
         qDebug() << "Account not found";
      }
      else {
         m_hAccountDetails.clear();
         //In case not all elements match, it is better to do a "merge"
         QMutableMapIterator<QString, QString> iter(aDetails);
         while (iter.hasNext()) {
            iter.next();
            m_hAccountDetails[iter.key()] = iter.value();
         }

         //Manually re-set elements that need extra business logic or caching
         q_ptr->setHostname(m_hAccountDetails[DRing::Account::ConfProperties::HOSTNAME]);

         const QString ca  (m_hAccountDetails[DRing::Account::ConfProperties::TLS::CA_LIST_FILE    ]);
         const QString cert(m_hAccountDetails[DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE]);
         const QString key (m_hAccountDetails[DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE]);
         const QString pass(m_hAccountDetails[DRing::Account::ConfProperties::TLS::PASSWORD]);

         if (!ca.isEmpty())
            q_ptr->setTlsCaListCertificate(ca);

         // Set the pvk file and password only if there is a certificate
         if (!cert.isEmpty()) {
            q_ptr->setTlsCertificate(cert);
            if (!key.isEmpty()) {
               q_ptr->setTlsPrivateKey(key);
                  if (!pass.isEmpty())
                     q_ptr->setTlsPassword(pass);
            }
         }

         m_RemoteEnabledState = q_ptr->isEnabled();
      }

      changeState(Account::EditState::READY);

      //TODO port this to the URI class helpers, this doesn't cover all corner cases
      const QString currentUri = QString("%1@%2").arg(q_ptr->username()).arg(m_HostName);

      if (!m_pAccountNumber || (m_pAccountNumber && m_pAccountNumber->uri() != currentUri)) {
         if (m_pAccountNumber) {
            disconnect(m_pAccountNumber,SIGNAL(presenceMessageChanged(QString)),this,SLOT(slotPresenceMessageChanged(QString)));
            disconnect(m_pAccountNumber,SIGNAL(presentChanged(bool)),this,SLOT(slotPresentChanged(bool)));
         }
         m_pAccountNumber = PhoneDirectoryModel::instance().getNumber(currentUri,q_ptr);
         m_pAccountNumber->setType(ContactMethod::Type::ACCOUNT);
         connect(m_pAccountNumber,SIGNAL(presenceMessageChanged(QString)),this,SLOT(slotPresenceMessageChanged(QString)));
         connect(m_pAccountNumber,SIGNAL(presentChanged(bool)),this,SLOT(slotPresentChanged(bool)));
      }

      //If the credential model is loaded, then update it
      if (m_pCredentials)
         m_pCredentials << CredentialModel::EditAction::RELOAD;

      //If the codec model is loaded, then update it
      if (m_pCodecModel)
         m_pCodecModel << CodecModel::EditAction::RELOAD;

      emit q_ptr->changed(q_ptr);

      //The registration state is cached, update that cache
      updateState();

      AccountModel::instance().d_ptr->slotVolatileAccountDetailsChange(q_ptr->id(),configurationManager.getVolatileAccountDetails(q_ptr->id()));
   }
}

void AccountPrivate::nothing()
{

}

void AccountPrivate::edit()    {
   changeState(Account::EditState::EDITING );
}

void AccountPrivate::modify()  {
   typedef Account::RoleStatus ST;
   typedef Account::Role       R ;
   //This check if the account can be saved or it would produce an invalid result

   //Future checks that could be implemented:
   // * 2 accounts with the same alias
   // * Weak password (will require a new COMPLETE_WARNING edit state)
   // * Invalid hostname
   // * Unreachable hostname (will require a new COMPLETE_WARNING edit state)
   // * Invalid username
   // * Non hash username for  Ring accounts


   /* ALIAS   : While valid, an account without one cause usability issues */
   /* HOSTNAME: Without hostname, an account cannot be "READY"             */
   /* USERNAME: All protocols but IP2IP require an username                */
   /* PASSWORD: SIP accounts require a password (unless used as IP2IP)     */

   m_hRoleStatus[(int)R::Alias   ] = q_ptr->alias   ().isEmpty() ? ST::REQUIRED_EMPTY : ST::OK;
   m_hRoleStatus[(int)R::Hostname] = q_ptr->hostname().isEmpty() ? ST::REQUIRED_EMPTY : ST::OK;
   m_hRoleStatus[(int)R::Username] = q_ptr->username().isEmpty() ? ST::REQUIRED_EMPTY : ST::OK;
   m_hRoleStatus[(int)R::Password] = q_ptr->password().isEmpty() ? ST::REQUIRED_EMPTY : ST::OK;

   //Apply some filters per protocol
   switch (q_ptr->protocol()) {
      case Account::Protocol::SIP:
         //IP2IP is very permissive about missing fields
         if (q_ptr->isIp2ip()) {
            m_hRoleStatus[(int)R::Alias   ] = ST::OK;
            m_hRoleStatus[(int)R::Username] = ST::OK;
            m_hRoleStatus[(int)R::Hostname] = ST::OK;
            m_hRoleStatus[(int)R::Password] = ST::OK;
         }
         break;
      case Account::Protocol::RING:
         m_hRoleStatus[(int)R::Hostname] = ST::OK;
         m_hRoleStatus[(int)R::Password] = ST::OK;

         //New accounts will get the hash later
         if (q_ptr->isNew() && q_ptr->username().isEmpty())
            m_hRoleStatus[(int)R::Username] = ST::OK     ;
         else if (q_ptr->isNew() && !q_ptr->username().isEmpty())
            m_hRoleStatus[(int)R::Username] = ST::INVALID;

         break;
      case Account::Protocol::COUNT__:
         //No changes needed
         break;
   }

   const bool isIncomplete = (
        (m_hRoleStatus[(int) R::Alias    ] != ST::OK)
      | (m_hRoleStatus[(int) R::Hostname ] != ST::OK)
      | (m_hRoleStatus[(int) R::Username ] != ST::OK)
      | (m_hRoleStatus[(int) R::Password ] != ST::OK)
   );

   const Account::EditState newState = isIncomplete ?
      Account::EditState::MODIFIED_INCOMPLETE :
      Account::EditState::MODIFIED_COMPLETE   ;

   if (newState != q_ptr->editState())
      changeState(newState);
}

void AccountPrivate::remove()  {
   changeState(Account::EditState::REMOVED );
}

void AccountPrivate::cancel()  {
   changeState(Account::EditState::READY   );
}

void AccountPrivate::outdate() {
   changeState(Account::EditState::OUTDATED);
}

void AccountPrivate::regenSecurityValidation()
{
   if (m_pSecurityEvaluationModel) {
      m_pSecurityEvaluationModel->d_ptr->update();
   }
}

/*****************************************************************************
 *                                                                           *
 *                                 Operator                                  *
 *                                                                           *
 ****************************************************************************/

///Are both account the same
bool Account::operator==(const Account& a)const
{
   return d_ptr->m_AccountId == a.d_ptr->m_AccountId;
}

/*****************************************************************************
 *                                                                           *
 *                               Placeholder                                 *
 *                                                                           *
 ****************************************************************************/

///Constructor
AccountPlaceHolder::AccountPlaceHolder(const QByteArray& uid) : Account(),
d_ptr(nullptr)
{
   Account::d_ptr->m_AccountId = uid  ;
   Account::d_ptr->m_isLoaded  = false;
}

/*****************************************************************************
 *                                                                           *
 *                                 Mutator                                   *
 *                                                                           *
 ****************************************************************************/

///Merge an existing account into this temporary one
bool AccountPrivate::merge(Account* account)
{
   if ((!account) || this == account->Account::d_ptr.data())
      return false;

   /*AccountPrivate* p = (AccountPrivate*) (q_ptr->Account::d_ptr.take());
   delete p;*/ //FIXME memory leak

   q_ptr->Account::d_ptr = account->Account::d_ptr;
   emit q_ptr->changed(q_ptr);

   return true;
}

/*****************************************************************************
 *                                                                           *
 *                                 Helper                                    *
 *                                                                           *
 ****************************************************************************/

void Account::regenSecurityValidation()
{
    d_ptr->regenSecurityValidation();
}

/**
 * The client have a different point of view when it come to the account
 * state. All the different errors are also handled elsewhere
 */
Account::RegistrationState Account::fromDaemonName(const QString& st)
{
   if     ( st == DRing::Account::States::REGISTERED
        ||  st == DRing::Account::States::READY                    )
      return Account::RegistrationState::READY;

   else if( st == DRing::Account::States::UNREGISTERED             )
      return Account::RegistrationState::UNREGISTERED;

   else if( st == DRing::Account::States::TRYING                   )
      return Account::RegistrationState::TRYING;

   else if (st == DRing::Account::States::INITIALIZING             )
      return Account::RegistrationState::INITIALIZING;

   else if( st == DRing::Account::States::ERROR
        ||  st == DRing::Account::States::ERROR_GENERIC
        ||  st == DRing::Account::States::ERROR_AUTH
        ||  st == DRing::Account::States::ERROR_NETWORK
        ||  st == DRing::Account::States::ERROR_HOST
        ||  st == DRing::Account::States::ERROR_CONF_STUN
        ||  st == DRing::Account::States::ERROR_EXIST_STUN
        ||  st == DRing::Account::States::ERROR_SERVICE_UNAVAILABLE
        ||  st == DRing::Account::States::ERROR_NOT_ACCEPTABLE
        ||  st == DRing::Account::States::ERROR_NEED_MIGRATION
        ||  st == DRing::Account::States::REQUEST_TIMEOUT          )
      return Account::RegistrationState::ERROR;

   else {
      qWarning() << "Unknown registration state" << st;
      return Account::RegistrationState::ERROR;
   }

}

#undef TO_BOOL
#undef IS_TRUE
#include <account.moc>

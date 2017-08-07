/****************************************************************************
 *   Copyright (C) 2009-2017 Savoir-faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *            Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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
#include "accountmodel.h"

//Std
#include <atomic>
#include <algorithm>

//Qt
#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QMimeData>
#include <QtCore/QDir>

//Ring daemon
#include <account_const.h>

//Ring library
#include "account.h"
#include "mime.h"
#include "profilemodel.h"
#include "protocolmodel.h"
#include "contactrequest.h"
#include "pendingcontactrequestmodel.h"
#include "ringdevicemodel.h"
#include "private/accountmodel_p.h"
#include "private/ringdevicemodel_p.h"
#include "accountstatusmodel.h"
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "dbus/instancemanager.h"
#include "codecmodel.h"
#include "private/pendingcontactrequestmodel_p.h"
#include "person.h"
#include "private/vcardutils.h"
#include "phonedirectorymodel.h"
#include "bannedcontactmodel.h"
#include "contactmethod.h"
#include "database.h"

QHash<QByteArray,AccountPlaceHolder*> AccountModelPrivate::m_hsPlaceHolder;

AccountModelPrivate::AccountModelPrivate(AccountModel* parent) : QObject(parent),q_ptr(parent),
m_pIP2IP(nullptr),m_pProtocolModel(nullptr),m_pSelectionModel(nullptr),m_lMimes({RingMimes::ACCOUNT}),
m_lSupportedProtocols {{
   /* SIP  */ false,
   /* RING */ false,
}}
{}

///Constructors
AccountModel::AccountModel()
    : QAbstractListModel(QCoreApplication::instance())
    , d_ptr(new AccountModelPrivate(this))
{}

void AccountModelPrivate::init()
{
    InstanceManager::instance(); // Make sure the daemon is running before calling updateAccounts()
    q_ptr->updateAccounts();

    CallManagerInterface& callManager = CallManager::instance();
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

    connect(&configurationManager, &ConfigurationManagerInterface::registrationStateChanged,this ,
            &AccountModelPrivate::slotDaemonAccountChanged, Qt::QueuedConnection);
    connect(&configurationManager, SIGNAL(accountsChanged())                               ,q_ptr,
            SLOT(updateAccounts()), Qt::QueuedConnection);
    connect(&callManager         , SIGNAL(voiceMailNotify(QString,int))                    ,this ,
            SLOT(slotVoiceMailNotify(QString,int))  );
    connect(&configurationManager, SIGNAL(volatileAccountDetailsChanged(QString,MapStringString)),this,
            SLOT(slotVolatileAccountDetailsChange(QString,MapStringString)), Qt::QueuedConnection);
    connect(&configurationManager, SIGNAL(mediaParametersChanged(QString))                 ,this ,
            SLOT(slotMediaParametersChanged(QString)), Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::incomingTrustRequest, this,
            &AccountModelPrivate::slotIncomingContactRequest, Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::knownDevicesChanged, this,
            &AccountModelPrivate::slotKownDevicesChanged, Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::exportOnRingEnded, this,
            &AccountModelPrivate::slotExportOnRingEnded, Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::migrationEnded, this,
            &AccountModelPrivate::slotMigrationEnded, Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::contactAdded, this,
            &AccountModelPrivate::slotContactAdded, Qt::QueuedConnection);
    connect(&configurationManager, &ConfigurationManagerInterface::contactRemoved, this,
            &AccountModelPrivate::slotContactRemoved, Qt::QueuedConnection);
}

///Destructor
AccountModel::~AccountModel()
{
   while(d_ptr->m_lAccounts.size()) {
      Account* a = d_ptr->m_lAccounts[0];
      d_ptr->m_lAccounts.remove(0);
      delete a;
   }
   for(Account* a : d_ptr->m_pRemovedAccounts) {
      delete a;
   }
   delete d_ptr;
}

#define CAST(item) static_cast<int>(item)
QHash<int,QByteArray> AccountModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(CAST(Account::Role::Alias                       ) ,QByteArray("alias"                         ));
      roles.insert(CAST(Account::Role::Proto                       ) ,QByteArray("protocol"                      ));
      roles.insert(CAST(Account::Role::Hostname                    ) ,QByteArray("hostname"                      ));
      roles.insert(CAST(Account::Role::Username                    ) ,QByteArray("username"                      ));
      roles.insert(CAST(Account::Role::Mailbox                     ) ,QByteArray("mailbox"                       ));
      roles.insert(CAST(Account::Role::Proxy                       ) ,QByteArray("proxy"                         ));
      roles.insert(CAST(Account::Role::TlsPassword                 ) ,QByteArray("tlsPassword"                   ));
      roles.insert(CAST(Account::Role::TlsCaListCertificate        ) ,QByteArray("tlsCaListCertificate"          ));
      roles.insert(CAST(Account::Role::TlsCertificate              ) ,QByteArray("tlsCertificate"                ));
      roles.insert(CAST(Account::Role::TlsServerName               ) ,QByteArray("tlsServerName"                 ));
      roles.insert(CAST(Account::Role::SipStunServer               ) ,QByteArray("sipStunServer"                 ));
      roles.insert(CAST(Account::Role::PublishedAddress            ) ,QByteArray("publishedAddress"              ));
      roles.insert(CAST(Account::Role::RingtonePath                ) ,QByteArray("ringtonePath"                  ));
      roles.insert(CAST(Account::Role::RegistrationExpire          ) ,QByteArray("registrationExpire"            ));
      roles.insert(CAST(Account::Role::TlsNegotiationTimeoutSec    ) ,QByteArray("tlsNegotiationTimeoutSec"      ));
      roles.insert(CAST(Account::Role::TlsNegotiationTimeoutMsec   ) ,QByteArray("tlsNegotiationTimeoutMsec"     ));
      roles.insert(CAST(Account::Role::LocalPort                   ) ,QByteArray("localPort"                     ));
      roles.insert(CAST(Account::Role::BootstrapPort               ) ,QByteArray("bootstrapPort"                 ));
      roles.insert(CAST(Account::Role::PublishedPort               ) ,QByteArray("publishedPort"                 ));
      roles.insert(CAST(Account::Role::Enabled                     ) ,QByteArray("enabled"                       ));
      roles.insert(CAST(Account::Role::AutoAnswer                  ) ,QByteArray("autoAnswer"                    ));
      roles.insert(CAST(Account::Role::TlsVerifyServer             ) ,QByteArray("tlsVerifyServer"               ));
      roles.insert(CAST(Account::Role::TlsVerifyClient             ) ,QByteArray("tlsVerifyClient"               ));
      roles.insert(CAST(Account::Role::TlsRequireClientCertificate ) ,QByteArray("tlsRequireClientCertificate"   ));
      roles.insert(CAST(Account::Role::TlsEnabled                  ) ,QByteArray("tlsEnabled"                    ));
      roles.insert(CAST(Account::Role::SrtpRtpFallback             ) ,QByteArray("srtpRtpFallback"               ));
      roles.insert(CAST(Account::Role::SipStunEnabled              ) ,QByteArray("sipStunEnabled"                ));
      roles.insert(CAST(Account::Role::PublishedSameAsLocal        ) ,QByteArray("publishedSameAsLocal"          ));
      roles.insert(CAST(Account::Role::RingtoneEnabled             ) ,QByteArray("ringtoneEnabled"               ));
      roles.insert(CAST(Account::Role::dTMFType                    ) ,QByteArray("dTMFType"                      ));
      roles.insert(CAST(Account::Role::Id                          ) ,QByteArray("id"                            ));
      roles.insert(CAST(Account::Role::Object                      ) ,QByteArray("object"                        ));
      roles.insert(CAST(Account::Role::TypeName                    ) ,QByteArray("typeName"                      ));
      roles.insert(CAST(Account::Role::PresenceStatus              ) ,QByteArray("presenceStatus"                ));
      roles.insert(CAST(Account::Role::PresenceMessage             ) ,QByteArray("presenceMessage"               ));
      roles.insert(CAST(Account::Role::UsedForOutgogingCall        ) ,QByteArray("usedForOutgogingCall"          ));
      roles.insert(CAST(Account::Role::TotalCallCount              ) ,QByteArray("totalCallCount"                ));
      roles.insert(CAST(Account::Role::WeekCallCount               ) ,QByteArray("weekCallCount"                 ));
      roles.insert(CAST(Account::Role::TrimesterCallCount          ) ,QByteArray("trimesterCallCount"            ));
      roles.insert(CAST(Account::Role::LastUsed                    ) ,QByteArray("lastUsed"                      ));
      roles.insert(CAST(Account::Role::UserAgent                   ) ,QByteArray("userAgent"                     ));
      roles.insert(CAST(Account::Role::Password                    ) ,QByteArray("password"                      ));
      roles.insert(CAST(Account::Role::SupportPresencePublish      ) ,QByteArray("supportPresencePublish"        ));
      roles.insert(CAST(Account::Role::SupportPresenceSubscribe    ) ,QByteArray("supportPresenceSubscribe"      ));
      roles.insert(CAST(Account::Role::PresenceEnabled             ) ,QByteArray("presenceEnabled"               ));
      roles.insert(CAST(Account::Role::IsVideoEnabled              ) ,QByteArray("isVideoEnabled"                ));
      roles.insert(CAST(Account::Role::VideoPortMax                ) ,QByteArray("videoPortMax"                  ));
      roles.insert(CAST(Account::Role::VideoPortMin                ) ,QByteArray("videoPortMin"                  ));
      roles.insert(CAST(Account::Role::AudioPortMin                ) ,QByteArray("audioPortMin"                  ));
      roles.insert(CAST(Account::Role::AudioPortMax                ) ,QByteArray("audioPortMax"                  ));
      roles.insert(CAST(Account::Role::IsUpnpEnabled               ) ,QByteArray("upnpEnabled"                   ));
      roles.insert(CAST(Account::Role::HasCustomUserAgent          ) ,QByteArray("hasCustomUserAgent"            ));
      roles.insert(CAST(Account::Role::LastTransportErrorCode      ) ,QByteArray("lastTransportErrorCode"        ));
      roles.insert(CAST(Account::Role::LastTransportErrorMessage   ) ,QByteArray("lastTransportErrorMessage"     ));
      roles.insert(CAST(Account::Role::UserAgent                   ) ,QByteArray("userAgent"                     ));
      roles.insert(CAST(Account::Role::UseDefaultPort              ) ,QByteArray("useDefaultPort"                ));
      roles.insert(CAST(Account::Role::TurnServer                  ) ,QByteArray("turnServer"                    ));
      roles.insert(CAST(Account::Role::HasProxy                    ) ,QByteArray("hasProxy"                      ));
      roles.insert(CAST(Account::Role::DisplayName                 ) ,QByteArray("displayName"                   ));
      roles.insert(CAST(Account::Role::SrtpEnabled                 ) ,QByteArray("srtpEnabled"                   ));
      roles.insert(CAST(Account::Role::HasCustomBootstrap          ) ,QByteArray("hasCustomBootstrap"            ));
      roles.insert(CAST(Account::Role::CredentialModel             ) ,QByteArray("credentialModel"               ));
      roles.insert(CAST(Account::Role::CodecModel                  ) ,QByteArray("codecModel"                    ));
      roles.insert(CAST(Account::Role::KeyExchangeModel            ) ,QByteArray("keyExchangeModel"              ));
      roles.insert(CAST(Account::Role::CipherModel                 ) ,QByteArray("cipherModel"                   ));
      roles.insert(CAST(Account::Role::StatusModel                 ) ,QByteArray("statusModel"                   ));
      roles.insert(CAST(Account::Role::SecurityEvaluationModel     ) ,QByteArray("securityEvaluationModel"       ));
      roles.insert(CAST(Account::Role::TlsMethodModel              ) ,QByteArray("tlsMethodModel"                ));
      roles.insert(CAST(Account::Role::ProtocolModel               ) ,QByteArray("protocolModel"                 ));
      roles.insert(CAST(Account::Role::BootstrapModel              ) ,QByteArray("bootstrapModel"                ));
      roles.insert(CAST(Account::Role::NetworkInterfaceModel       ) ,QByteArray("networkInterfaceModel"         ));
      roles.insert(CAST(Account::Role::KnownCertificateModel       ) ,QByteArray("knownCertificateModel"         ));
      roles.insert(CAST(Account::Role::BannedCertificatesModel     ) ,QByteArray("bannedCertificatesModel"       ));
      roles.insert(CAST(Account::Role::AllowedCertificatesModel    ) ,QByteArray("allowedCertificatesModel"      ));
      roles.insert(CAST(Account::Role::AllowIncomingFromHistory    ) ,QByteArray("allowIncomingFromHistory"      ));
      roles.insert(CAST(Account::Role::AllowIncomingFromContact    ) ,QByteArray("allowIncomingFromContact"      ));
      roles.insert(CAST(Account::Role::AllowIncomingFromUnknown    ) ,QByteArray("allowIncomingFromUnknown"      ));
      roles.insert(CAST(Account::Role::ActiveCallLimit             ) ,QByteArray("activeCallLimit"               ));
      roles.insert(CAST(Account::Role::HasActiveCallLimit          ) ,QByteArray("hasActiveCallLimit"            ));
      roles.insert(CAST(Account::Role::SecurityLevel               ) ,QByteArray("securityLevel"                 ));
      roles.insert(CAST(Account::Role::SecurityLevelIcon           ) ,QByteArray("securityLevelIcon"             ));
      roles.insert(CAST(Account::Role::TurnServerUsername          ) ,QByteArray("turnServerUsername"            ));
      roles.insert(CAST(Account::Role::TurnServerPassword          ) ,QByteArray("turnServerPassword"            ));
      roles.insert(CAST(Account::Role::TurnServerRealm             ) ,QByteArray("turnServerRealm"               ));
      roles.insert(CAST(Account::Role::TurnServerEnabled           ) ,QByteArray("turnEnabled"                   ));
      roles.insert(CAST(Account::Role::TlsPrivateKey               ) ,QByteArray("tlsPrivateKey"                 ));
      roles.insert(CAST(Account::Role::LastStatusChangeTimeStamp   ) ,QByteArray("lastStatusChangeTimeStamp"     ));
      roles.insert(CAST(Account::Role::RegisteredName              ) ,QByteArray("registeredName"                ));

   }
   return roles;
}
#undef CAST

///Get the first IP2IP account
Account* AccountModel::ip2ip() const
{
   if (!d_ptr->m_pIP2IP) {
      foreach(Account* a, d_ptr->m_lAccounts) {
         if (a->isIp2ip())
            d_ptr->m_pIP2IP = a;
      }
   }
   return d_ptr->m_pIP2IP;
}

///Singleton
AccountModel& AccountModel::instance()
{
    static auto instance = new AccountModel;

    // Upload account configuration only once in re-entrant way
    static std::atomic_flag init_flag {ATOMIC_FLAG_INIT};
    if (not init_flag.test_and_set())
        instance->d_ptr->init();

    return *instance;
}

QItemSelectionModel* AccountModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel)
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<AccountModel*>(this));

   return d_ptr->m_pSelectionModel;
}


QItemSelectionModel* AccountModel::userSelectionModel() const
{
   if (!d_ptr->m_pUserSelectionModel)
      d_ptr->m_pUserSelectionModel = new QItemSelectionModel(const_cast<AccountModel*>(this));

   return d_ptr->m_pUserSelectionModel;
}

/**
 * returns the select account
 */
Account*
AccountModel::selectedAccount() const
{
   auto accIdx = AccountModel::instance().selectionModel()->currentIndex();
   return AccountModel::instance().getAccountByModelIndex(accIdx);
}

/**
 * Helper method to update the selected index in user selection model
 */
void
AccountModel::setUserChosenAccount(Account* account)
{
    if (!account) {
        return;
    }

    const auto idx = account->index();

    userSelectionModel()->setCurrentIndex(
        idx, QItemSelectionModel::ClearAndSelect
    );
}

/**
 * returns the user chosen account
 */
Account*
AccountModel::userChosenAccount() const
{
   auto accIdx = AccountModel::instance().userSelectionModel()->currentIndex();
   return AccountModel::instance().getAccountByModelIndex(accIdx);
}

QList<Account*> AccountModel::accountsToMigrate() const
{
    QList<Account*> accounts;
    foreach(Account* account, d_ptr->m_lAccounts) {
        if (account->needsMigration())
            accounts << account;
    }
    return accounts;
}

/**
 * returns a vector of contacts from the daemon
 * @param account the account to query
 * @return contacts a QVector<QMap<QString, QString>>
 */
QVector<QMap<QString, QString>>
AccountModel::getContacts(const Account* account) const
{
    return ConfigurationManager::instance().getContacts(account->id().data());
}

///Account status changed
void AccountModelPrivate::slotDaemonAccountChanged(const QString& account, const QString& registration_state, unsigned code, const QString& status)
{
   Q_UNUSED(registration_state);
   Account* a = q_ptr->getById(account.toLatin1());

   //TODO move this to AccountStatusModel
   if (!a || (a && a->lastSipRegistrationStatus() != status )) {
      if (status != "OK") //Do not pollute the log
         qDebug() << "Account" << account << "status changed to" << status;
   }

   if (a)
      a->setLastSipRegistrationStatus(status);

   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();

   //The account may have been deleted by the user, but 'apply' have not been pressed
   if (!a) {
      qDebug() << "received account changed for non existing account" << account;
      const QStringList accountIds = configurationManager.getAccountList();
      for (int i = 0; i < accountIds.size(); ++i) {
         if ((!q_ptr->getById(accountIds[i].toLatin1())) && m_lDeletedAccounts.indexOf(accountIds[i]) == -1) {
            Account* acc = Account::buildExistingAccountFromId(accountIds[i].toLatin1());
            qDebug() << "building missing account" << accountIds[i];
            insertAccount(acc,i);
            connect(acc, &Account::changed                , this, &AccountModelPrivate::slotAccountChanged                );
            connect(acc, &Account::presenceEnabledChanged , this, &AccountModelPrivate::slotAccountPresenceEnabledChanged );
            connect(acc, &Account::enabled                , this, &AccountModelPrivate::slotSupportedProtocolsChanged     );
            emit q_ptr->dataChanged(q_ptr->index(i,0),q_ptr->index(q_ptr->size()-1));
            emit q_ptr->layoutChanged();

            if (!acc->isIp2ip())
               enableProtocol(acc->protocol());

         }
      }

      // remove any accounts that are not found in the daemon and which are marked to be REMOVED
      // its not clear to me why this would ever happen in the first place, but the code does seem
      // to be able to enter into this state...
      QMutableVectorIterator<Account *> accIter(m_lAccounts);
      int modelRow = 0;
      while (accIter.hasNext()) {
          auto acc = accIter.next();
          const int daemonRow = accountIds.indexOf(acc->id());

          if (daemonRow == -1 && (acc->editState() == Account::EditState::READY || acc->editState() == Account::EditState::REMOVED)) {
              q_ptr->beginRemoveRows(QModelIndex(), modelRow, modelRow);
              accIter.remove();
              q_ptr->endRemoveRows();

              // should we put it in the list of deleted accounts? who knows?

              // decrement which row we're on
              --modelRow;
          }
          // we're going to the next row
          ++modelRow;
      }
   }
   else {
      const bool isRegistered = a->registrationState() == Account::RegistrationState::READY;
      a->updateState();
      const QModelIndex idx = a->index();
      emit q_ptr->dataChanged(idx, idx);
      const bool regStateChanged = isRegistered != (a->registrationState() == Account::RegistrationState::READY);

      //Handle some important events directly
      if (regStateChanged && (code == 502 || code == 503)) {
         emit q_ptr->badGateway();
      }
      else if (regStateChanged)
         emit q_ptr->registrationChanged(a,a->registrationState() == Account::RegistrationState::READY);

      //Send the messages to AccountStatusModel for processing
      a->statusModel()->addSipRegistrationEvent(status,code);

      //Make sure volatile details get reloaded
      //TODO eventually remove this call and trust the signal
      slotVolatileAccountDetailsChange(account,configurationManager.getVolatileAccountDetails(account));

      emit q_ptr->accountStateChanged(a,a->registrationState());
   }

}

void AccountModelPrivate::slotSupportedProtocolsChanged()
{
    emit q_ptr->supportedProtocolsChanged();
}

///Tell the model something changed
void AccountModelPrivate::slotAccountChanged(Account* a)
{
   int idx = m_lAccounts.indexOf(a);
   if (idx != -1) {
      emit q_ptr->dataChanged(q_ptr->index(idx, 0), q_ptr->index(idx, 0));
   }
}


/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/


///When a new voice mail is available
void AccountModelPrivate::slotVoiceMailNotify(const QString &accountID, int count)
{
   Account* a = q_ptr->getById(accountID.toLatin1());
   if (a) {
      a->setVoiceMailCount(count);
      emit q_ptr->voiceMailNotify(a,count);
   }
}

///Propagate account presence state
void AccountModelPrivate::slotAccountPresenceEnabledChanged(bool state)
{
   Q_UNUSED(state)
   emit q_ptr->presenceEnabledChanged(q_ptr->isPresenceEnabled());
}

///Emitted when some runtime details changes
void AccountModelPrivate::slotVolatileAccountDetailsChange(const QString& accountId, const MapStringString& details)
{
   Account* a = q_ptr->getById(accountId.toLatin1());
   if (a) {
      const int     transportCode = details[DRing::Account::VolatileProperties::Transport::STATE_CODE].toInt();
      const QString transportDesc = details[DRing::Account::VolatileProperties::Transport::STATE_DESC];
      const QString status        = details[DRing::Account::VolatileProperties::Registration::STATUS];

      a->statusModel()->addTransportEvent(transportDesc,transportCode);

      a->setLastTransportCode(transportCode);
      a->setLastTransportMessage(transportDesc);

      const Account::RegistrationState state = Account::fromDaemonName(a->accountDetail(DRing::Account::ConfProperties::Registration::STATUS));
      a->setRegistrationState(state);
   }
}

///When a Ring-DHT trust request arrive
void AccountModelPrivate::slotIncomingContactRequest(const QString& accountId, const QString& ringID, const QByteArray& payload, time_t time)
{
   Q_UNUSED(payload);

   Account* a = q_ptr->getById(accountId.toLatin1());

   if (!a) {
      qWarning() << "Incoming trust request for unknown account" << accountId;
      return;
   }

   /* do not pass a person before the contact request was added to his model */
   ContactRequest* r = new ContactRequest(a, nullptr, ringID, time);
   a->pendingContactRequestModel()->d_ptr->addRequest(r);

   auto contactMethod = PhoneDirectoryModel::instance().getNumber(ringID, a);
   r->setPeer(VCardUtils::mapToPersonFromReceivedProfile(contactMethod, payload));

    /* add to database */
    DataBase::instance().addContact(ringID, payload);

}

///Known Ring devices have changed
void AccountModelPrivate::slotKownDevicesChanged(const QString& accountId, const MapStringString& accountDevices)
{
   qDebug() << "Known devices changed" << accountId;

   Account* a = q_ptr->getById(accountId.toLatin1());

   if (!a) {
      qWarning() << "Known devices changed for unknown account" << accountId;
      return;
  }

   a->ringDeviceModel()->d_ptr->reload(accountDevices);
}

///Export on Ring ended
void AccountModelPrivate::slotExportOnRingEnded(const QString& accountId, int status, const QString& pin)
{
   qDebug() << "Export on ring ended" << accountId;

   Account* a = q_ptr->getById(accountId.toLatin1());

   if (!a) {
      qWarning() << "export on Ring ended for unknown account" << accountId;
      return;
  }

  emit a->exportOnRingEnded(static_cast<Account::ExportOnRingStatus>(status), pin);
}

/// Migration ended
void
AccountModelPrivate::slotMigrationEnded(const QString& accountId, const QString& result)
{
    Account* a = q_ptr->getById(accountId.toLatin1());

    Account::MigrationEndedStatus status;
    if(result == "SUCCESS")
        status = Account::MigrationEndedStatus::SUCCESS;
    else if(result == "INVALID")
        status = Account::MigrationEndedStatus::INVALID;
    else
        status = Account::MigrationEndedStatus::UNDEFINED_STATUS;


    if (status == Account::MigrationEndedStatus::UNDEFINED_STATUS)
        qWarning() << "cannot emit migrationEnded signal, status is undefined";
    else
        emit a->migrationEnded(status);
}


/// slot function used with ConfigurationManagerInterface::contactAdded signal
void
AccountModelPrivate::slotContactAdded(const QString &accountID, const QString &uri, bool confirmed)
{
    if (auto account = q_ptr->getById(accountID.toLatin1())) {
        if (auto cm = PhoneDirectoryModel::instance().getNumber(uri, account)) {
            cm->setConfirmed(confirmed);
            auto& daemon_contacts = account->getContacts();
            if (not daemon_contacts.contains(cm))
                daemon_contacts << cm;
        }
    }
}

/**
 * slot function used with ConfigurationManagerInterface::contactRemoved signal
 */
void
AccountModelPrivate::slotContactRemoved(const QString &accountID, const QString &uri, bool banned)
{
    if (auto account = q_ptr->getById(accountID.toLatin1())) {
        if (auto cm = PhoneDirectoryModel::instance().getNumber(uri, account)) {
            auto& daemon_contacts = account->getContacts();
            // TODO: removeAll() is 5.4 - not yet supported by debian 8
            auto index = daemon_contacts.indexOf(cm);
            if (index >= 0)
                daemon_contacts.remove(index);
            if (banned)
                account->bannedContactModel()->add(cm);
        }
    }
}

///Update accounts
void AccountModel::update()
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManager::instance();
   QList<Account*> tmp;
   for (int i = 0; i < d_ptr->m_lAccounts.size(); i++)
      tmp << d_ptr->m_lAccounts[i];

   for (int i = 0; i < tmp.size(); i++) {
      Account* current = tmp[i];
      if (!current->isNew() && (current->editState() != Account::EditState::NEW
         && current->editState() != Account::EditState::MODIFIED_COMPLETE
         && current->editState() != Account::EditState::MODIFIED_INCOMPLETE
         && current->editState() != Account::EditState::OUTDATED))
         remove(current);
   }
   //ask for the list of accounts ids to the configurationManager
   const QStringList accountIds = configurationManager.getAccountList();
   for (int i = 0; i < accountIds.size(); ++i) {
      if (d_ptr->m_lDeletedAccounts.indexOf(accountIds[i]) == -1) {
         Account* a = Account::buildExistingAccountFromId(accountIds[i].toLatin1());
         d_ptr->insertAccount(a,i);
         emit dataChanged(index(i,0),index(size()-1,0));
         connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
         //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));
         connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
         emit layoutChanged();

         if (!a->isIp2ip())
            d_ptr->enableProtocol(a->protocol());
      }
   }
} //update

///Update accounts
void AccountModel::updateAccounts()
{
   qDebug() << "Updating all accounts";
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   QStringList accountIds = configurationManager.getAccountList();

   // Detect removed accounts
   foreach(Account* account, d_ptr->m_lAccounts) {
       if (accountIds.indexOf(account->id()) == -1) {
           remove(account);
       }
   }

   //m_lAccounts.clear();
   for (int i = 0; i < accountIds.size(); ++i) {
      Account* acc = getById(accountIds[i].toLatin1());
      if (!acc) {
         Account* a = Account::buildExistingAccountFromId(accountIds[i].toLatin1());
         d_ptr->insertAccount(a,d_ptr->m_lAccounts.size());
         connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
         //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));
         connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
         emit dataChanged(index(size()-1,0),index(size()-1,0));

         if (!a->isIp2ip())
            d_ptr->enableProtocol(a->protocol());

         emit accountAdded(a);
      }
      else {
         acc->performAction(Account::EditAction::RELOAD);
      }
   }
   emit accountListUpdated();
} //updateAccounts

///Save accounts details and reload it
void AccountModel::save()
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   const QStringList accountIds = QStringList(configurationManager.getAccountList());

   //create or update each account from accountList
   for (int i = 0; i < size(); i++) {
      Account* current = (*this)[i];
      current->performAction(Account::EditAction::SAVE);
   }

   //remove accounts that are in the configurationManager but not in the client
   for (int i = 0; i < accountIds.size(); i++) {
      if(!getById(accountIds[i].toLatin1())) {
         configurationManager.removeAccount(accountIds[i]);
      }
   }

   //Set account order
   QString order;
   for( int i = 0 ; i < size() ; i++)
      order += d_ptr->m_lAccounts[i]->id() + '/';
   configurationManager.setAccountsOrder(order);
   d_ptr->m_lDeletedAccounts.clear();
}

int AccountModel::exportAccounts(const QStringList& accountIDs, const QString& filePath, const QString& password)
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    return configurationManager.exportAccounts(accountIDs, filePath, password);
}

int AccountModel::importAccounts(const QString& filePath, const QString& password)
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    return configurationManager.importAccounts(filePath, password);
}

///Move account up
bool AccountModel::moveUp()
{
   if (d_ptr->m_pSelectionModel) {
      const QModelIndex& idx = d_ptr->m_pSelectionModel->currentIndex();

      if (!idx.isValid())
         return false;

      if (dropMimeData(mimeData({idx}), Qt::MoveAction, idx.row()-1, idx.column(),idx.parent())) {
         return true;
      }
   }
   return false;
}

///Move account down
bool AccountModel::moveDown()
{
   if (d_ptr->m_pSelectionModel) {
      const QModelIndex& idx = d_ptr->m_pSelectionModel->currentIndex();

      if (!idx.isValid())
         return false;

      if (dropMimeData(mimeData({idx}), Qt::MoveAction, idx.row()+1, idx.column(),idx.parent())) {
         return true;
      }
   }
   return false;
}

///Try to register all enabled accounts
void AccountModel::registerAllAccounts()
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   configurationManager.registerAllAccounts();
}

///Cancel all modifications
void AccountModel::cancel() {
   foreach (Account* a, d_ptr->m_lAccounts) {
      // Account::EditState::NEW is only for new and unmodified accounts
      if (a->isNew())
         remove(a);
      else {
         switch(a->editState()) {
            case Account::EditState::NEW                :
               remove(a);
               break;
            case Account::EditState::MODIFIED_INCOMPLETE:
            case Account::EditState::MODIFIED_COMPLETE  :
               a << Account::EditAction::CANCEL;
               break;
            case Account::EditState::OUTDATED           :
               a << Account::EditAction::RELOAD;
               break;
            case Account::EditState::READY              :
            case Account::EditState::REMOVED            :
            case Account::EditState::EDITING            :
            case Account::EditState::COUNT__            :
               break;
         }
      }
   }
   d_ptr->m_lDeletedAccounts.clear();
}


void AccountModelPrivate::enableProtocol(Account::Protocol proto)
{
   const bool cache = m_lSupportedProtocols[proto];

   //Set the supported protocol bits, for now, this intentionally ignore account states
   m_lSupportedProtocols.setAt(proto, true);

   if (!cache) {
      emit q_ptr->supportedProtocolsChanged();
   }
}

AccountModel::EditState AccountModelPrivate::convertAccountEditState(const Account::EditState s)
{
   AccountModel::EditState ams = AccountModel::EditState::INVALID;

   switch (s) {
      case Account::EditState::READY              :
      case Account::EditState::OUTDATED           :
      case Account::EditState::EDITING            :
      case Account::EditState::COUNT__            :
         ams = AccountModel::EditState::SAVED;
         break;
      case Account::EditState::MODIFIED_INCOMPLETE:
         ams = AccountModel::EditState::INVALID;
         break;
      case Account::EditState::NEW                :
      case Account::EditState::REMOVED            :
      case Account::EditState::MODIFIED_COMPLETE  :
         ams = AccountModel::EditState::UNSAVED;
         break;
   }

   return ams;
}

///Check if the AccountModel need/can be saved as a whole
AccountModel::EditState AccountModel::editState() const
{
   typedef AccountModel::EditState  ES ;
   typedef const Account::EditState AES;

   static ES s_CurrentState = ES::INVALID;

   //This class is a singleton, so using static variables is ok
   static Matrix1D<ES,int> estates = {
      { ES::SAVED   , 0},
      { ES::UNSAVED , 0},
      { ES::INVALID , 0},
   };

   auto genState = [this]( const Account* a, AES s, AES p ) {
      Q_UNUSED(a)

      const ES newState = d_ptr->convertAccountEditState(s);
      const ES oldState = d_ptr->convertAccountEditState(p);

      if (newState != oldState)
         estates.setAt(oldState,estates[oldState]-1);

      estates.setAt(newState,estates[newState]+1);

      const ES oldGlobalState = s_CurrentState;

      s_CurrentState = estates[ES::INVALID] ? ES::INVALID:
                       estates[ES::UNSAVED] ? ES::UNSAVED:
                                              ES::SAVED  ;

      if (s_CurrentState != oldGlobalState)
         emit editStateChanged(s_CurrentState, oldGlobalState);

   };

   static bool isInit = false;
   if (!isInit) {
      isInit = true;

      for (const Account* a : d_ptr->m_lAccounts) {
         genState(a,a->editState(),a->editState());
      }

      connect(this, &AccountModel::accountEditStateChanged, genState);
   }


   return s_CurrentState;
}

///Called when codec bitrate changes
void AccountModelPrivate::slotMediaParametersChanged(const QString& accountId)
{
   Account* a = q_ptr->getById(accountId.toLatin1());
   if (a) {
      if (auto codecModel = a->codecModel()) {
         qDebug() << "reloading codecs";
         codecModel << CodecModel::EditAction::RELOAD;
      }
   }
}

/*****************************************************************************
 *                                                                           *
 *                                  Getters                                  *
 *                                                                           *
 ****************************************************************************/

/**
 * Get an account by its ID
 *
 * @note This method have O(N) complexity, but the average account count is low
 *
 * @param id The account identifier
 * @param usePlaceHolder Return a placeholder for a future account instead of nullptr
 * @return an account if it exist, a placeholder if usePlaceHolder==true or nullptr
 */
Account* AccountModel::getById(const QByteArray& id, bool usePlaceHolder) const
{
   if (id.isEmpty())
       return nullptr;
   //This function use a loop as the expected size is < 5
   for (int i = 0; i < d_ptr->m_lAccounts.size(); i++) {
      Account* acc = d_ptr->m_lAccounts[i];
      if (acc && !acc->isNew() && acc->id() == id)
         return acc;
   }

   //The account doesn't exist (yet)
   if (usePlaceHolder) {
      AccountPlaceHolder* ph =  d_ptr->m_hsPlaceHolder[id];
      if (!ph) {
         ph = new AccountPlaceHolder(id);
         d_ptr->m_hsPlaceHolder[id] = ph;
      }
      return ph;
   }

   return nullptr;
}

///Get the account size
int AccountModel::size() const
{
   return d_ptr->m_lAccounts.size();
}

///Get data from the model
QVariant AccountModel::data ( const QModelIndex& idx, int role) const
{
   if (!idx.isValid() || idx.row() < 0 || idx.row() >= rowCount())
      return QVariant();

   return d_ptr->m_lAccounts[idx.row()]->roleData(role);
} //data

///Flags for "idx"
Qt::ItemFlags AccountModel::flags(const QModelIndex& idx) const
{
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
   return QAbstractItemModel::flags(idx);
}

///Number of account
int AccountModel::rowCount(const QModelIndex& parentIdx) const
{
   return parentIdx.isValid() ? 0 : d_ptr->m_lAccounts.size();
}

Account* AccountModel::getAccountByModelIndex(const QModelIndex& item) const
{
   if (!item.isValid())
      return nullptr;
   return d_ptr->m_lAccounts[item.row()];
}

///Generate an unique suffix to prevent multiple account from sharing alias
QString AccountModel::getSimilarAliasIndex(const QString& alias)
{
    auto& self = instance();

    int count = 0;
    foreach (Account* a, self.d_ptr->m_lAccounts) {
        if (a->alias().left(alias.size()) == alias)
            count++;
    }
    bool found = true;
    do {
        found = false;
        foreach (Account* a, self.d_ptr->m_lAccounts) {
            if (a->alias() == alias+QString(" (%1)").arg(count)) {
                count++;
                found = false;
                break;
            }
        }
    } while(found);
    if (count)
        return QString(" (%1)").arg(count);
    return QString();
}

QList<Account*> AccountModel::getAccountsByProtocol( const Account::Protocol protocol ) const
{
   switch(protocol) {
      case Account::Protocol::SIP:
         return d_ptr->m_lSipAccounts;
      case Account::Protocol::RING:
         return d_ptr->m_lRingAccounts;
      case Account::Protocol::COUNT__:
         break;
   }

   return {};
}

bool AccountModel::isPresenceEnabled() const
{
   foreach(Account* a, d_ptr->m_lAccounts) {
      if (a->presenceEnabled())
         return true;
   }
   return false;
}

bool AccountModel::isPresencePublishSupported() const
{
   foreach(Account* a,d_ptr->m_lAccounts) {
      if (a->supportPresencePublish())
         return true;
   }
   return false;
}

bool AccountModel::isPresenceSubscribeSupported() const
{
   foreach(Account* a,d_ptr->m_lAccounts) {
      if (a->supportPresenceSubscribe())
         return true;
   }
   return false;
}

bool AccountModel::isSipSupported() const
{
   return d_ptr->m_lSupportedProtocols[Account::Protocol::SIP];
}

bool AccountModel::isIP2IPSupported() const
{
    if (auto a = ip2ip())
        return a->isEnabled();
    return false;
}

bool AccountModel::isRingSupported() const
{
   return d_ptr->m_lSupportedProtocols[Account::Protocol::RING];
}


ProtocolModel* AccountModel::protocolModel() const
{
   if (!d_ptr->m_pProtocolModel)
      d_ptr->m_pProtocolModel = new ProtocolModel();
   return d_ptr->m_pProtocolModel;
}


/*****************************************************************************
 *                                                                           *
 *                                  Setters                                  *
 *                                                                           *
 ****************************************************************************/

///Have a single place where m_lAccounts receive inserts
void AccountModelPrivate::insertAccount(Account* a, int idx)
{
   q_ptr->beginInsertRows(QModelIndex(), idx, idx);
   m_lAccounts.insert(idx,a);
   q_ptr->endInsertRows();

   connect(a,&Account::editStateChanged, [a,this](const Account::EditState state, const Account::EditState previous) {
      emit q_ptr->accountEditStateChanged(a, state, previous);
   });

   // Connect the signal when a contact was added by an account
   connect(a, &Account::contactRequestAccepted, [a, this](const ContactRequest* r){
      emit q_ptr->accountContactAdded(a, r);
   });

   switch(a->protocol()) {
      case Account::Protocol::SIP:
         m_lSipAccounts  << a;
         break;
      case Account::Protocol::RING:
         m_lRingAccounts << a;
         break;
      case Account::Protocol::COUNT__:
         break;
   }
}

void AccountModelPrivate::removeAccount(Account* account)
{
   const int aindex = m_lAccounts.indexOf(account);

   q_ptr->beginRemoveRows(QModelIndex(),aindex,aindex);
   m_lAccounts.remove(aindex);
   m_lDeletedAccounts << account->id();
   q_ptr->endRemoveRows();

   m_pRemovedAccounts << account;

   switch(account->protocol()) {
      case Account::Protocol::RING:
         m_lRingAccounts.removeOne(account);
         break;
      case Account::Protocol::SIP:
         m_lSipAccounts.removeOne(account);
         break;
      case Account::Protocol::COUNT__:
         break;
   }
}

Account* AccountModel::add(const QString& alias, const Account::Protocol proto)
{
   Account* a = Account::buildNewAccountFromAlias(proto,alias);
   connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
   d_ptr->insertAccount(a,d_ptr->m_lAccounts.size());
   connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
   //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));

   emit dataChanged(index(d_ptr->m_lAccounts.size()-1,0), index(d_ptr->m_lAccounts.size()-1,0));

   if (d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel->setCurrentIndex(index(d_ptr->m_lAccounts.size()-1,0), QItemSelectionModel::ClearAndSelect);
   }

   if (!a->isIp2ip())
      d_ptr->enableProtocol(proto);

// Override ringtone path
#if defined(Q_OS_OSX)
    QDir ringtonesDir(QCoreApplication::applicationDirPath());
    ringtonesDir.cdUp();
    ringtonesDir.cd("Resources/ringtones/");
    a->setRingtonePath(ringtonesDir.path()+"/default.wav");
#endif

   emit accountAdded(a);

   editState();

   return a;
}

Account* AccountModel::add(const QString& alias, const QModelIndex& idx)
{
   return add(alias, qvariant_cast<Account::Protocol>(idx.data((int)ProtocolModel::Role::Protocol)));
}

///Remove an account
void AccountModel::remove(Account* account)
{
  if (not account) {
    return;
  }
  qDebug() << "Removing" << account->alias() << account->id();
  d_ptr->removeAccount(account);
  emit accountRemoved(account);
}

void AccountModel::remove(const QModelIndex& idx )
{
   remove(getAccountByModelIndex(idx));
}

///Set model data
bool AccountModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
   if (idx.isValid() && idx.column() == 0 && role == Qt::CheckStateRole) {
      const bool prevEnabled = d_ptr->m_lAccounts[idx.row()]->isEnabled();
      d_ptr->m_lAccounts[idx.row()]->setEnabled(value.toBool());
      emit dataChanged(idx, idx);
      if (prevEnabled != value.toBool())
         emit accountEnabledChanged(d_ptr->m_lAccounts[idx.row()]);
      emit dataChanged(idx, idx);
      return true;
   }
   else if ( role == Qt::EditRole ) {
      if (value.toString() != data(idx,Qt::EditRole)) {
         d_ptr->m_lAccounts[idx.row()]->setAlias(value.toString());
         emit dataChanged(idx, idx);
      }
   }
   return false;
}


/*****************************************************************************
 *                                                                           *
 *                                 Operator                                  *
 *                                                                           *
 ****************************************************************************/

///Get the account from its index
const Account* AccountModel::operator[] (int i) const
{
   return d_ptr->m_lAccounts[i];
}

///Get the account from its index
Account* AccountModel::operator[] (int i)
{
   return d_ptr->m_lAccounts[i];
}

///Get accoutn by id
Account* AccountModel::operator[] (const QByteArray& i) {
   return getById(i);
}

void AccountModel::add(Account* acc)
{
   d_ptr->insertAccount(acc,d_ptr->m_lAccounts.size());
}


/*****************************************************************************
 *                                                                           *
 *                              Drag and drop                                *
 *                                                                           *
 ****************************************************************************/


QStringList AccountModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

bool AccountModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
   Q_UNUSED(action)
   if(parent.isValid() || column > 0) {
      qDebug() << "column invalid";
      return false;
   }

   if (data->hasFormat(RingMimes::ACCOUNT)) {
      int destinationRow = -1;

      if(row < 0) {
         //drop on top
         destinationRow = d_ptr->m_lAccounts.size() - 1;
      }
      else if(row >= d_ptr->m_lAccounts.size()) {
         destinationRow = 0;
      }
      else {
         destinationRow = row;
      }

      Account* dest = getById(data->data(RingMimes::ACCOUNT));
      if (!dest)
         return false;

      const QModelIndex accIdx = dest->index();

      beginRemoveRows(QModelIndex(), accIdx.row(), accIdx.row());
      Account* acc = d_ptr->m_lAccounts[accIdx.row()];
      d_ptr->m_lAccounts.removeAt(accIdx.row());
      endRemoveRows();

      d_ptr->insertAccount(acc,destinationRow);

      d_ptr->m_pSelectionModel->setCurrentIndex(index(destinationRow), QItemSelectionModel::ClearAndSelect);

      return true;
   }

   return false;
}

QMimeData* AccountModel::mimeData(const QModelIndexList& indexes) const
{
   QMimeData* mMimeData = new QMimeData();

   for (const QModelIndex& index : indexes) {
      if (index.isValid()) {
         mMimeData->setData(RingMimes::ACCOUNT, index.data((int)Account::Role::Id).toByteArray());
      }
   }

   return mMimeData;
}

Qt::DropActions AccountModel::supportedDragActions() const
{
   return Qt::MoveAction | Qt::TargetMoveAction;
}

Qt::DropActions AccountModel::supportedDropActions() const
{
   return Qt::MoveAction | Qt::TargetMoveAction;
}

void AccountModel::slotConnectivityChanged()
{
    ConfigurationManager::instance().connectivityChanged();
}

Account* AccountModel::findPlaceHolder(const QByteArray& accountId) const
{
    auto iter = d_ptr->m_hsPlaceHolder.find(accountId);
    if (iter != d_ptr->m_hsPlaceHolder.end())
        return *iter;
    return nullptr;
}

Account* AccountModel::findAccountIf(const std::function<bool(const Account&)>& pred) const
{
    auto iter = std::find_if(std::begin(d_ptr->m_lAccounts), std::end(d_ptr->m_lAccounts),
                             [&](Account* acc){ return acc and pred(*acc); });
    if (iter != std::end(d_ptr->m_lAccounts))
        return *iter;
    return nullptr;
}

#include <accountmodel.moc>

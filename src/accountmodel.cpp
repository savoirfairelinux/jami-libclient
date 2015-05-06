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
#include "accountmodel.h"

//Qt
#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QMimeData>

//Ring daemon
#include <account_const.h>

//Ring library
#include "account.h"
#include "mime.h"
#include "profilemodel.h"
#include "protocolmodel.h"
#include "private/account_p.h"
#include "private/accountmodel_p.h"
#include "accountstatusmodel.h"
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "dbus/instancemanager.h"

QHash<QByteArray,AccountPlaceHolder*> AccountModelPrivate::m_hsPlaceHolder;
AccountModel*     AccountModelPrivate::m_spAccountList;

AccountModelPrivate::AccountModelPrivate(AccountModel* parent) : QObject(parent),q_ptr(parent),
m_pIP2IP(nullptr),m_pProtocolModel(nullptr),m_pSelectionModel(nullptr),m_lMimes({RingMimes::ACCOUNT}),
m_lSupportedProtocols {{
   /* SIP  */ false,
   /* IAX  */ false,
   /* RING */ false,
}}
{
}

///Constructors
///@param fill Whether to fill the list with accounts from configurationManager or not.
AccountModel::AccountModel() : QAbstractListModel(QCoreApplication::instance())
,d_ptr(new AccountModelPrivate(this))
{
   //Make sure the daemon is running as this can be called first
   DBus::InstanceManager::instance();
}

///Prevent constructor loop
void AccountModelPrivate::init()
{
   q_ptr->updateAccounts();
   CallManagerInterface& callManager = DBus::CallManager::instance();
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   connect(&configurationManager, &ConfigurationManagerInterface::registrationStateChanged,this ,
      &AccountModelPrivate::slotDaemonAccountChanged);
   connect(&configurationManager, SIGNAL(accountsChanged())                               ,q_ptr,
      SLOT(updateAccounts())                  );
   connect(&callManager         , SIGNAL(voiceMailNotify(QString,int))                    ,this ,
      SLOT(slotVoiceMailNotify(QString,int))  );
   connect(&configurationManager, SIGNAL(volatileAccountDetailsChanged(QString,MapStringString)),this,
      SLOT(slotVolatileAccountDetailsChange(QString,MapStringString)));

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
      roles.insert(CAST(Account::Role::TlsPrivateKeyCertificate    ) ,QByteArray("tlsPrivateKeyCertificate"      ));
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
      roles.insert(CAST(Account::Role::DisplaySasOnce              ) ,QByteArray("displaySasOnce"                ));
      roles.insert(CAST(Account::Role::SrtpRtpFallback             ) ,QByteArray("srtpRtpFallback"               ));
      roles.insert(CAST(Account::Role::ZrtpDisplaySas              ) ,QByteArray("zrtpDisplaySas"                ));
      roles.insert(CAST(Account::Role::ZrtpNotSuppWarning          ) ,QByteArray("zrtpNotSuppWarning"            ));
      roles.insert(CAST(Account::Role::ZrtpHelloHash               ) ,QByteArray("zrtpHelloHash"                 ));
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
      roles.insert(CAST(Account::Role::SipTurnServer               ) ,QByteArray("sipTurnServer"                 ));
      roles.insert(CAST(Account::Role::SipTurnEnabled              ) ,QByteArray("sipTurnEnabled"                ));
   }
   return roles;
}
#undef CAST

///Get the IP2IP account
Account* AccountModel::ip2ip() const
{
   if (!d_ptr->m_pIP2IP) {
      foreach(Account* a, d_ptr->m_lAccounts) {
         if (a->id() == DRing::Account::ProtocolNames::IP2IP) {
            d_ptr->m_pIP2IP = a;
            connect(a,SIGNAL(enabled()),this,SLOT(slotSupportedProtocolsChanged()));
         }
      }
   }
   return d_ptr->m_pIP2IP;
}

///Singleton
AccountModel* AccountModel::instance()
{
   if (! AccountModelPrivate::m_spAccountList) {
      AccountModelPrivate::m_spAccountList = new AccountModel();
      AccountModelPrivate::m_spAccountList->d_ptr->init();
   }
   return AccountModelPrivate::m_spAccountList;
}

QItemSelectionModel* AccountModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel)
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<AccountModel*>(this));

   return d_ptr->m_pSelectionModel;
}

/**
 * The client have a different point of view when it come to the account
 * state. All the different errors are also handled elsewhere
 */
Account::RegistrationState AccountModelPrivate::fromDaemonName(const QString& st)
{
   if     ( st == DRing::Account::States::REGISTERED
        ||  st == DRing::Account::States::READY                    )
      return Account::RegistrationState::READY;

   else if( st == DRing::Account::States::UNREGISTERED             )
      return Account::RegistrationState::UNREGISTERED;

   else if( st == DRing::Account::States::TRYING                   )
      return Account::RegistrationState::TRYING;

   else if( st == DRing::Account::States::ERROR
        ||  st == DRing::Account::States::ERROR_GENERIC
        ||  st == DRing::Account::States::ERROR_AUTH
        ||  st == DRing::Account::States::ERROR_NETWORK
        ||  st == DRing::Account::States::ERROR_HOST
        ||  st == DRing::Account::States::ERROR_CONF_STUN
        ||  st == DRing::Account::States::ERROR_EXIST_STUN
        ||  st == DRing::Account::States::ERROR_SERVICE_UNAVAILABLE
        ||  st == DRing::Account::States::ERROR_NOT_ACCEPTABLE
        ||  st == DRing::Account::States::REQUEST_TIMEOUT          )
      return Account::RegistrationState::ERROR;

   else {
      qWarning() << "Unknown registration state" << st;
      return Account::RegistrationState::ERROR;
   }

}

///Account status changed
void AccountModelPrivate::slotDaemonAccountChanged(const QString& account, const QString& registration_state, unsigned code, const QString& status)
{
   Account* a = q_ptr->getById(account.toLatin1());

   //TODO move this to AccountStatusModel
   if (!a || (a && a->d_ptr->m_LastSipRegistrationStatus != status )) {
      if (status != "OK") //Do not pollute the log
         qDebug() << "Account" << account << "status changed to" << status;
   }

   if (a)
      a->d_ptr->m_LastSipRegistrationStatus = status;

   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   //The account may have been deleted by the user, but 'apply' have not been pressed
   if (!a) {
      const QStringList accountIds = configurationManager.getAccountList();
      for (int i = 0; i < accountIds.size(); ++i) {
         if ((!q_ptr->getById(accountIds[i].toLatin1())) && m_lDeletedAccounts.indexOf(accountIds[i]) == -1) {
            Account* acc = AccountPrivate::buildExistingAccountFromId(accountIds[i].toLatin1());
            m_lAccounts.insert(i, acc);
            connect(acc,SIGNAL(changed(Account*)),this,SLOT(slotAccountChanged(Account*)));
            connect(acc,SIGNAL(presenceEnabledChanged(bool)),this,SLOT(slotAccountPresenceEnabledChanged(bool)));
            emit q_ptr->dataChanged(q_ptr->index(i,0),q_ptr->index(q_ptr->size()-1));
            emit q_ptr->layoutChanged();

            if (acc->id() != DRing::Account::ProtocolNames::IP2IP)
               enableProtocol(acc->protocol());

         }
      }
      foreach (Account* acc, m_lAccounts) {
         const int idx =accountIds.indexOf(acc->id());
         if (idx == -1 && (acc->editState() == Account::EditState::READY || acc->editState() == Account::EditState::REMOVED)) {
            m_lAccounts.remove(idx);
            emit q_ptr->dataChanged(q_ptr->index(idx - 1, 0), q_ptr->index(m_lAccounts.size()-1, 0));
            emit q_ptr->layoutChanged();
         }
      }
   }
   else {
      const bool isRegistered = a->registrationState() == Account::RegistrationState::READY;
      a->d_ptr->updateState();
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

      //Keep the error message
      a->setLastErrorMessage(status);
      a->setLastErrorCode(code);

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

      a->d_ptr->m_LastTransportCode    = transportCode;
      a->d_ptr->m_LastTransportMessage = transportDesc;

      const Account::RegistrationState state = fromDaemonName(a->d_ptr->accountDetail(DRing::Account::ConfProperties::Registration::STATUS));
      a->d_ptr->m_RegistrationState = state;
   }
}

///Update accounts
void AccountModel::update()
{
   ConfigurationManagerInterface & configurationManager = DBus::ConfigurationManager::instance();
   QList<Account*> tmp;
   for (int i = 0; i < d_ptr->m_lAccounts.size(); i++)
      tmp << d_ptr->m_lAccounts[i];

   for (int i = 0; i < tmp.size(); i++) {
      Account* current = tmp[i];
      if (!current->isNew() && (current->editState() != Account::EditState::NEW
         && current->editState() != Account::EditState::MODIFIED
         && current->editState() != Account::EditState::OUTDATED))
         remove(current);
   }
   //ask for the list of accounts ids to the configurationManager
   const QStringList accountIds = configurationManager.getAccountList();
   for (int i = 0; i < accountIds.size(); ++i) {
      if (d_ptr->m_lDeletedAccounts.indexOf(accountIds[i]) == -1) {
         Account* a = AccountPrivate::buildExistingAccountFromId(accountIds[i].toLatin1());
         d_ptr->m_lAccounts.insert(i, a);
         emit dataChanged(index(i,0),index(size()-1,0));
         connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
         //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));
         connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
         emit layoutChanged();

         if (a->id() != DRing::Account::ProtocolNames::IP2IP)
            d_ptr->enableProtocol(a->protocol());
      }
   }
} //update

///Update accounts
void AccountModel::updateAccounts()
{
   qDebug() << "Updating all accounts";
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   QStringList accountIds = configurationManager.getAccountList();
   //m_lAccounts.clear();
   for (int i = 0; i < accountIds.size(); ++i) {
      Account* acc = getById(accountIds[i].toLatin1());
      if (!acc) {
         Account* a = AccountPrivate::buildExistingAccountFromId(accountIds[i].toLatin1());
         beginInsertRows(QModelIndex(),d_ptr->m_lAccounts.size(),d_ptr->m_lAccounts.size());
         d_ptr->m_lAccounts += a;
         endInsertRows();
         connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
         //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));
         connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
         emit dataChanged(index(size()-1,0),index(size()-1,0));

         if (a->id() != DRing::Account::ProtocolNames::IP2IP)
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
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
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

///Move account up
bool AccountModel::moveUp()
{
   if (d_ptr->m_pSelectionModel) {
      const QModelIndex& idx = d_ptr->m_pSelectionModel->currentIndex();
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
      if (dropMimeData(mimeData({idx}), Qt::MoveAction, idx.row()+1, idx.column(),idx.parent())) {
         return true;
      }
   }
   return false;
}

///Try to register all enabled accounts
void AccountModel::registerAllAccounts()
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   configurationManager.registerAllAccounts();
}

///Cancel all modifications
void AccountModel::cancel() {
   foreach (Account* a, d_ptr->m_lAccounts) {
      if (a->editState() == Account::EditState::MODIFIED || a->editState() == Account::EditState::OUTDATED)
         a->performAction(Account::EditAction::CANCEL);
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
   Q_ASSERT(!id.isEmpty());
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
   Q_UNUSED(parentIdx);
   return d_ptr->m_lAccounts.size();
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
   int count = 0;
   foreach (Account* a, instance()->d_ptr->m_lAccounts) {
      if (a->alias().left(alias.size()) == alias)
         count++;
   }
   bool found = true;
   do {
      found = false;
      foreach (Account* a, instance()->d_ptr->m_lAccounts) {
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

bool AccountModel::isIAXSupported() const
{
   return d_ptr->m_lSupportedProtocols[Account::Protocol::IAX];
}

bool AccountModel::isIP2IPSupported() const
{
   //When this account isn't enable, it is as it wasn't there at all
   return ip2ip()->isEnabled();
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

Account* AccountModel::add(const QString& alias, const Account::Protocol proto)
{
   Account* a = AccountPrivate::buildNewAccountFromAlias(proto,alias);
   connect(a,SIGNAL(changed(Account*)),d_ptr,SLOT(slotAccountChanged(Account*)));
   beginInsertRows(QModelIndex(),d_ptr->m_lAccounts.size(),d_ptr->m_lAccounts.size());
   d_ptr->m_lAccounts += a;
   endInsertRows();
   connect(a,SIGNAL(presenceEnabledChanged(bool)),d_ptr,SLOT(slotAccountPresenceEnabledChanged(bool)));
   //connect(a,SIGNAL(propertyChanged(Account*,QString,QString,QString)),d_ptr,SLOT(slotAccountChanged(Account*)));

   emit dataChanged(index(d_ptr->m_lAccounts.size()-1,0), index(d_ptr->m_lAccounts.size()-1,0));

   if (d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel->setCurrentIndex(index(d_ptr->m_lAccounts.size()-1,0), QItemSelectionModel::ClearAndSelect);
   }

   if (a->id() != DRing::Account::ProtocolNames::IP2IP)
      d_ptr->enableProtocol(proto);

   emit accountAdded(a);

   return a;
}

Account* AccountModel::add(const QString& alias, const QModelIndex& idx)
{
   return add(alias, qvariant_cast<Account::Protocol>(idx.data((int)ProtocolModel::Role::Protocol)));
}

///Remove an account
void AccountModel::remove(Account* account)
{
   if (not account) return;
   qDebug() << "Removing" << account->alias() << account->id();
   const int aindex = d_ptr->m_lAccounts.indexOf(account);
   beginRemoveRows(QModelIndex(),aindex,aindex);
   d_ptr->m_lAccounts.remove(aindex);
   d_ptr->m_lDeletedAccounts << account->id();
   endRemoveRows();
   emit accountRemoved(account);
   //delete account;
   d_ptr->m_pRemovedAccounts << account;
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
   d_ptr->m_lAccounts << acc;
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

      beginInsertRows(QModelIndex(), destinationRow, destinationRow);
      d_ptr->m_lAccounts.insert(destinationRow,acc);
      endInsertRows();

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

#include <accountmodel.moc>

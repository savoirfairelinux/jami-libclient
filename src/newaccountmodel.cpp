/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
 *   Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>         *
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
#include "api/newaccountmodel.h"

// daemon
#include <account_const.h>

//qt
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtCore/QBuffer>

// new LRC
#include "api/lrc.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/peerdiscoverymodel.h"
#include "api/newcallmodel.h"
#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"
#include "api/behaviorcontroller.h"
#include "authority/storagehelper.h"
#include "callbackshandler.h"
#include "database.h"
#include "vcard.h"

// old LRC
#include "api/profile.h"

#include "daemonproxy.h"

#include <atomic>
#include <limits>

namespace lrc
{

using namespace api;

class NewAccountModelPimpl: public QObject
{
    Q_OBJECT
public:
    NewAccountModelPimpl(NewAccountModel& linked,
                         Lrc& lrc,
                         const CallbacksHandler& callbackHandler,
                         const BehaviorController& behaviorController,
                         MigrationCb& willMigrateCb,
                         MigrationCb& didMigrateCb);
    ~NewAccountModelPimpl();

    using AccountInfoDbMap = std::map<std::string,
                                      std::pair<account::Info, std::shared_ptr<Database>>>;

    NewAccountModel& linked;
    Lrc& lrc;
    const CallbacksHandler& callbacksHandler;
    const BehaviorController& behaviorController;
    AccountInfoDbMap accounts;

    // Synchronization tools
    std::mutex m_mutex_account;
    std::mutex m_mutex_account_removal;
    std::condition_variable m_condVar_account_removal;
    std::atomic_bool username_changed;
    std::string new_username;

    /**
     * Add the profile information from an account to the db then add it to accounts.
     * @param accountId
     * @param db an optional migrated database object
     * @note this method get details for an account from the daemon.
     */
    void addToAccounts(const std::string& accountId, std::shared_ptr<Database> db = nullptr);

    /**
     * Remove account from accounts list. Emit accountRemoved.
     * @param accountId
     */
    void removeFromAccounts(const std::string& accountId);

    /**
     * Sync changes to the accounts list with the lrc.
     */
    void updateAccounts();

public Q_SLOTS:

    /**
     * Emit accountStatusChanged.
     * @param accountId
     * @param status
     */
    void slotAccountStatusChanged(const std::string& accountID, const api::account::Status status);

    /**
     * Emit exportOnRingEnded.
     * @param accountId
     * @param status
     * @param pin
     */
    void slotExportOnRingEnded(const std::string& accountID, int status, const std::string& pin);

    /**
     * @param accountId
     * @param details
     */
    void slotAccountDetailsChanged(const std::string& accountID, const std::map<std::string, std::string>& details);

    /**
     * Emit nameRegistrationEnded
     * @param accountId
     * @param status
     * @param name
     */
    void slotNameRegistrationEnded(const std::string& accountId, int status, const std::string& name);

    /**
     * Emit registeredNameFound
     * @param accountId
     * @param status
     * @param address
     * @param name
     */
    void slotRegisteredNameFound(const std::string& accountId, int status, const std::string& address, const std::string& name);

    /**
     * Emit migrationEnded
     * @param accountId
     * @param ok
     */
    void slotMigrationEnded(const std::string& accountId, bool ok);
};

NewAccountModel::NewAccountModel(Lrc& lrc,
                                 const CallbacksHandler& callbacksHandler,
                                 const BehaviorController& behaviorController,
                                 MigrationCb& willMigrateCb,
                                 MigrationCb& didMigrateCb)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(*this, lrc, callbacksHandler, behaviorController,
                                                willMigrateCb, didMigrateCb))
{
}

NewAccountModel::~NewAccountModel()
{
}

std::vector<std::string>
NewAccountModel::getAccountList() const
{
    std::vector<std::string> accountsId;
    const std::vector<std::string> accountIds = DaemonProxy::instance().getAccountList();

    for (auto const& id : accountIds) {
        auto account = pimpl_->accounts.find(id);
        // Do not include accounts flagged for removal
        if (account != pimpl_->accounts.end() && account->second.first.valid)
            accountsId.emplace_back(id);
    }

    return accountsId;
}

void
NewAccountModel::setAccountEnabled(const std::string& accountId, bool enabled) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::getAccountConfig, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    accountInfo.enabled = enabled;
    DaemonProxy::instance().sendRegister(accountId, enabled);
}

void
NewAccountModel::setAccountConfig(const std::string& accountId,
                                  const account::ConfProperties_t& confProperties) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::save, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    std::map<std::string, std::string> details = confProperties.toDetails();
    // Set values from Info. No need to include ID and TYPE. SIP accounts may modify the USERNAME
    // TODO: move these into the ConfProperties_t struct ?
    using namespace DRing::Account;
    qDebug("UPNP_ENABLED: %s\n", details[ConfProperties::UPNP_ENABLED].c_str());
    details[ConfProperties::ENABLED]      = accountInfo.enabled? "true" : "false";
    details[ConfProperties::ALIAS]        = accountInfo.profileInfo.alias;
    details[ConfProperties::DISPLAYNAME]  = accountInfo.profileInfo.alias;
    details[ConfProperties::TYPE]         = (accountInfo.profileInfo.type == profile::Type::RING)? ProtocolNames::RING : ProtocolNames::SIP;
    if (accountInfo.profileInfo.type == profile::Type::RING) {
        details[ConfProperties::USERNAME] = "ring:" + accountInfo.profileInfo.uri;
    } else if (accountInfo.profileInfo.type == profile::Type::SIP) {
        std::map<std::string, std::string> credentials;
        credentials[ConfProperties::USERNAME] = confProperties.username;
        credentials[ConfProperties::PASSWORD] = confProperties.password;
        credentials[ConfProperties::REALM] = confProperties.realm.empty()? "*" : confProperties.realm;
        std::vector<std::map<std::string, std::string>> credentialsVec;
        credentialsVec.push_back(credentials);
        DaemonProxy::instance().setCredentials(accountId, credentialsVec);
        details[ConfProperties::USERNAME] = confProperties.username;
    }
    DaemonProxy::instance().setAccountDetails(accountId, details);
}

account::ConfProperties_t
NewAccountModel::getAccountConfig(const std::string& accountId) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::getAccountConfig, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    return accountInfo.confProperties;
}

void
NewAccountModel::setAlias(const std::string& accountId, const std::string& alias)
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::setAlias, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    accountInfo.profileInfo.alias = alias;

    authority::storage::createOrUpdateProfile(accountInfo.id, accountInfo.profileInfo);

    Q_EMIT profileUpdated(accountId);
}

void
NewAccountModel::setAvatar(const std::string& accountId, const std::string& avatar)
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::setAvatar, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    accountInfo.profileInfo.avatar = avatar;

    authority::storage::createOrUpdateProfile(accountInfo.id, accountInfo.profileInfo);

    Q_EMIT profileUpdated(accountId);
}

bool
NewAccountModel::registerName(const std::string& accountId, const std::string& password, const std::string& username)
{
    return DaemonProxy::instance().registerName(accountId, password, username);
}

bool
NewAccountModel::exportToFile(const std::string& accountId, const std::string& path, const std::string& password) const
{
    return DaemonProxy::instance().exportToFile(accountId, path, password);
}

bool
NewAccountModel::exportOnRing(const std::string& accountId, const std::string& password) const
{
    return DaemonProxy::instance().exportOnRing(accountId, password);
}

void
NewAccountModel::removeAccount(const std::string& accountId) const
{
    DaemonProxy::instance().removeAccount(accountId);
}

bool
NewAccountModel::changeAccountPassword(const std::string& accountId,
                                       const std::string& currentPassword,
                                       const std::string& newPassword) const
{
    return DaemonProxy::instance().changeAccountPassword(accountId, currentPassword, newPassword);
}

void
NewAccountModel::flagFreeable(const std::string& accountId) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::flagFreeable, can't find " + accountId);

    {
        std::lock_guard<std::mutex> lock(pimpl_->m_mutex_account_removal);
        account->second.first.freeable = true;
    }
    pimpl_->m_condVar_account_removal.notify_all();
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::getAccountInfo, can't find " + accountId);

    return accountInfo->second.first;
}

NewAccountModelPimpl::NewAccountModelPimpl(NewAccountModel& linked,
                                           Lrc& lrc,
                                           const CallbacksHandler& callbacksHandler,
                                           const BehaviorController& behaviorController,
                                           MigrationCb& willMigrateCb,
                                           MigrationCb& didMigrateCb)
: linked(linked)
, lrc {lrc}
, behaviorController(behaviorController)
, callbacksHandler(callbacksHandler)
, username_changed(false)
{
    const std::vector<std::string> accountIds = DaemonProxy::instance().getAccountList();
    auto accountDbs = authority::storage::migrateIfNeeded(accountIds, willMigrateCb, didMigrateCb);

    for (const auto& id : accountIds) {
        int index = 0;
        for (const auto& id2: accountIds) {
            if (id2 == id) break;
            index++;
        }
        addToAccounts(id, accountDbs.at(index));
    }

    connect(&callbacksHandler, &CallbacksHandler::accountsChanged, this, &NewAccountModelPimpl::updateAccounts, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::accountStatusChanged, this, &NewAccountModelPimpl::slotAccountStatusChanged, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::accountDetailsChanged, this, &NewAccountModelPimpl::slotAccountDetailsChanged, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::exportOnRingEnded, this, &NewAccountModelPimpl::slotExportOnRingEnded, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::nameRegistrationEnded, this, &NewAccountModelPimpl::slotNameRegistrationEnded, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::registeredNameFound, this, &NewAccountModelPimpl::slotRegisteredNameFound, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::migrationEnded, this, &NewAccountModelPimpl::slotMigrationEnded, Qt::QueuedConnection);
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{
}

void
NewAccountModelPimpl::updateAccounts()
{
    qDebug() << "Syncing lrc accounts list with the daemon";
    std::vector<std::string> accountIds = DaemonProxy::instance().getAccountList();

    // Detect removed accounts
    std::list<std::string> toBeRemoved;
    for (auto& it : accounts) {
        auto& accountInfo = it.second.first;
        bool found = false;
        for (auto& id: accountIds)
            if (id == accountInfo.id) { found = true; break; }

        if (!found) {
            qDebug("detected account removal %s", accountInfo.id.c_str());
            toBeRemoved.push_back(accountInfo.id);
        }
    }

    for (auto it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it) {
        removeFromAccounts(*it);
    }

    // Detect new accounts
    for (auto& id : accountIds) {
        auto account = accounts.find(id);
        // NOTE: If the daemon is down, but dbus answered, id can contain
        // "Remote peer disconnected", "The name is not activable", etc.
        // So avoid to create useless directories.
        if (account == accounts.end() && id.find(' ') == std::string::npos) {
            qDebug("detected new account %s", id.c_str());
            addToAccounts(id);
            auto updatedAccount = accounts.find(id);
            if (updatedAccount == accounts.end()) {
                return;
            }
            if (updatedAccount->second.first.profileInfo.type == profile::Type::SIP) {
                // NOTE: At this point, a SIP account is ready, but not a Ring
                // account. Indeed, the keys are not generated at this point.
                // See slotAccountStatusChanged for more details.
                Q_EMIT linked.accountAdded(id);
            }
        }
    }
}

void
NewAccountModelPimpl::slotAccountStatusChanged(const std::string& accountID, const api::account::Status status)
{
    if (status == api::account::Status::INVALID) {
        Q_EMIT linked.invalidAccountDetected(accountID);
        return;
    }
    auto it = accounts.find(accountID);

    // If account is not in the map yet, don't add it, it is updateAccounts's job
    if (it == accounts.end()) {
        return;
    }

    auto& accountInfo = it->second.first;

    if (accountInfo.profileInfo.type != profile::Type::SIP) {
        if (status != api::account::Status::INITIALIZING
            && accountInfo.status == api::account::Status::INITIALIZING) {
            // Detect when a new account is generated (keys are ready). During
            // the generation, a Ring account got the "INITIALIZING" status.
            // When keys are generated, the status will change.
            accounts.erase(accountID);
            addToAccounts(accountID);
            Q_EMIT linked.accountAdded(accountID);
        } else if (!accountInfo.profileInfo.uri.empty()) {
            accountInfo.status = status;
            Q_EMIT linked.accountStatusChanged(accountID);
        }
    } else {
        accountInfo.status = status;
        Q_EMIT linked.accountStatusChanged(accountID);
    }
}

void
NewAccountModelPimpl::slotAccountDetailsChanged(const std::string& accountId, const std::map<std::string, std::string>& details)
{
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        throw std::out_of_range("NewAccountModelPimpl::slotAccountDetailsChanged, can't find " + accountId);
    }
    auto& accountInfo = account->second.first;
    accountInfo.fromDetails(details);
    if (username_changed) {
        username_changed = false;
        accountInfo.registeredName = new_username;
        Q_EMIT linked.profileUpdated(accountId);
    }
    Q_EMIT linked.accountStatusChanged(accountId);
}

void
NewAccountModelPimpl::slotExportOnRingEnded(const std::string& accountID, int status, const std::string& pin)
{
    account::ExportOnRingStatus convertedStatus = account::ExportOnRingStatus::INVALID;
    switch (status) {
    case 0:
        convertedStatus = account::ExportOnRingStatus::SUCCESS;
        break;
    case 1:
        convertedStatus = account::ExportOnRingStatus::WRONG_PASSWORD;
        break;
    case 2:
        convertedStatus = account::ExportOnRingStatus::NETWORK_ERROR;
        break;
    default:
        break;
    }
    Q_EMIT linked.exportOnRingEnded(accountID, convertedStatus, pin);
}

void
NewAccountModelPimpl::slotNameRegistrationEnded(const std::string& accountId, int status, const std::string& name)
{
    account::RegisterNameStatus convertedStatus = account::RegisterNameStatus::INVALID;
    switch (status)
    {
    case 0: {
        convertedStatus = account::RegisterNameStatus::SUCCESS;
        auto account = accounts.find(accountId);
        if (account != accounts.end() && account->second.first.registeredName.empty()) {
            auto conf = linked.getAccountConfig(accountId);
            username_changed = true;
            new_username = name;
            linked.setAccountConfig(accountId, conf);
        }
        break;
      }
    case 1:
        convertedStatus = account::RegisterNameStatus::WRONG_PASSWORD;
        break;
    case 2:
        convertedStatus = account::RegisterNameStatus::INVALID_NAME;
        break;
    case 3:
        convertedStatus = account::RegisterNameStatus::ALREADY_TAKEN;
        break;
    case 4:
        convertedStatus = account::RegisterNameStatus::NETWORK_ERROR;
        break;
    default:
        break;
    }
    Q_EMIT linked.nameRegistrationEnded(accountId, convertedStatus, name);
}

void
NewAccountModelPimpl::slotRegisteredNameFound(const std::string& accountId, int status, const std::string& address, const std::string& name)
{
    account::LookupStatus convertedStatus = account::LookupStatus::INVALID;
    switch (status)
    {
    case 0:
        convertedStatus = account::LookupStatus::SUCCESS;
        break;
    case 1:
        convertedStatus = account::LookupStatus::INVALID_NAME;
        break;
    case 2:
        convertedStatus = account::LookupStatus::NOT_FOUND;
        break;
    case 3:
        convertedStatus = account::LookupStatus::ERROR;
        break;
    default:
        break;
    }
    Q_EMIT linked.registeredNameFound(accountId, convertedStatus, address, name);
}

void
NewAccountModelPimpl::slotMigrationEnded(const std::string& accountId, bool ok)
{
    if (ok) {
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            addToAccounts(accountId);
            return;
        }
        auto& accountInfo = it->second.first;
        std::map<std::string, std::string> details = DaemonProxy::instance().getAccountDetails(accountId);
        accountInfo.fromDetails(details);
        std::map<std::string, std::string> volatileDetails = DaemonProxy::instance().getVolatileAccountDetails(accountId);
        std::string daemonStatus = volatileDetails[DRing::Account::ConfProperties::Registration::STATUS];
        accountInfo.status = lrc::api::account::to_status(daemonStatus);
    }
    Q_EMIT linked.migrationEnded(accountId, ok);
}

void
NewAccountModelPimpl::addToAccounts(const std::string& accountId,
                                    std::shared_ptr<Database> db)
{
    if (db == nullptr) {
        try {
            auto appPath = authority::storage::getPath();
            auto dbName = QString::fromStdString(accountId + "/history");
            db = DatabaseFactory::create<Database>(dbName, appPath);
            // create the profiles path if necessary
            QDir profilesDir(appPath + QString::fromStdString(accountId) + "/profiles");
            if (!profilesDir.exists()) {
                profilesDir.mkpath(".");
            }
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
            return;
        }
    }

    auto it = accounts.emplace(accountId, std::make_pair(account::Info(), db));

    if (!it.second) {
        qWarning("failed to add new account: id already present in map");
        return;
    }

    // Init profile
    account::Info& newAccInfo = (it.first)->second.first;
    newAccInfo.id = accountId;
    newAccInfo.profileInfo.avatar = authority::storage::getAccountAvatar(accountId);

    // Fill account::Info struct with details from daemon
    std::map<std::string, std::string> details = DaemonProxy::instance().getAccountDetails(accountId);
    newAccInfo.fromDetails(details);

    // Init models for this account
    newAccInfo.accountModel = &linked;
    newAccInfo.callModel = std::make_unique<NewCallModel>(newAccInfo, callbacksHandler);
    newAccInfo.contactModel = std::make_unique<ContactModel>(newAccInfo, *db, callbacksHandler, behaviorController);
    newAccInfo.conversationModel = std::make_unique<ConversationModel>(newAccInfo, lrc, *db, callbacksHandler, behaviorController);
    newAccInfo.peerDiscoveryModel = std::make_unique<PeerDiscoveryModel>(callbacksHandler, accountId);
    newAccInfo.deviceModel = std::make_unique<NewDeviceModel>(newAccInfo, callbacksHandler);
    newAccInfo.codecModel = std::make_unique<NewCodecModel>(newAccInfo, callbacksHandler);

    std::map<std::string, std::string> volatileDetails = DaemonProxy::instance().getVolatileAccountDetails(accountId);
    std::string daemonStatus = volatileDetails[DRing::Account::ConfProperties::Registration::STATUS];
    newAccInfo.status = lrc::api::account::to_status(daemonStatus);
}

void
NewAccountModelPimpl::removeFromAccounts(const std::string& accountId)
{
    /* Update db before waiting for the client to stop using the structs is fine
       as long as we don't free anything */
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        return;
    }
    auto& accountInfo = account->second.first;
    /* Inform client about account removal. Do *not* free account structures
       before we are sure that the client stopped using it, otherwise we might
       get into use-after-free troubles. */
    accountInfo.valid = false;
    Q_EMIT linked.accountRemoved(accountId);

#ifdef CHK_FREEABLE_BEFORE_ERASE_ACCOUNT
    std::unique_lock<std::mutex> lock(m_mutex_account_removal);
    // Wait for client to stop using old account structs
    m_condVar_account_removal.wait(lock, [&](){return accounts[accountId].freeable;});
    lock.unlock();
#endif

    // Now we can free them
    accounts.erase(accountId);
}

static inline bool
to_bool(std::string s) noexcept
{
    return s == "true" ? true : false;
}

static inline std::string
to_string(bool b) noexcept
{
    return b ? "true" : "false";
}

void
account::Info::fromDetails(const std::map<std::string, std::string>& details)
{
    using namespace DRing::Account;
    const std::map<std::string, std::string> volatileDetails = DaemonProxy::instance().getVolatileAccountDetails(id);

    // General
    try {
        profileInfo.type                                    = details.at(ConfProperties::TYPE) == ProtocolNames::RING ? profile::Type::RING : profile::Type::SIP;
    } catch (const std::out_of_range& e) {
        profileInfo.type                                    = profile::Type::INVALID;
    }

    try {
        registeredName                                      = profileInfo.type == profile::Type::RING ? volatileDetails.at(VolatileProperties::REGISTERED_NAME) : "";
    } catch (const std::out_of_range& e) {
        registeredName                                      = "";
    }

    try {
        profileInfo.alias                                   = details.at(ConfProperties::DISPLAYNAME);
    } catch (const std::out_of_range& e) {
        profileInfo.alias                                   = "";
    }

    try {
        profileInfo.uri                                     = (profileInfo.type == profile::Type::RING && details.at(ConfProperties::USERNAME).compare(0,5,"ring:") == 0)
                                                              ? details.at(ConfProperties::USERNAME).substr(std::string("ring:").size())
                                                              : details.at(ConfProperties::USERNAME);
    } catch (const std::out_of_range& e) {
        profileInfo.uri                                     = "";
    }

    try {
        enabled                                             = to_bool(details.at(ConfProperties::ENABLED));
    } catch (const std::out_of_range& e) {
        enabled                                             = false;
    }

    try {
        confProperties.mailbox                              = details.at(ConfProperties::MAILBOX);
    } catch (const std::out_of_range& e) {
        confProperties.mailbox                              = "";
    }

    try {
        confProperties.dtmfType                             = details.at(ConfProperties::DTMF_TYPE);
    } catch (const std::out_of_range& e) {
        confProperties.dtmfType                             = "";
    }

    try {
        confProperties.autoAnswer                           = to_bool(details.at(ConfProperties::AUTOANSWER));
    } catch (const std::out_of_range& e) {
        confProperties.autoAnswer                           = false;
    }

    try {
        confProperties.activeCallLimit                      = std::stoi(details.at(ConfProperties::ACTIVE_CALL_LIMIT));
    } catch (const std::out_of_range& e) {
        confProperties.activeCallLimit                      = std::numeric_limits<int>::max();
    } catch (const std::invalid_argument& e) {
        confProperties.activeCallLimit                      = std::numeric_limits<int>::max();
    }

    try {
        confProperties.hostname                             = details.at(ConfProperties::HOSTNAME);
    } catch (const std::out_of_range& e) {
        confProperties.hostname                             = "";
    }

    try {
        confProperties.username                             = details.at(ConfProperties::USERNAME);
    } catch (const std::out_of_range& e) {
        confProperties.username                             = "";
    }

    try {
        confProperties.routeset                             = details.at(ConfProperties::ROUTE);
    } catch (const std::out_of_range& e) {
        confProperties.routeset                             = "";
    }

    try {
        confProperties.password                             = details.at(ConfProperties::PASSWORD);
    } catch (const std::out_of_range& e) {
        confProperties.password                             = "";
    }

    try {
        confProperties.realm                                = details.at(ConfProperties::REALM);
    } catch (const std::out_of_range& e) {
        confProperties.realm                                = "";
    }

    try {
        confProperties.localInterface                       = details.at(ConfProperties::LOCAL_INTERFACE);
    } catch (const std::out_of_range& e) {
        confProperties.localInterface                       = "";
    }

    try {
        confProperties.deviceId                             = details.at(ConfProperties::RING_DEVICE_ID);
    } catch (const std::out_of_range& e) {
        confProperties.deviceId                             = "";
    }

    try {
        confProperties.deviceName                           = details.at(ConfProperties::RING_DEVICE_NAME);
    } catch (const std::out_of_range& e) {
        confProperties.deviceName                           = "";
    }

    try {
        confProperties.publishedSameAsLocal                 = to_bool(details.at(ConfProperties::PUBLISHED_SAMEAS_LOCAL));
    } catch (const std::out_of_range& e) {
        confProperties.publishedSameAsLocal                 = true;
    }

    try {
        confProperties.localPort                            = std::stoi(details.at(ConfProperties::LOCAL_PORT));
    } catch (const std::out_of_range& e) {
        confProperties.localPort                            = 1024;
    } catch (const std::invalid_argument& e) {
        confProperties.localPort                            = 1024;
    }

    try {
        confProperties.publishedPort                        = std::stoi(details.at(ConfProperties::PUBLISHED_PORT));
    } catch (const std::out_of_range& e) {
        confProperties.publishedPort                        = 1024;
    } catch (const std::invalid_argument& e) {
        confProperties.publishedPort                        = 1024;
    }

    try {
        confProperties.publishedAddress                     = details.at(ConfProperties::PUBLISHED_ADDRESS);
    } catch (const std::out_of_range& e) {
        confProperties.publishedAddress                     = "";
    }

    try {
        confProperties.userAgent                            = details.at(ConfProperties::USER_AGENT);
    } catch (const std::out_of_range& e) {
        confProperties.userAgent                            = "";
    }

    try {
        confProperties.upnpEnabled                          = to_bool(details.at(ConfProperties::UPNP_ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.upnpEnabled                          = true;
    }

    try {
        confProperties.hasCustomUserAgent                   = to_bool(details.at(ConfProperties::HAS_CUSTOM_USER_AGENT));
    } catch (const std::out_of_range& e) {
        confProperties.hasCustomUserAgent                   = false;
    }

    try {
        confProperties.allowIncoming                        = to_bool(details.at(ConfProperties::ALLOW_CERT_FROM_HISTORY))
                                                            | to_bool(details.at(ConfProperties::ALLOW_CERT_FROM_CONTACT))
                                                            | to_bool(details.at(ConfProperties::ALLOW_CERT_FROM_TRUSTED));
    } catch (const std::out_of_range& e) {
        confProperties.allowIncoming                        = true;
    }

    try {
        confProperties.archivePassword                      = details.at(ConfProperties::ARCHIVE_PASSWORD);
    } catch (const std::out_of_range& e) {
        confProperties.archivePassword                      = "";
    }

    try {
        confProperties.archiveHasPassword                   = to_bool(details.at(ConfProperties::ARCHIVE_HAS_PASSWORD));
    } catch (const std::out_of_range& e) {
        confProperties.archiveHasPassword                   = false;
    }

    try {
        confProperties.archivePath                          = details.at(ConfProperties::ARCHIVE_PATH);
    } catch (const std::out_of_range& e) {
        confProperties.archivePath                          = "";
    }

    try {
        confProperties.archivePin                           = details.at(ConfProperties::ARCHIVE_PIN);
    } catch (const std::out_of_range& e) {
        confProperties.archivePin                           = "";
    }

    try {
        confProperties.proxyEnabled                         = to_bool(details.at(ConfProperties::PROXY_ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.proxyEnabled                         = false;
    }

    try {
        confProperties.proxyServer                          = details.at(ConfProperties::PROXY_SERVER);
    } catch (const std::out_of_range& e) {
        confProperties.proxyServer                          = "";
    }

    try {
        confProperties.proxyPushToken                       = details.at(ConfProperties::PROXY_PUSH_TOKEN);
    } catch (const std::out_of_range& e) {
        confProperties.proxyPushToken                       = "";
    }

    try {
        confProperties.peerDiscovery                        = to_bool(details.at(ConfProperties::DHT_PEER_DISCOVERY));
    } catch (const std::out_of_range& e) {
        confProperties.peerDiscovery                        = true;
    }

    try {
        confProperties.accountDiscovery                     = to_bool(details.at(ConfProperties::ACCOUNT_PEER_DISCOVERY));
    } catch (const std::out_of_range& e) {
        confProperties.accountDiscovery                     = true;
    }

    try {
        confProperties.accountPublish                       = to_bool(details.at(ConfProperties::ACCOUNT_PUBLISH));
    } catch (const std::out_of_range& e) {
        confProperties.accountPublish                       = true;
    }

    // Audio
    try {
        confProperties.Audio.audioPortMax                   = std::stoi(details.at(ConfProperties::Audio::PORT_MAX));
    } catch (const std::out_of_range& e) {
        confProperties.Audio.audioPortMax                   = 65535;
    } catch (const std::invalid_argument& e) {
        confProperties.Audio.audioPortMax                   = 65535;
    }

    try {
        confProperties.Audio.audioPortMin                   = std::stoi(details.at(ConfProperties::Audio::PORT_MIN));
    } catch (const std::out_of_range& e) {
        confProperties.Audio.audioPortMin                   = 1024;
    } catch (const std::invalid_argument& e) {
        confProperties.Audio.audioPortMin                   = 1024;
    }

    // Video
    try {
        confProperties.Video.videoEnabled                   = to_bool(details.at(ConfProperties::Video::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.Video.videoEnabled                   = true;
    }

    try {
        confProperties.Video.videoPortMax                   = std::stoi(details.at(ConfProperties::Video::PORT_MAX));
    } catch (const std::out_of_range& e) {
        confProperties.Video.videoPortMax                   = 65535;
    } catch (const std::invalid_argument& e) {
        confProperties.Video.videoPortMax                   = 65535;
    }

    try {
        confProperties.Video.videoPortMin                   = std::stoi(details.at(ConfProperties::Video::PORT_MIN));
    } catch (const std::out_of_range& e) {
        confProperties.Video.videoPortMin                   = 1024;
    } catch (const std::invalid_argument& e) {
        confProperties.Video.videoPortMin                   = 1024;
    }

    // STUN
    try {
        confProperties.STUN.enable                          = to_bool(details.at(ConfProperties::STUN::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.STUN.enable                          = true;
    }

    try {
        confProperties.STUN.server                          = details.at(ConfProperties::STUN::SERVER);
    } catch (const std::out_of_range& e) {
        confProperties.STUN.server                          = "";
    }

    // TURN
    try {
        confProperties.TURN.enable                          = to_bool(details.at(ConfProperties::TURN::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.TURN.enable                          = true;
    }

    try {
        confProperties.TURN.server                          = details.at(ConfProperties::TURN::SERVER);
    } catch (const std::out_of_range& e) {
        confProperties.TURN.server                          = "";
    }

    try {
        confProperties.TURN.username                        = details.at(ConfProperties::TURN::SERVER_UNAME);
    } catch (const std::out_of_range& e) {
        confProperties.TURN.username                        = "";
    }

    try {
        confProperties.TURN.password                        = details.at(ConfProperties::TURN::SERVER_PWD);
    } catch (const std::out_of_range& e) {
        confProperties.TURN.password                        = "";
    }

    try {
        confProperties.TURN.realm                           = details.at(ConfProperties::TURN::SERVER_REALM);
    } catch (const std::out_of_range& e) {
        confProperties.TURN.realm                           = "";
    }

    // Presence
    try {
        confProperties.Presence.presenceEnabled             = to_bool(details.at(ConfProperties::Presence::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.Presence.presenceEnabled             = true;
    }

    try {
        confProperties.Presence.presencePublishSupported    = to_bool(details.at(ConfProperties::Presence::SUPPORT_PUBLISH));
    } catch (const std::out_of_range& e) {
        confProperties.Presence.presencePublishSupported    = true;
    }

    try {
        confProperties.Presence.presenceSubscribeSupported  = to_bool(details.at(ConfProperties::Presence::SUPPORT_SUBSCRIBE));
    } catch (const std::out_of_range& e) {
        confProperties.Presence.presenceSubscribeSupported  = true;
    }

    // Ringtone
    try {
        confProperties.Ringtone.ringtonePath                = details.at(ConfProperties::Ringtone::PATH);
    } catch (const std::out_of_range& e) {
        confProperties.Ringtone.ringtonePath                = "";
    }

    try {
        confProperties.Ringtone.ringtoneEnabled             = to_bool(details.at(ConfProperties::Ringtone::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.Ringtone.ringtoneEnabled             = true;
    }

    // SRTP
    try {
        confProperties.SRTP.keyExchange                     = details.at(ConfProperties::SRTP::KEY_EXCHANGE).empty()? account::KeyExchangeProtocol::NONE : account::KeyExchangeProtocol::SDES;
    } catch (const std::out_of_range& e) {
        confProperties.SRTP.keyExchange                     = account::KeyExchangeProtocol::SDES;
    }

    try {
        confProperties.SRTP.enable                          = to_bool(details.at(ConfProperties::SRTP::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.SRTP.enable                          = true;
    }

    try {
        confProperties.SRTP.rtpFallback                     = to_bool(details.at(ConfProperties::SRTP::RTP_FALLBACK));
    } catch (const std::out_of_range& e) {
        confProperties.SRTP.rtpFallback                     = false;
    }

    // TLS
    try {
        confProperties.TLS.listenerPort                     = std::stoi(details.at(ConfProperties::TLS::LISTENER_PORT));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.listenerPort                     = 22500;
    } catch (const std::invalid_argument& e) {
        confProperties.TLS.listenerPort                     = 22500;
    }

    try {
        confProperties.TLS.enable                           = details.at(ConfProperties::TYPE) == ProtocolNames::RING? true : to_bool(details.at(ConfProperties::TLS::ENABLED));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.enable                           = true;
    }

    try {
        confProperties.TLS.port                             = std::stoi(details.at(ConfProperties::TLS::PORT));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.port                             = 22501;
    } catch (const std::invalid_argument& e) {
        confProperties.TLS.port                             = 22501;
    }

    try {
        confProperties.TLS.certificateListFile              = details.at(ConfProperties::TLS::CA_LIST_FILE);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.certificateListFile              = "";
    }

    try {
        confProperties.TLS.certificateFile                  = details.at(ConfProperties::TLS::CERTIFICATE_FILE);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.certificateFile                  = "";
    }

    try {
        confProperties.TLS.privateKeyFile                   = details.at(ConfProperties::TLS::PRIVATE_KEY_FILE);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.privateKeyFile                   = "";
    }

    try {
        confProperties.TLS.password                         = details.at(ConfProperties::TLS::PASSWORD);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.password                         = "";
    }

    try {
        auto method = details.at(ConfProperties::TLS::METHOD);
        if (method == "TLSv1") {
            confProperties.TLS.method                       = account::TlsMethod::TLSv1;
        } else if (method == "TLSv1.1") {
            confProperties.TLS.method                       = account::TlsMethod::TLSv1_1;
        } else if (method == "TLSv1.2") {
            confProperties.TLS.method                       = account::TlsMethod::TLSv1_2;
        } else {
            confProperties.TLS.method                       = account::TlsMethod::DEFAULT;
        }
    } catch (const std::out_of_range& e) {
            confProperties.TLS.method                       = account::TlsMethod::DEFAULT;
    }

    try {
        confProperties.TLS.ciphers                          = details.at(ConfProperties::TLS::CIPHERS);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.ciphers                          = "";
    }

    try {
        confProperties.TLS.serverName                       = details.at(ConfProperties::TLS::SERVER_NAME);
    } catch (const std::out_of_range& e) {
        confProperties.TLS.serverName                       = "";
    }

    try {
        confProperties.TLS.verifyServer                     = to_bool(details.at(ConfProperties::TLS::VERIFY_SERVER));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.verifyServer                     = true;
    }

    try {
        confProperties.TLS.verifyClient                     = to_bool(details.at(ConfProperties::TLS::VERIFY_CLIENT));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.verifyClient                     = true;
    }

    try {
        confProperties.TLS.requireClientCertificate         = to_bool(details.at(ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.requireClientCertificate         = true;
    }

    try {
        confProperties.TLS.negotiationTimeoutSec            = std::stoi(details.at(ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC));
    } catch (const std::out_of_range& e) {
        confProperties.TLS.negotiationTimeoutSec            = 10;
    } catch (const std::invalid_argument& e) {
        confProperties.TLS.negotiationTimeoutSec            = 10;
    }

    // DHT
    try {
        confProperties.DHT.port                             = std::stoi(details.at(ConfProperties::DHT::PORT));
    } catch (const std::out_of_range& e) {
        confProperties.DHT.port                             = 4222;
    } catch (const std::invalid_argument& e) {
        confProperties.DHT.port                             = 4222;
    }

    try {
        confProperties.DHT.PublicInCalls                    = to_bool(details.at(ConfProperties::DHT::PUBLIC_IN_CALLS));
    } catch (const std::out_of_range& e) {
        confProperties.DHT.PublicInCalls                    = false;
    }

    try {
        confProperties.DHT.AllowFromTrusted                 = to_bool(details.at(ConfProperties::DHT::ALLOW_FROM_TRUSTED));
    } catch (const std::out_of_range& e) {
        confProperties.DHT.AllowFromTrusted                 = true;
    }

    // RingNS
    try {
        confProperties.RingNS.uri                           = details.at(ConfProperties::RingNS::URI);
    } catch (const std::out_of_range& e) {
        confProperties.RingNS.uri                           = "";
    }

    try {
        confProperties.RingNS.account                       = details.at(ConfProperties::RingNS::ACCOUNT);
    } catch (const std::out_of_range& e) {
        confProperties.RingNS.account                       = "";
    }

    // Registration
    try {
        confProperties.Registration.expire                  = std::stoi(details.at(ConfProperties::Registration::EXPIRE));
    } catch (const std::out_of_range& e) {
        confProperties.Registration.expire                  = 0;
    } catch (const std::invalid_argument& e) {
        confProperties.Registration.expire                  = 0;
    }

    // Jams
    try {
        confProperties.managerUri                           = details.at(ConfProperties::MANAGER_URI);
    } catch (const std::out_of_range& e) {
        confProperties.managerUri                           = "";
    }

    try {
        confProperties.managerUsername                      = details.at(ConfProperties::MANAGER_USERNAME);
    } catch (const std::out_of_range& e) {
        confProperties.managerUsername                      = "";
    }
}

std::map<std::string, std::string>
account::ConfProperties_t::toDetails() const
{
    using namespace DRing::Account;
    std::map<std::string, std::string> details;
    // General
    details[ConfProperties::MAILBOX]                    = this->mailbox;
    details[ConfProperties::DTMF_TYPE]                  = this->dtmfType;
    details[ConfProperties::AUTOANSWER]                 = std::to_string(this->autoAnswer);
    details[ConfProperties::ACTIVE_CALL_LIMIT]          = std::to_string(this->activeCallLimit);
    details[ConfProperties::HOSTNAME]                   = this->hostname;
    details[ConfProperties::ROUTE]                      = this->routeset;
    details[ConfProperties::PASSWORD]                   = this->password;
    details[ConfProperties::REALM]                      = this->realm;
    details[ConfProperties::RING_DEVICE_ID]             = this->deviceId;
    details[ConfProperties::RING_DEVICE_NAME]           = this->deviceName;
    details[ConfProperties::LOCAL_INTERFACE]            = this->localInterface;
    details[ConfProperties::PUBLISHED_SAMEAS_LOCAL]     = std::to_string(this->publishedSameAsLocal);
    details[ConfProperties::LOCAL_PORT]                 = std::to_string(this->localPort);
    details[ConfProperties::PUBLISHED_PORT]             = std::to_string(this->publishedPort);
    details[ConfProperties::PUBLISHED_ADDRESS]          = this->publishedAddress;
    details[ConfProperties::USER_AGENT]                 = this->userAgent;
    details[ConfProperties::UPNP_ENABLED]               = std::to_string(this->upnpEnabled);
    details[ConfProperties::HAS_CUSTOM_USER_AGENT]      = std::to_string(this->hasCustomUserAgent);
    details[ConfProperties::ALLOW_CERT_FROM_HISTORY]    = std::to_string(this->allowIncoming);
    details[ConfProperties::ALLOW_CERT_FROM_CONTACT]    = std::to_string(this->allowIncoming);
    details[ConfProperties::ALLOW_CERT_FROM_TRUSTED]    = std::to_string(this->allowIncoming);
    details[ConfProperties::ARCHIVE_PASSWORD]           = this->archivePassword;
    details[ConfProperties::ARCHIVE_HAS_PASSWORD]       = std::to_string(this->archiveHasPassword);
    details[ConfProperties::ARCHIVE_PATH]               = this->archivePath;
    details[ConfProperties::ARCHIVE_PIN]                = this->archivePin;
    // ConfProperties::DEVICE_NAME name is set with NewDeviceModel interface
    details[ConfProperties::PROXY_ENABLED]              = std::to_string(this->proxyEnabled);
    details[ConfProperties::PROXY_SERVER]               = this->proxyServer;
    details[ConfProperties::PROXY_PUSH_TOKEN]           = this->proxyPushToken;
    details[ConfProperties::DHT_PEER_DISCOVERY]         = std::to_string(this->peerDiscovery);
    details[ConfProperties::ACCOUNT_PEER_DISCOVERY]     = std::to_string(this->accountDiscovery);
    details[ConfProperties::ACCOUNT_PUBLISH]            = std::to_string(this->accountPublish);
    // Audio
    details[ConfProperties::Audio::PORT_MAX]            = std::to_string(this->Audio.audioPortMax);
    details[ConfProperties::Audio::PORT_MIN]            = std::to_string(this->Audio.audioPortMin);
    // Video
    details[ConfProperties::Video::ENABLED]             = std::to_string(this->Video.videoEnabled);
    details[ConfProperties::Video::PORT_MAX]            = std::to_string(this->Video.videoPortMax);
    details[ConfProperties::Video::PORT_MIN]            = std::to_string(this->Video.videoPortMin);
    // STUN
    details[ConfProperties::STUN::SERVER]               = this->STUN.server;
    details[ConfProperties::STUN::ENABLED]              = std::to_string(this->STUN.enable);
    // TURN
    details[ConfProperties::TURN::SERVER]               = this->TURN.server;
    details[ConfProperties::TURN::ENABLED]              = std::to_string(this->TURN.enable);
    details[ConfProperties::TURN::SERVER_UNAME]         = this->TURN.username;
    details[ConfProperties::TURN::SERVER_PWD]           = this->TURN.password;
    details[ConfProperties::TURN::SERVER_REALM]         = this->TURN.realm;
    // Presence
    details[ConfProperties::Presence::SUPPORT_PUBLISH]  = std::to_string(this->Presence.presencePublishSupported);
    details[ConfProperties::Presence::SUPPORT_SUBSCRIBE] = std::to_string(this->Presence.presenceSubscribeSupported);
    details[ConfProperties::Presence::ENABLED]          = std::to_string(this->Presence.presenceEnabled);
    // Ringtone
    details[ConfProperties::Ringtone::PATH]             = this->Ringtone.ringtonePath;
    details[ConfProperties::Ringtone::ENABLED]          = std::to_string(this->Ringtone.ringtoneEnabled);
    // SRTP
    details[ConfProperties::SRTP::KEY_EXCHANGE]         = this->SRTP.keyExchange == account::KeyExchangeProtocol::NONE? "" : "sdes";
    details[ConfProperties::SRTP::ENABLED]              = std::to_string(this->SRTP.enable);
    details[ConfProperties::SRTP::RTP_FALLBACK]         = std::to_string(this->SRTP.rtpFallback);
    // TLS
    details[ConfProperties::TLS::LISTENER_PORT]         = std::to_string(this->TLS.listenerPort);
    details[ConfProperties::TLS::ENABLED]               = std::to_string(this->TLS.enable);
    details[ConfProperties::TLS::PORT]                  = std::to_string(this->TLS.port);
    details[ConfProperties::TLS::CA_LIST_FILE]          = this->TLS.certificateListFile;
    details[ConfProperties::TLS::CERTIFICATE_FILE]      = this->TLS.certificateFile;
    details[ConfProperties::TLS::PRIVATE_KEY_FILE]      = this->TLS.privateKeyFile;
    details[ConfProperties::TLS::PASSWORD]              = this->TLS.password;
    switch (this->TLS.method) {
    case account::TlsMethod::TLSv1:
        details[ConfProperties::TLS::METHOD]            = "TLSv1";
        break;
    case account::TlsMethod::TLSv1_1:
        details[ConfProperties::TLS::METHOD]            = "TLSv1.1";
        break;
    case account::TlsMethod::TLSv1_2:
        details[ConfProperties::TLS::METHOD]            = "TLSv1.2";
        break;
    case account::TlsMethod::DEFAULT:
    default:
        details[ConfProperties::TLS::METHOD]            = "Default";
        break;
    }
    details[ConfProperties::TLS::CIPHERS]               = this->TLS.ciphers;
    details[ConfProperties::TLS::SERVER_NAME]           = this->TLS.serverName;
    details[ConfProperties::TLS::VERIFY_SERVER]         = std::to_string(this->TLS.verifyServer);
    details[ConfProperties::TLS::VERIFY_CLIENT]         = std::to_string(this->TLS.verifyClient);
    details[ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE] = std::to_string(this->TLS.requireClientCertificate);
    details[ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC] = std::to_string(this->TLS.negotiationTimeoutSec);
    // DHT
    details[ConfProperties::DHT::PORT]                  = std::to_string(this->DHT.port);
    details[ConfProperties::DHT::PUBLIC_IN_CALLS]       = std::to_string(this->DHT.PublicInCalls);
    details[ConfProperties::DHT::ALLOW_FROM_TRUSTED]    = std::to_string(this->DHT.AllowFromTrusted);
    // RingNS
    details[ConfProperties::RingNS::URI]                = this->RingNS.uri;
    details[ConfProperties::RingNS::ACCOUNT]            = this->RingNS.account;
    // Registration
    details[ConfProperties::Registration::EXPIRE]       = std::to_string(this->Registration.expire);
    // Manager
    details[ConfProperties::MANAGER_URI]                = this->managerUri;
    details[ConfProperties::MANAGER_USERNAME]           = this->managerUsername;

    return details;
}

std::string
NewAccountModel::createNewAccount(profile::Type type,
                                  const std::string& displayName,
                                  const std::string& archivePath,
                                  const std::string& password,
                                  const std::string& pin,
                                  const std::string& uri)
{

    std::map<std::string, std::string> details = type == profile::Type::SIP?
                              DaemonProxy::instance().getAccountTemplate("SIP") :
                              DaemonProxy::instance().getAccountTemplate("RING");
    using namespace DRing::Account;
    details[ConfProperties::TYPE] = type == profile::Type::SIP? "SIP" : "RING";
    details[ConfProperties::DISPLAYNAME] = displayName;
    details[ConfProperties::ALIAS] = displayName;
    details[ConfProperties::UPNP_ENABLED] = "true";
    details[ConfProperties::ARCHIVE_PASSWORD] = password;
    details[ConfProperties::ARCHIVE_PIN] = pin;
    details[ConfProperties::ARCHIVE_PATH] = archivePath;
    if (type == profile::Type::SIP) {
        details[ConfProperties::USERNAME] = uri;
    }

    return DaemonProxy::instance().addAccount(details);
}



std::string
NewAccountModel::connectToAccountManager(const std::string& username,
                                         const std::string& password,
                                         const std::string& serverUri)
{
    std::map<std::string, std::string> details = DaemonProxy::instance().getAccountTemplate("RING");
    using namespace DRing::Account;
    details[ConfProperties::TYPE] = "RING";
    details[ConfProperties::MANAGER_URI] = serverUri.c_str();
    details[ConfProperties::MANAGER_USERNAME] = username.c_str();
    details[ConfProperties::ARCHIVE_PASSWORD] = password.c_str();

    return DaemonProxy::instance().addAccount(details);
}

void
NewAccountModel::setTopAccount(const std::string& accountId)
{
    bool found = false;
    std::string order = {};

    const std::vector<std::string> accountIds = DaemonProxy::instance().getAccountList();
    for (auto& id : accountIds)
    {
        if (id == accountId) {
            found = true;
        } else {
            order += id + "/";
        }
    }
    if (found) {
        order = accountId + "/" + order;
    }
    DaemonProxy::instance().setAccountsOrder(order);
}

std::string
NewAccountModel::accountVCard(const std::string& accountId, bool compressImage) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        return {};
    }
    auto& accountInfo = account->second.first;
    return authority::storage::vcard::profileToVcard(accountInfo.profileInfo, compressImage);
}

} // namespace lrc

#include "newaccountmodel.moc"

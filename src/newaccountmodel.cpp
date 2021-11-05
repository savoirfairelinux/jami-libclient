/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
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

// new LRC
#include "api/lrc.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/peerdiscoverymodel.h"
#include "api/newcallmodel.h"
#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"
#include "api/behaviorcontroller.h"
#include "api/datatransfermodel.h"
#include "authority/storagehelper.h"
#include "callbackshandler.h"
#include "database.h"
#include "vcard.h"

// old LRC
#include "api/profile.h"
#include "qtwrapper/conversions_wrap.hpp"

// Dbus
#include "dbus/configurationmanager.h"

// daemon
#include <account_const.h>

// qt
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtCore/QBuffer>

#include <atomic>

namespace lrc {

using namespace api;

class NewAccountModelPimpl : public QObject
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

    using AccountInfoDbMap = std::map<QString, std::pair<account::Info, std::shared_ptr<Database>>>;

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
    QString new_username;

    /**
     * Add the profile information from an account to the db then add it to accounts.
     * @param accountId
     * @param db an optional migrated database object
     * @note this method get details for an account from the daemon.
     */
    void addToAccounts(const QString& accountId, std::shared_ptr<Database> db = nullptr);

    /**
     * Remove account from accounts list. Emit accountRemoved.
     * @param accountId
     */
    void removeFromAccounts(const QString& accountId);

    /**
     * Sync changes to the accounts list with the lrc.
     */
    void updateAccounts();

    /**
     * Update accountInfo with details from daemon
     * @param account       account to update
     */
    void updateAccountDetails(account::Info& account);

    /**
     * get a modifiable account informations associated to an accountId.
     * @param accountId.
     * @return a account::Info& structure.
     */
    account::Info& getAccountInfo(const QString& accountId);

public Q_SLOTS:

    /**
     * Emit accountStatusChanged.
     * @param accountId
     * @param status
     */
    void slotAccountStatusChanged(const QString& accountID, const api::account::Status status);

    /**
     * Emit exportOnRingEnded.
     * @param accountId
     * @param status
     * @param pin
     */
    void slotExportOnRingEnded(const QString& accountID, int status, const QString& pin);

    /**
     * @param accountId
     * @param details
     */
    void slotAccountDetailsChanged(const QString& accountID, const MapStringString& details);

    /**
     * @param accountId
     * @param details
     */
    void slotVolatileAccountDetailsChanged(const QString& accountID, const MapStringString& details);

    /**
     * Emit nameRegistrationEnded
     * @param accountId
     * @param status
     * @param name
     */
    void slotNameRegistrationEnded(const QString& accountId, int status, const QString& name);

    /**
     * Emit registeredNameFound
     * @param accountId
     * @param status
     * @param address
     * @param name
     */
    void slotRegisteredNameFound(const QString& accountId,
                                 int status,
                                 const QString& address,
                                 const QString& name);

    /**
     * Emit migrationEnded
     * @param accountId
     * @param ok
     */
    void slotMigrationEnded(const QString& accountId, bool ok);

    /**
     * Emit accountProfileReceived
     * @param accountId
     * @param displayName
     * @param userPhoto
     */
    void slotAccountProfileReceived(const QString& accountId,
                                    const QString& displayName,
                                    const QString& userPhoto);
};

NewAccountModel::NewAccountModel(Lrc& lrc,
                                 const CallbacksHandler& callbacksHandler,
                                 const BehaviorController& behaviorController,
                                 MigrationCb& willMigrateCb,
                                 MigrationCb& didMigrateCb)
    : QObject(nullptr)
    , pimpl_(std::make_unique<NewAccountModelPimpl>(*this,
                                                    lrc,
                                                    callbacksHandler,
                                                    behaviorController,
                                                    willMigrateCb,
                                                    didMigrateCb))
{}

NewAccountModel::~NewAccountModel() {}

QStringList
NewAccountModel::getAccountList() const
{
    QStringList filteredAccountIds;
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto const& id : accountIds) {
        auto account = pimpl_->accounts.find(id);
        // Do not include accounts flagged for removal
        if (account != pimpl_->accounts.end() && account->second.first.valid)
            filteredAccountIds.push_back(id);
    }

    return filteredAccountIds;
}

void
NewAccountModel::setAccountEnabled(const QString& accountId, bool enabled) const
{
    auto& accountInfo = pimpl_->getAccountInfo(accountId);
    accountInfo.enabled = enabled;
    ConfigurationManager::instance().sendRegister(accountId, enabled);
}

void
NewAccountModel::setAccountConfig(const QString& accountId,
                                  const account::ConfProperties_t& confProperties) const
{
    auto& accountInfo = pimpl_->getAccountInfo(accountId);
    auto& configurationManager = ConfigurationManager::instance();
    MapStringString details = confProperties.toDetails();
    // Set values from Info. No need to include ID and TYPE. SIP accounts may modify the USERNAME
    // TODO: move these into the ConfProperties_t struct ?
    using namespace DRing::Account;
    details[ConfProperties::ENABLED] = accountInfo.enabled ? QString("true") : QString("false");
    details[ConfProperties::ALIAS] = accountInfo.profileInfo.alias;
    details[ConfProperties::DISPLAYNAME] = accountInfo.profileInfo.alias;
    details[ConfProperties::TYPE] = (accountInfo.profileInfo.type == profile::Type::JAMI)
                                        ? QString(ProtocolNames::RING)
                                        : QString(ProtocolNames::SIP);
    if (accountInfo.profileInfo.type == profile::Type::JAMI) {
        details[ConfProperties::USERNAME] = accountInfo.profileInfo.uri;
    } else if (accountInfo.profileInfo.type == profile::Type::SIP) {
        VectorMapStringString finalCred;

        MapStringString credentials;
        credentials[ConfProperties::USERNAME] = confProperties.username;
        credentials[ConfProperties::PASSWORD] = confProperties.password;
        credentials[ConfProperties::REALM] = confProperties.realm.isEmpty() ? "*"
                                                                            : confProperties.realm;

        auto credentialsVec = confProperties.credentials;
        credentialsVec[0] = credentials;
        for (auto const& i : credentialsVec) {
            QMap<QString, QString> credMap;
            for (auto const& j : i.toStdMap()) {
                credMap[j.first] = j.second;
            }
            finalCred.append(credMap);
        }

        ConfigurationManager::instance().setCredentials(accountId, finalCred);
        details[ConfProperties::USERNAME] = confProperties.username;
        accountInfo.confProperties.credentials.swap(credentialsVec);
    }
    configurationManager.setAccountDetails(accountId, details);
}

account::ConfProperties_t
NewAccountModel::getAccountConfig(const QString& accountId) const
{
    return getAccountInfo(accountId).confProperties;
}

void
NewAccountModel::setAlias(const QString& accountId, const QString& alias)
{
    auto& accountInfo = pimpl_->getAccountInfo(accountId);
    if (accountInfo.profileInfo.alias == alias)
        return;
    accountInfo.profileInfo.alias = alias;

    authority::storage::createOrUpdateProfile(accountInfo.id, accountInfo.profileInfo);

    emit profileUpdated(accountId);
}

void
NewAccountModel::setAvatar(const QString& accountId, const QString& avatar)
{
    auto& accountInfo = pimpl_->getAccountInfo(accountId);
    if (accountInfo.profileInfo.avatar == avatar)
        return;
    accountInfo.profileInfo.avatar = avatar;

    authority::storage::createOrUpdateProfile(accountInfo.id, accountInfo.profileInfo);

    emit profileUpdated(accountId);
}

bool
NewAccountModel::registerName(const QString& accountId,
                              const QString& password,
                              const QString& username)
{
    return ConfigurationManager::instance().registerName(accountId, password, username);
}

bool
NewAccountModel::exportToFile(const QString& accountId,
                              const QString& path,
                              const QString& password) const
{
    return ConfigurationManager::instance().exportToFile(accountId, path, password);
}

bool
NewAccountModel::exportOnRing(const QString& accountId, const QString& password) const
{
    return ConfigurationManager::instance().exportOnRing(accountId, password);
}

void
NewAccountModel::removeAccount(const QString& accountId) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end()) {
        return;
    }

    // Close db here for its removal
    account->second.second->close();
    ConfigurationManager::instance().removeAccount(accountId);
}

bool
NewAccountModel::changeAccountPassword(const QString& accountId,
                                       const QString& currentPassword,
                                       const QString& newPassword) const
{
    return ConfigurationManager::instance().changeAccountPassword(accountId,
                                                                  currentPassword,
                                                                  newPassword);
}

void
NewAccountModel::flagFreeable(const QString& accountId) const
{
    auto account = pimpl_->accounts.find(accountId);
    if (account == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::flagFreeable, can't find "
                                + accountId.toStdString());

    {
        std::lock_guard<std::mutex> lock(pimpl_->m_mutex_account_removal);
        account->second.first.freeable = true;
    }
    pimpl_->m_condVar_account_removal.notify_all();
}

const account::Info&
NewAccountModel::getAccountInfo(const QString& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::getAccountInfo, can't find "
                                + accountId.toStdString());

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
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    // NOTE: If the daemon is down, but dbus answered, id can contains
    // "Remote peer disconnected", "The name is not activable", etc.
    // So avoid to migrate useless directories.
    for (auto& id : accountIds)
        if (id.indexOf(" ") != -1) {
            qWarning() << "Invalid dbus answer. Daemon not running";
            return;
        }

    auto accountDbs = authority::storage::migrateIfNeeded(accountIds, willMigrateCb, didMigrateCb);
    for (const auto& id : accountIds) {
        addToAccounts(id, accountDbs.at(accountIds.indexOf(id)));
    }

    connect(&callbacksHandler,
            &CallbacksHandler::accountsChanged,
            this,
            &NewAccountModelPimpl::updateAccounts);
    connect(&callbacksHandler,
            &CallbacksHandler::accountStatusChanged,
            this,
            &NewAccountModelPimpl::slotAccountStatusChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::accountDetailsChanged,
            this,
            &NewAccountModelPimpl::slotAccountDetailsChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::volatileAccountDetailsChanged,
            this,
            &NewAccountModelPimpl::slotVolatileAccountDetailsChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::exportOnRingEnded,
            this,
            &NewAccountModelPimpl::slotExportOnRingEnded);
    connect(&callbacksHandler,
            &CallbacksHandler::nameRegistrationEnded,
            this,
            &NewAccountModelPimpl::slotNameRegistrationEnded);
    connect(&callbacksHandler,
            &CallbacksHandler::registeredNameFound,
            this,
            &NewAccountModelPimpl::slotRegisteredNameFound);
    connect(&callbacksHandler,
            &CallbacksHandler::migrationEnded,
            this,
            &NewAccountModelPimpl::slotMigrationEnded);
    connect(&callbacksHandler,
            &CallbacksHandler::accountProfileReceived,
            this,
            &NewAccountModelPimpl::slotAccountProfileReceived);
}

NewAccountModelPimpl::~NewAccountModelPimpl() {}

void
NewAccountModelPimpl::updateAccounts()
{
    qDebug() << "Syncing lrc accounts list with the daemon";
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    QStringList accountIds = configurationManager.getAccountList();

    // Detect removed accounts
    QStringList toBeRemoved;
    for (auto& it : accounts) {
        auto& accountInfo = it.second.first;
        if (!accountIds.contains(accountInfo.id)) {
            qDebug() << QString("detected account removal %1").arg(accountInfo.id);
            toBeRemoved.push_back(accountInfo.id);
        }
    }

    for (auto it = toBeRemoved.begin(); it != toBeRemoved.end(); ++it) {
        removeFromAccounts(*it);
    }

    // Detect new accounts
    for (auto& id : accountIds) {
        auto account = accounts.find(id);
        // NOTE: If the daemon is down, but dbus answered, id can contains
        // "Remote peer disconnected", "The name is not activable", etc.
        // So avoid to create useless directories.
        if (account == accounts.end() && id.indexOf(" ") == -1) {
            qWarning() << QString("detected new account %1").arg(id);
            addToAccounts(id);
            auto updatedAccount = accounts.find(id);
            if (updatedAccount == accounts.end()) {
                return;
            }
            if (updatedAccount->second.first.profileInfo.type == profile::Type::SIP) {
                // NOTE: At this point, a SIP account is ready, but not a Ring
                // account. Indeed, the keys are not generated at this point.
                // See slotAccountStatusChanged for more details.
                emit linked.accountAdded(id);
            }
        }
    }
}

void
NewAccountModelPimpl::updateAccountDetails(account::Info& accountInfo)
{
    // Fill account::Info struct with details from daemon
    MapStringString details = ConfigurationManager::instance().getAccountDetails(accountInfo.id);
    accountInfo.fromDetails(details);

    // Fill account::Info::confProperties credentials
    VectorMapStringString credGet = ConfigurationManager::instance().getCredentials(accountInfo.id);
    VectorMapStringString credToStore;
    for (auto const& i : std::vector<MapStringString>(credGet.begin(), credGet.end())) {
        MapStringString credMap;
        for (auto const& j : i.toStdMap()) {
            credMap[j.first] = j.second;
        }
        credToStore.push_back(credMap);
    }

    accountInfo.confProperties.credentials.swap(credToStore);

    MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(
        accountInfo.id);
    QString daemonStatus = volatileDetails[DRing::Account::ConfProperties::Registration::STATUS];
    accountInfo.status = lrc::api::account::to_status(daemonStatus);
}

account::Info&
NewAccountModelPimpl::getAccountInfo(const QString& accountId)
{
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        throw std::out_of_range("NewAccountModelPimpl::getAccountInfo, can't find "
                                + accountId.toStdString());
    }
    return account->second.first;
}

void
NewAccountModelPimpl::slotAccountStatusChanged(const QString& accountID,
                                               const api::account::Status status)
{
    if (status == api::account::Status::INVALID) {
        emit linked.invalidAccountDetected(accountID);
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
            // The account is already added and initialized. Just update details from daemon
            updateAccountDetails(accountInfo);
            emit linked.accountAdded(accountID);
        } else if (!accountInfo.profileInfo.uri.isEmpty()) {
            accountInfo.status = status;
            emit linked.accountStatusChanged(accountID);
        }
    } else {
        accountInfo.status = status;
        emit linked.accountStatusChanged(accountID);
    }
}

void
NewAccountModelPimpl::slotAccountDetailsChanged(const QString& accountId,
                                                const MapStringString& details)
{
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        throw std::out_of_range("NewAccountModelPimpl::slotAccountDetailsChanged, can't find "
                                + accountId.toStdString());
    }
    auto& accountInfo = account->second.first;
    accountInfo.fromDetails(details);
    if (username_changed) {
        username_changed = false;
        accountInfo.registeredName = new_username;
        emit linked.profileUpdated(accountId);
    }
    // TODO: Remove accountStatusChanged here.
    emit linked.accountStatusChanged(accountId);
    emit linked.accountDetailsChanged(accountId);
}

void
NewAccountModelPimpl::slotVolatileAccountDetailsChanged(const QString& accountId,
                                                        const MapStringString& details)
{
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        qWarning() << "NewAccountModelPimpl::slotVolatileAccountDetailsChanged, can't find "
                   << accountId;
        return;
    }
    auto& accountInfo = account->second.first;

    auto new_usernameIt = details.find(DRing::Account::VolatileProperties::REGISTERED_NAME);
    if (new_usernameIt == details.end())
        return;
    accountInfo.registeredName = new_usernameIt.value();
    emit linked.profileUpdated(accountId);
}

void
NewAccountModelPimpl::slotExportOnRingEnded(const QString& accountID, int status, const QString& pin)
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
    emit linked.exportOnRingEnded(accountID, convertedStatus, pin);
}

void
NewAccountModelPimpl::slotNameRegistrationEnded(const QString& accountId,
                                                int status,
                                                const QString& name)
{
    account::RegisterNameStatus convertedStatus = account::RegisterNameStatus::INVALID;
    switch (status) {
    case 0: {
        convertedStatus = account::RegisterNameStatus::SUCCESS;
        auto account = accounts.find(accountId);
        if (account != accounts.end() && account->second.first.registeredName.isEmpty()) {
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
    emit linked.nameRegistrationEnded(accountId, convertedStatus, name);
}

void
NewAccountModelPimpl::slotRegisteredNameFound(const QString& accountId,
                                              int status,
                                              const QString& address,
                                              const QString& name)
{
    account::LookupStatus convertedStatus = account::LookupStatus::INVALID;
    switch (status) {
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
    emit linked.registeredNameFound(accountId, convertedStatus, address, name);
}

void
NewAccountModelPimpl::slotMigrationEnded(const QString& accountId, bool ok)
{
    if (ok) {
        auto it = accounts.find(accountId);
        if (it == accounts.end()) {
            addToAccounts(accountId);
            return;
        }
        auto& accountInfo = it->second.first;
        MapStringString details = ConfigurationManager::instance().getAccountDetails(accountId);
        accountInfo.fromDetails(details);
        MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(
            accountId);
        QString daemonStatus = volatileDetails[DRing::Account::ConfProperties::Registration::STATUS];
        accountInfo.status = lrc::api::account::to_status(daemonStatus);
    }
    emit linked.migrationEnded(accountId, ok);
}

void
NewAccountModelPimpl::slotAccountProfileReceived(const QString& accountId,
                                                 const QString& displayName,
                                                 const QString& userPhoto)
{
    auto account = accounts.find(accountId);
    if (account == accounts.end())
        return;
    auto& accountInfo = account->second.first;
    accountInfo.profileInfo.avatar = userPhoto;
    accountInfo.profileInfo.alias = displayName;

    authority::storage::createOrUpdateProfile(accountInfo.id, accountInfo.profileInfo);

    emit linked.profileUpdated(accountId);
}

void
NewAccountModelPimpl::addToAccounts(const QString& accountId, std::shared_ptr<Database> db)
{
    if (db == nullptr) {
        try {
            auto appPath = authority::storage::getPath();
            auto dbName = accountId + "/history";
            db = DatabaseFactory::create<Database>(dbName, appPath);
            // create the profiles path if necessary
            QDir profilesDir(appPath + accountId + "/profiles");
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
    updateAccountDetails(newAccInfo);

    // Init models for this account
    newAccInfo.accountModel = &linked;
    newAccInfo.callModel = std::make_unique<NewCallModel>(newAccInfo,
                                                          lrc,
                                                          callbacksHandler,
                                                          behaviorController);
    newAccInfo.contactModel = std::make_unique<ContactModel>(newAccInfo,
                                                             *db,
                                                             callbacksHandler,
                                                             behaviorController);
    newAccInfo.conversationModel = std::make_unique<ConversationModel>(newAccInfo,
                                                                       lrc,
                                                                       *db,
                                                                       callbacksHandler,
                                                                       behaviorController);
    newAccInfo.peerDiscoveryModel = std::make_unique<PeerDiscoveryModel>(callbacksHandler,
                                                                         accountId);
    newAccInfo.deviceModel = std::make_unique<NewDeviceModel>(newAccInfo, callbacksHandler);
    newAccInfo.codecModel = std::make_unique<NewCodecModel>(newAccInfo, callbacksHandler);
    newAccInfo.dataTransferModel = std::make_unique<DataTransferModel>();
}

void
NewAccountModelPimpl::removeFromAccounts(const QString& accountId)
{
    /* Update db before waiting for the client to stop using the structs is fine
       as long as we don't free anything */
    auto account = accounts.find(accountId);
    if (account == accounts.end()) {
        return;
    }
    auto& accountInfo = account->second.first;
    if (accountInfo.profileInfo.type == profile::Type::SIP) {
        auto accountDir = QDir(authority::storage::getPath() + accountId);
        accountDir.removeRecursively();
    }

    /* Inform client about account removal. Do *not* free account structures
       before we are sure that the client stopped using it, otherwise we might
       get into use-after-free troubles. */
    accountInfo.valid = false;
    emit linked.accountRemoved(accountId);

#ifdef CHK_FREEABLE_BEFORE_ERASE_ACCOUNT
    std::unique_lock<std::mutex> lock(m_mutex_account_removal);
    // Wait for client to stop using old account structs
    m_condVar_account_removal.wait(lock, [&]() { return accounts[accountId].freeable; });
    lock.unlock();
#endif

    // Now we can free them
    accounts.erase(accountId);
}

void
account::Info::fromDetails(const MapStringString& details)
{
    using namespace DRing::Account;
    const MapStringString volatileDetails = ConfigurationManager::instance()
                                                .getVolatileAccountDetails(id);

    // General
    if (details[ConfProperties::TYPE] != "")
        profileInfo.type = details[ConfProperties::TYPE] == QString(ProtocolNames::RING)
                               ? profile::Type::JAMI
                               : profile::Type::SIP;
    registeredName = profileInfo.type == profile::Type::JAMI
                         ? volatileDetails[VolatileProperties::REGISTERED_NAME]
                         : "";
    profileInfo.alias = details[ConfProperties::DISPLAYNAME];
    enabled = toBool(details[ConfProperties::ENABLED]);
    confProperties.mailbox = details[ConfProperties::MAILBOX];
    confProperties.dtmfType = details[ConfProperties::DTMF_TYPE];
    confProperties.autoAnswer = toBool(details[ConfProperties::AUTOANSWER]);
    confProperties.sendReadReceipt = toBool(details[ConfProperties::SENDREADRECEIPT]);
    confProperties.isRendezVous = toBool(details[ConfProperties::ISRENDEZVOUS]);
    confProperties.activeCallLimit = toInt(details[ConfProperties::ACTIVE_CALL_LIMIT]);
    confProperties.hostname = details[ConfProperties::HOSTNAME];
    profileInfo.uri = (profileInfo.type == profile::Type::JAMI
                       and details[ConfProperties::USERNAME].contains("ring:"))
                          ? QString(details[ConfProperties::USERNAME]).remove(QString("ring:"))
                          : details[ConfProperties::USERNAME];
    confProperties.username = details[ConfProperties::USERNAME];
    confProperties.routeset = details[ConfProperties::ROUTE];
    confProperties.password = details[ConfProperties::PASSWORD];
    confProperties.realm = details[ConfProperties::REALM];
    confProperties.localInterface = details[ConfProperties::LOCAL_INTERFACE];
    confProperties.deviceId = details[ConfProperties::DEVICE_ID];
    confProperties.deviceName = details[ConfProperties::DEVICE_NAME];
    confProperties.publishedSameAsLocal = toBool(details[ConfProperties::PUBLISHED_SAMEAS_LOCAL]);
    confProperties.localPort = toInt(details[ConfProperties::LOCAL_PORT]);
    confProperties.publishedPort = toInt(details[ConfProperties::PUBLISHED_PORT]);
    confProperties.publishedAddress = details[ConfProperties::PUBLISHED_ADDRESS];
    confProperties.userAgent = details[ConfProperties::USER_AGENT];
    confProperties.upnpEnabled = toBool(details[ConfProperties::UPNP_ENABLED]);
    confProperties.hasCustomUserAgent = toBool(details[ConfProperties::HAS_CUSTOM_USER_AGENT]);
    confProperties.allowIncoming = toBool(details[ConfProperties::ALLOW_CERT_FROM_HISTORY])
                                   | toBool(details[ConfProperties::ALLOW_CERT_FROM_CONTACT])
                                   | toBool(details[ConfProperties::ALLOW_CERT_FROM_TRUSTED]);
    confProperties.allowIPAutoRewrite = toBool(details[ConfProperties::ACCOUNT_IP_AUTO_REWRITE]);
    confProperties.archivePassword = details[ConfProperties::ARCHIVE_PASSWORD];
    confProperties.archiveHasPassword = toBool(details[ConfProperties::ARCHIVE_HAS_PASSWORD]);
    confProperties.archivePath = details[ConfProperties::ARCHIVE_PATH];
    confProperties.archivePin = details[ConfProperties::ARCHIVE_PIN];
    confProperties.proxyEnabled = toBool(details[ConfProperties::PROXY_ENABLED]);
    confProperties.proxyServer = details[ConfProperties::PROXY_SERVER];
    confProperties.proxyPushToken = details[ConfProperties::PROXY_PUSH_TOKEN];
    confProperties.peerDiscovery = toBool(details[ConfProperties::DHT_PEER_DISCOVERY]);
    confProperties.accountDiscovery = toBool(details[ConfProperties::ACCOUNT_PEER_DISCOVERY]);
    confProperties.accountPublish = toBool(details[ConfProperties::ACCOUNT_PUBLISH]);
    confProperties.keepAliveEnabled = toBool(details[ConfProperties::KEEP_ALIVE_ENABLED]);
    confProperties.bootstrapListUrl = QString(details[ConfProperties::BOOTSTRAP_LIST_URL]);
    confProperties.dhtProxyListUrl = QString(details[ConfProperties::DHT_PROXY_LIST_URL]);
    confProperties.defaultModerators = QString(details[ConfProperties::DEFAULT_MODERATORS]);
    confProperties.localModeratorsEnabled = toBool(
        details[ConfProperties::LOCAL_MODERATORS_ENABLED]);
    // Audio
    confProperties.Audio.audioPortMax = toInt(details[ConfProperties::Audio::PORT_MAX]);
    confProperties.Audio.audioPortMin = toInt(details[ConfProperties::Audio::PORT_MIN]);
    // Video
    confProperties.Video.videoEnabled = toBool(details[ConfProperties::Video::ENABLED]);
    confProperties.Video.videoPortMax = toInt(details[ConfProperties::Video::PORT_MAX]);
    confProperties.Video.videoPortMin = toInt(details[ConfProperties::Video::PORT_MIN]);
    // STUN
    confProperties.STUN.server = details[ConfProperties::STUN::SERVER];
    confProperties.STUN.enable = toBool(details[ConfProperties::STUN::ENABLED]);
    // TURN
    confProperties.TURN.server = details[ConfProperties::TURN::SERVER];
    confProperties.TURN.enable = toBool(details[ConfProperties::TURN::ENABLED]);
    confProperties.TURN.username = details[ConfProperties::TURN::SERVER_UNAME];
    confProperties.TURN.password = details[ConfProperties::TURN::SERVER_PWD];
    confProperties.TURN.realm = details[ConfProperties::TURN::SERVER_REALM];
    // Presence
    confProperties.Presence.presencePublishSupported = toBool(
        details[ConfProperties::Presence::SUPPORT_PUBLISH]);
    confProperties.Presence.presenceSubscribeSupported = toBool(
        details[ConfProperties::Presence::SUPPORT_SUBSCRIBE]);
    confProperties.Presence.presenceEnabled = toBool(details[ConfProperties::Presence::ENABLED]);
    // Ringtone
    confProperties.Ringtone.ringtonePath = details[ConfProperties::Ringtone::PATH];
    confProperties.Ringtone.ringtoneEnabled = toBool(details[ConfProperties::Ringtone::ENABLED]);
    // SRTP
    confProperties.SRTP.keyExchange = details[ConfProperties::SRTP::KEY_EXCHANGE].isEmpty()
                                          ? account::KeyExchangeProtocol::NONE
                                          : account::KeyExchangeProtocol::SDES;
    confProperties.SRTP.enable = toBool(details[ConfProperties::SRTP::ENABLED]);
    confProperties.SRTP.rtpFallback = toBool(details[ConfProperties::SRTP::RTP_FALLBACK]);
    // TLS
    confProperties.TLS.listenerPort = toInt(details[ConfProperties::TLS::LISTENER_PORT]);
    confProperties.TLS.enable = details[ConfProperties::TYPE] == QString(ProtocolNames::RING)
                                    ? true
                                    : toBool(details[ConfProperties::TLS::ENABLED]);
    confProperties.TLS.port = toInt(details[ConfProperties::TLS::PORT]);
    confProperties.TLS.certificateListFile = details[ConfProperties::TLS::CA_LIST_FILE];
    confProperties.TLS.certificateFile = details[ConfProperties::TLS::CERTIFICATE_FILE];
    confProperties.TLS.privateKeyFile = details[ConfProperties::TLS::PRIVATE_KEY_FILE];
    confProperties.TLS.password = details[ConfProperties::TLS::PASSWORD];
    auto method = toStdString(details[ConfProperties::TLS::METHOD]);
    if (method == "TLSv1") {
        confProperties.TLS.method = account::TlsMethod::TLSv1;
    } else if (method == "TLSv1.1") {
        confProperties.TLS.method = account::TlsMethod::TLSv1_1;
    } else if (method == "TLSv1.2") {
        confProperties.TLS.method = account::TlsMethod::TLSv1_2;
    } else {
        confProperties.TLS.method = account::TlsMethod::DEFAULT;
    }
    confProperties.TLS.ciphers = details[ConfProperties::TLS::CIPHERS];
    confProperties.TLS.serverName = details[ConfProperties::TLS::SERVER_NAME];
    confProperties.TLS.verifyServer = toBool(details[ConfProperties::TLS::VERIFY_SERVER]);
    confProperties.TLS.verifyClient = toBool(details[ConfProperties::TLS::VERIFY_CLIENT]);
    confProperties.TLS.requireClientCertificate = toBool(
        details[ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE]);
    confProperties.TLS.negotiationTimeoutSec = toInt(
        details[ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC]);
    // DHT
    confProperties.DHT.port = toInt(details[ConfProperties::DHT::PORT]);
    confProperties.DHT.PublicInCalls = toBool(details[ConfProperties::DHT::PUBLIC_IN_CALLS]);
    confProperties.DHT.AllowFromTrusted = toBool(details[ConfProperties::DHT::ALLOW_FROM_TRUSTED]);
    // RingNS
    confProperties.RingNS.uri = details[ConfProperties::RingNS::URI];
    confProperties.RingNS.account = details[ConfProperties::RingNS::ACCOUNT];
    // Registration
    confProperties.Registration.expire = toInt(details[ConfProperties::Registration::EXPIRE]);
    // Jams
    confProperties.managerUri = details[ConfProperties::MANAGER_URI];
    confProperties.managerUsername = details[ConfProperties::MANAGER_USERNAME];
}

MapStringString
account::ConfProperties_t::toDetails() const
{
    using namespace DRing::Account;
    MapStringString details;
    // General
    details[ConfProperties::MAILBOX] = this->mailbox;
    details[ConfProperties::DTMF_TYPE] = this->dtmfType;
    details[ConfProperties::AUTOANSWER] = toQString(this->autoAnswer);
    details[ConfProperties::SENDREADRECEIPT] = toQString(this->sendReadReceipt);
    details[ConfProperties::ISRENDEZVOUS] = toQString(this->isRendezVous);
    details[ConfProperties::ACTIVE_CALL_LIMIT] = toQString(this->activeCallLimit);
    details[ConfProperties::HOSTNAME] = this->hostname;
    details[ConfProperties::ROUTE] = this->routeset;
    details[ConfProperties::PASSWORD] = this->password;
    details[ConfProperties::REALM] = this->realm;
    details[ConfProperties::DEVICE_ID] = this->deviceId;
    details[ConfProperties::DEVICE_NAME] = this->deviceName;
    details[ConfProperties::LOCAL_INTERFACE] = this->localInterface;
    details[ConfProperties::PUBLISHED_SAMEAS_LOCAL] = toQString(this->publishedSameAsLocal);
    details[ConfProperties::LOCAL_PORT] = toQString(this->localPort);
    details[ConfProperties::PUBLISHED_PORT] = toQString(this->publishedPort);
    details[ConfProperties::PUBLISHED_ADDRESS] = this->publishedAddress;
    details[ConfProperties::USER_AGENT] = this->userAgent;
    details[ConfProperties::UPNP_ENABLED] = toQString(this->upnpEnabled);
    details[ConfProperties::HAS_CUSTOM_USER_AGENT] = toQString(this->hasCustomUserAgent);
    details[ConfProperties::ALLOW_CERT_FROM_HISTORY] = toQString(this->allowIncoming);
    details[ConfProperties::ALLOW_CERT_FROM_CONTACT] = toQString(this->allowIncoming);
    details[ConfProperties::ALLOW_CERT_FROM_TRUSTED] = toQString(this->allowIncoming);
    details[ConfProperties::ACCOUNT_IP_AUTO_REWRITE] = toQString(this->allowIPAutoRewrite);
    details[ConfProperties::ARCHIVE_PASSWORD] = this->archivePassword;
    details[ConfProperties::ARCHIVE_HAS_PASSWORD] = toQString(this->archiveHasPassword);
    details[ConfProperties::ARCHIVE_PATH] = this->archivePath;
    details[ConfProperties::ARCHIVE_PIN] = this->archivePin;
    // ConfProperties::DEVICE_NAME name is set with NewDeviceModel interface
    details[ConfProperties::PROXY_ENABLED] = toQString(this->proxyEnabled);
    details[ConfProperties::PROXY_SERVER] = this->proxyServer;
    details[ConfProperties::PROXY_PUSH_TOKEN] = this->proxyPushToken;
    details[ConfProperties::DHT_PEER_DISCOVERY] = toQString(this->peerDiscovery);
    details[ConfProperties::ACCOUNT_PEER_DISCOVERY] = toQString(this->accountDiscovery);
    details[ConfProperties::ACCOUNT_PUBLISH] = toQString(this->accountPublish);
    details[ConfProperties::KEEP_ALIVE_ENABLED] = toQString(this->keepAliveEnabled);
    details[ConfProperties::BOOTSTRAP_LIST_URL] = this->bootstrapListUrl;
    details[ConfProperties::DHT_PROXY_LIST_URL] = this->dhtProxyListUrl;
    details[ConfProperties::DEFAULT_MODERATORS] = this->defaultModerators;
    details[ConfProperties::LOCAL_MODERATORS_ENABLED] = toQString(this->localModeratorsEnabled);
    // Audio
    details[ConfProperties::Audio::PORT_MAX] = toQString(this->Audio.audioPortMax);
    details[ConfProperties::Audio::PORT_MIN] = toQString(this->Audio.audioPortMin);
    // Video
    details[ConfProperties::Video::ENABLED] = toQString(this->Video.videoEnabled);
    details[ConfProperties::Video::PORT_MAX] = toQString(this->Video.videoPortMax);
    details[ConfProperties::Video::PORT_MIN] = toQString(this->Video.videoPortMin);
    // STUN
    details[ConfProperties::STUN::SERVER] = this->STUN.server;
    details[ConfProperties::STUN::ENABLED] = toQString(this->STUN.enable);
    // TURN
    details[ConfProperties::TURN::SERVER] = this->TURN.server;
    details[ConfProperties::TURN::ENABLED] = toQString(this->TURN.enable);
    details[ConfProperties::TURN::SERVER_UNAME] = this->TURN.username;
    details[ConfProperties::TURN::SERVER_PWD] = this->TURN.password;
    details[ConfProperties::TURN::SERVER_REALM] = this->TURN.realm;
    // Presence
    details[ConfProperties::Presence::SUPPORT_PUBLISH] = toQString(
        this->Presence.presencePublishSupported);
    details[ConfProperties::Presence::SUPPORT_SUBSCRIBE] = toQString(
        this->Presence.presenceSubscribeSupported);
    details[ConfProperties::Presence::ENABLED] = toQString(this->Presence.presenceEnabled);
    // Ringtone
    details[ConfProperties::Ringtone::PATH] = this->Ringtone.ringtonePath;
    details[ConfProperties::Ringtone::ENABLED] = toQString(this->Ringtone.ringtoneEnabled);
    // SRTP
    details[ConfProperties::SRTP::KEY_EXCHANGE] = this->SRTP.keyExchange
                                                          == account::KeyExchangeProtocol::NONE
                                                      ? ""
                                                      : "sdes";
    details[ConfProperties::SRTP::ENABLED] = toQString(this->SRTP.enable);
    details[ConfProperties::SRTP::RTP_FALLBACK] = toQString(this->SRTP.rtpFallback);
    // TLS
    details[ConfProperties::TLS::LISTENER_PORT] = toQString(this->TLS.listenerPort);
    details[ConfProperties::TLS::ENABLED] = toQString(this->TLS.enable);
    details[ConfProperties::TLS::PORT] = toQString(this->TLS.port);
    details[ConfProperties::TLS::CA_LIST_FILE] = this->TLS.certificateListFile;
    details[ConfProperties::TLS::CERTIFICATE_FILE] = this->TLS.certificateFile;
    details[ConfProperties::TLS::PRIVATE_KEY_FILE] = this->TLS.privateKeyFile;
    details[ConfProperties::TLS::PASSWORD] = this->TLS.password;
    switch (this->TLS.method) {
    case account::TlsMethod::TLSv1:
        details[ConfProperties::TLS::METHOD] = "TLSv1";
        break;
    case account::TlsMethod::TLSv1_1:
        details[ConfProperties::TLS::METHOD] = "TLSv1.1";
        break;
    case account::TlsMethod::TLSv1_2:
        details[ConfProperties::TLS::METHOD] = "TLSv1.2";
        break;
    case account::TlsMethod::DEFAULT:
    default:
        details[ConfProperties::TLS::METHOD] = "Default";
        break;
    }
    details[ConfProperties::TLS::CIPHERS] = this->TLS.ciphers;
    details[ConfProperties::TLS::SERVER_NAME] = this->TLS.serverName;
    details[ConfProperties::TLS::VERIFY_SERVER] = toQString(this->TLS.verifyServer);
    details[ConfProperties::TLS::VERIFY_CLIENT] = toQString(this->TLS.verifyClient);
    details[ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE] = toQString(
        this->TLS.requireClientCertificate);
    details[ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC] = toQString(
        this->TLS.negotiationTimeoutSec);
    // DHT
    details[ConfProperties::DHT::PORT] = toQString(this->DHT.port);
    details[ConfProperties::DHT::PUBLIC_IN_CALLS] = toQString(this->DHT.PublicInCalls);
    details[ConfProperties::DHT::ALLOW_FROM_TRUSTED] = toQString(this->DHT.AllowFromTrusted);
    // RingNS
    details[ConfProperties::RingNS::URI] = this->RingNS.uri;
    details[ConfProperties::RingNS::ACCOUNT] = this->RingNS.account;
    // Registration
    details[ConfProperties::Registration::EXPIRE] = toQString(this->Registration.expire);
    // Manager
    details[ConfProperties::MANAGER_URI] = this->managerUri;
    details[ConfProperties::MANAGER_USERNAME] = this->managerUsername;

    return details;
}

QString
NewAccountModel::createNewAccount(profile::Type type,
                                  const QString& displayName,
                                  const QString& archivePath,
                                  const QString& password,
                                  const QString& pin,
                                  const QString& uri,
                                  const MapStringString& config)
{
    MapStringString details = type == profile::Type::SIP
                                  ? ConfigurationManager::instance().getAccountTemplate("SIP")
                                  : ConfigurationManager::instance().getAccountTemplate("RING");
    using namespace DRing::Account;
    details[ConfProperties::TYPE] = type == profile::Type::SIP ? "SIP" : "RING";
    details[ConfProperties::DISPLAYNAME] = displayName;
    details[ConfProperties::ALIAS] = displayName;
    details[ConfProperties::UPNP_ENABLED] = "true";
    details[ConfProperties::ARCHIVE_PASSWORD] = password;
    details[ConfProperties::ARCHIVE_PIN] = pin;
    details[ConfProperties::ARCHIVE_PATH] = archivePath;
    if (type == profile::Type::SIP)
        details[ConfProperties::USERNAME] = uri;
    if (!config.isEmpty()) {
        for (MapStringString::const_iterator it = config.begin(); it != config.end(); it++) {
            details[it.key()] = it.value();
        }
    }

    QString accountId = ConfigurationManager::instance().addAccount(details);
    return accountId;
}

QString
NewAccountModel::connectToAccountManager(const QString& username,
                                         const QString& password,
                                         const QString& serverUri,
                                         const MapStringString& config)
{
    MapStringString details = ConfigurationManager::instance().getAccountTemplate("RING");
    using namespace DRing::Account;
    details[ConfProperties::TYPE] = "RING";
    details[ConfProperties::MANAGER_URI] = serverUri;
    details[ConfProperties::MANAGER_USERNAME] = username;
    details[ConfProperties::ARCHIVE_PASSWORD] = password;
    if (!config.isEmpty()) {
        for (MapStringString::const_iterator it = config.begin(); it != config.end(); it++) {
            details[it.key()] = it.value();
        }
    }

    QString accountId = ConfigurationManager::instance().addAccount(details);
    return accountId;
}

void
NewAccountModel::setTopAccount(const QString& accountId)
{
    bool found = false;
    QString order = {};

    const QStringList accountIds = ConfigurationManager::instance().getAccountList();
    for (auto& id : accountIds) {
        if (id == accountId) {
            found = true;
        } else {
            order += id + "/";
        }
    }
    if (found) {
        order = accountId + "/" + order;
    }
    ConfigurationManager::instance().setAccountsOrder(order);
}

QString
NewAccountModel::accountVCard(const QString& accountId, bool compressImage) const
{
    return authority::storage::vcard::profileToVcard(getAccountInfo(accountId).profileInfo,
                                                     compressImage);
}

const QString
NewAccountModel::bestNameForAccount(const QString& accountID)
{
    // Order: Alias, registeredName, uri
    auto& accountInfo = getAccountInfo(accountID);

    auto alias = accountInfo.profileInfo.alias.simplified();
    auto registeredName = accountInfo.registeredName.simplified();
    auto infoHash = accountInfo.profileInfo.uri.simplified();

    if (alias.isEmpty()) {
        if (registeredName.isEmpty())
            return infoHash;
        else
            return registeredName;
    }
    return alias;
}

const QString
NewAccountModel::bestIdForAccount(const QString& accountID)
{
    // Order: RegisteredName, uri after best name
    //        return empty string if duplicated with best name
    auto& accountInfo = getAccountInfo(accountID);

    auto registeredName = accountInfo.registeredName.simplified();
    auto infoHash = accountInfo.profileInfo.uri.simplified();

    return registeredName.isEmpty() ? infoHash : registeredName;
}

void
NewAccountModel::setDefaultModerator(const QString& accountID,
                                     const QString& peerURI,
                                     const bool& state)
{
    ConfigurationManager::instance().setDefaultModerator(accountID, peerURI, state);
}

QStringList
NewAccountModel::getDefaultModerators(const QString& accountID)
{
    return ConfigurationManager::instance().getDefaultModerators(accountID);
}

void
NewAccountModel::enableLocalModerators(const QString& accountID, const bool& isModEnabled)
{
    ConfigurationManager::instance().enableLocalModerators(accountID, isModEnabled);
}

bool
NewAccountModel::isLocalModeratorsEnabled(const QString& accountID)
{
    return ConfigurationManager::instance().isLocalModeratorsEnabled(accountID);
}

void
NewAccountModel::setAllModerators(const QString& accountID, const bool& allModerators)
{
    ConfigurationManager::instance().setAllModerators(accountID, allModerators);
}

bool
NewAccountModel::isAllModerators(const QString& accountID)
{
    return ConfigurationManager::instance().isAllModerators(accountID);
}

int
NewAccountModel::notificationsCount() const
{
    auto total = 0;
    for (const auto& [_id, account] : pimpl_->accounts) {
        total += account.first.conversationModel->notificationsCount();
    }
    return total;
}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
#include "newaccountmodel.moc"

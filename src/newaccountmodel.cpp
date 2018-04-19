/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// new LRC
#include "api/lrc.h"
#include "api/newcallmodel.h"
#include "api/contactmodel.h"
#include "api/conversationmodel.h"
#include "api/newdevicemodel.h"
#include "api/account.h"
#include "api/behaviorcontroller.h"
#include "authority/databasehelper.h"
#include "callbackshandler.h"
#include "database.h"

// old LRC
#include "accountmodel.h"
#include "profilemodel.h"
#include "profile.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class NewAccountModelPimpl: public QObject
{
    Q_OBJECT
public:
    NewAccountModelPimpl(NewAccountModel& linked,
                         Lrc& lrc,
                         Database& database,
                         const CallbacksHandler& callbackHandler,
                         const BehaviorController& behaviorController);
    ~NewAccountModelPimpl();

    NewAccountModel& linked;
    Lrc& lrc;
    const CallbacksHandler& callbacksHandler;
    Database& database;
    NewAccountModel::AccountInfoMap accounts;
    const BehaviorController& behaviorController;

    // Synchronization tools for account removal
    std::mutex m_mutex_account_removal;
    std::condition_variable m_condVar_account_removal;

    /**
     * Add the profile information from an account to the db then add it to accounts.
     * @param accountId
     * @note this method get details for an account from the daemon.
     */
    void addToAccounts(const std::string& accountId);

public Q_SLOTS:
    /**
     * Emit accountStatusChanged.
     * @param accountId
     * @param status
     */
    void slotAccountStatusChanged(const std::string& accountID, const api::account::Status status);
    /**
     * Emit accountRemoved.
     * @param account
     */
    void slotAccountRemoved(Account* account);

    void slotProfileUpdated(const Profile* profile);
};

NewAccountModel::NewAccountModel(Lrc& lrc,
                                 Database& database,
                                 const CallbacksHandler& callbacksHandler,
                                 const BehaviorController& behaviorController)
: QObject()
, pimpl_(std::make_unique<NewAccountModelPimpl>(*this, lrc, database, callbacksHandler, behaviorController))
{
}

NewAccountModel::~NewAccountModel()
{
}

std::vector<std::string>
NewAccountModel::getAccountList() const
{
    std::vector<std::string> accountsId;

    for(auto const& accountInfo: pimpl_->accounts) {
        // Do not include accounts flagged for removal
        if (accountInfo.second.valid)
            accountsId.emplace_back(accountInfo.first);
    }

    return accountsId;
}

void
NewAccountModel::save(const std::string& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end()) {
        throw std::out_of_range("NewAccountModel::save, can't find " + accountId);
    }

    auto& configurationManager = ConfigurationManager::instance();
    configurationManager.setAccountDetails(QString::fromStdString(accountId), accountInfo->second.toDetails());
}

void
NewAccountModel::flagFreeable(const std::string& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::flagFreeable, can't find " + accountId);

    {
        std::lock_guard<std::mutex> lock(pimpl_->m_mutex_account_removal);
        accountInfo->second.freeable = true;
    }
    pimpl_->m_condVar_account_removal.notify_all();
}

const account::Info&
NewAccountModel::getAccountInfo(const std::string& accountId) const
{
    auto accountInfo = pimpl_->accounts.find(accountId);
    if (accountInfo == pimpl_->accounts.end())
        throw std::out_of_range("NewAccountModel::getAccountInfo, can't find " + accountId);

    return accountInfo->second;
}

NewAccountModelPimpl::NewAccountModelPimpl(NewAccountModel& linked,
                                           Lrc& lrc,
                                           Database& database,
                                           const CallbacksHandler& callbacksHandler,
                                           const BehaviorController& behaviorController)
: linked(linked)
, lrc {lrc}
, behaviorController(behaviorController)
, callbacksHandler(callbacksHandler)
, database(database)
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();

    for (auto& id : accountIds)
        addToAccounts(id.toStdString());

    connect(&callbacksHandler, &CallbacksHandler::accountStatusChanged, this, &NewAccountModelPimpl::slotAccountStatusChanged);

    // NOTE: because we still use the legacy LRC for configuration, we are still using old signals
    connect(&AccountModel::instance(), &AccountModel::accountRemoved, this,  &NewAccountModelPimpl::slotAccountRemoved);
    connect(&ProfileModel::instance(), &ProfileModel::profileUpdated, this,  &NewAccountModelPimpl::slotProfileUpdated);
}

NewAccountModelPimpl::~NewAccountModelPimpl()
{

}

void
NewAccountModelPimpl::slotAccountStatusChanged(const std::string& accountID, const api::account::Status status)
{
    auto accountInfo = accounts.find(accountID);
    if (status == api::account::Status::REGISTERED && accountInfo == accounts.end()) {
        // Update account
        // NOTE we don't connect to newAccountAdded from AccountModel
        // because the account is not ready.
        accounts.erase(accountID);
        addToAccounts(accountID);
        emit linked.accountAdded(accountID);
    } else if (accountInfo != accounts.end()) {
        accountInfo->second.status = status;
        if (status == api::account::Status::REGISTERED and not accounts[accountID].enabled) {
            accounts[accountID].enabled = true;
            emit linked.accountStatusChanged(accountID);
        } else if (status == api::account::Status::UNREGISTERED and accounts[accountID].enabled) {
            accounts[accountID].enabled = false;
            emit linked.accountStatusChanged(accountID);
        } else
            emit linked.accountStatusChanged(accountID);
    }
}

void
NewAccountModelPimpl::addToAccounts(const std::string& accountId)
{
    // Init profile
    auto& item = *(accounts.emplace(accountId, account::Info()).first);
    auto& owner = item.second;

    // Fill account::Info struct with details from daemon
    owner.fromDetails(accountId);

    // Add profile into database
    auto accountType = owner.profileInfo.type == profile::Type::RING ? std::string("RING") : std::string("SIP");
    auto accountProfileId = authority::database::getOrInsertProfile(database, owner.profileInfo.uri,
                                                                    owner.profileInfo.alias, "",
                                                                    accountType);
    // Retrieve avatar from database
    auto avatar = authority::database::getAvatarForProfileId(database, accountProfileId);
    owner.profileInfo.avatar = avatar;
    // Init models for this account
    owner.callModel = std::make_unique<NewCallModel>(owner, callbacksHandler);
    owner.contactModel = std::make_unique<ContactModel>(owner, database, callbacksHandler);
    owner.conversationModel = std::make_unique<ConversationModel>(owner, lrc, database, callbacksHandler, behaviorController);
    owner.deviceModel = std::make_unique<NewDeviceModel>(owner, callbacksHandler);
    owner.accountModel = &linked;
}

void
NewAccountModelPimpl::slotAccountRemoved(Account* account)
{
    auto accountId = account->id().toStdString();

    /* Update db before waiting for the client to stop using the structs is fine
       as long as we don't free anything */
    authority::database::removeAccount(database, accounts[accountId].profileInfo.uri);

    /* Inform client about account removal. Do *not* free account structures
       before we are sure that the client stopped using it, otherwise we might
       get into use-after-free troubles. */
    accounts[accountId].valid = false;
    emit linked.accountRemoved(accountId);

#ifdef CHK_FREEABLE_BEFORE_ERASE_ACCOUNT
    std::unique_lock<std::mutex> lock(m_mutex_account_removal);
    // Wait for client to stop using old account structs
    m_condVar_account_removal.wait(lock, [&](){return accounts[accountId].freeable;});
    lock.unlock();
#endif

    // Now we can free them
    accounts.erase(accountId);
}

void
NewAccountModelPimpl::slotProfileUpdated(const Profile* profile)
{
    auto& accounts = profile->accounts();
    if (!accounts.empty())
        emit linked.profileUpdated(accounts.first()->id().toStdString());
}

void
account::Info::fromDetails(const std::string& accountId)
{
    using namespace DRing::Account;
    MapStringString details = ConfigurationManager::instance().getAccountDetails(accountId.c_str());
    const MapStringString volatileDetails = ConfigurationManager::instance().getVolatileAccountDetails(accountId.c_str());

    // General
    id                                                  = accountId;
    profileInfo.type                                    = details[ConfProperties::TYPE] == QString(ProtocolNames::RING) ? profile::Type::RING : profile::Type::SIP;
    registeredName                                      = profileInfo.type == profile::Type::RING ? volatileDetails[VolatileProperties::REGISTERED_NAME].toStdString() : profileInfo.alias;
    profileInfo.alias                                   = toStdString(details[ConfProperties::ALIAS]);
    ConfProperties.displayName                          = toStdString(details[ConfProperties::DISPLAYNAME]);
    enabled                                             = toBool(details[ConfProperties::ENABLED]);
    ConfProperties.mailbox                              = toStdString(details[ConfProperties::MAILBOX]);
    ConfProperties.dtmfType                             = toStdString(details[ConfProperties::DTMF_TYPE]);
    ConfProperties.autoAnswer                           = toBool(details[ConfProperties::AUTOANSWER]);
    ConfProperties.activeCallLimit                      = toInt(details[ConfProperties::ACTIVE_CALL_LIMIT]);
    ConfProperties.hostname                             = toStdString(details[ConfProperties::HOSTNAME]);
    profileInfo.uri                                     = (profileInfo.type == profile::Type::RING and details[ConfProperties::USERNAME].contains("ring:")) ? details[ConfProperties::USERNAME].toStdString().substr(std::string("ring:").size()) : details[ConfProperties::USERNAME].toStdString();
    ConfProperties.routeset                             = toStdString(details[ConfProperties::ROUTE]);
    ConfProperties.password                             = toStdString(details[ConfProperties::PASSWORD]);
    ConfProperties.realm                                = toStdString(details[ConfProperties::REALM]);
    ConfProperties.localInterface                       = toStdString(details[ConfProperties::LOCAL_INTERFACE]);
    ConfProperties.publishedSameAsLocal                 = toBool(details[ConfProperties::PUBLISHED_SAMEAS_LOCAL]);
    ConfProperties.localPort                            = toInt(details[ConfProperties::LOCAL_PORT]);
    ConfProperties.publishedPort                        = toInt(details[ConfProperties::PUBLISHED_PORT]);
    ConfProperties.publishedAddress                     = toStdString(details[ConfProperties::PUBLISHED_ADDRESS]);
    ConfProperties.userAgent                            = toStdString(details[ConfProperties::USER_AGENT]);
    ConfProperties.upnpEnabled                          = toBool(details[ConfProperties::UPNP_ENABLED]);
    ConfProperties.hasCustomUserAgent                   = toBool(details[ConfProperties::HAS_CUSTOM_USER_AGENT]);
    ConfProperties.allowIncomingFromHistory             = toBool(details[ConfProperties::ALLOW_CERT_FROM_HISTORY]);
    ConfProperties.allowIncomingFromContact             = toBool(details[ConfProperties::ALLOW_CERT_FROM_CONTACT]);
    ConfProperties.allowIncomingFromTrusted             = toBool(details[ConfProperties::ALLOW_CERT_FROM_TRUSTED]);
    ConfProperties.archivePassword                      = toStdString(details[ConfProperties::ARCHIVE_PASSWORD]);
    ConfProperties.archiveHasPassword                   = toBool(details[ConfProperties::ARCHIVE_HAS_PASSWORD]);
    ConfProperties.archivePath                          = toStdString(details[ConfProperties::ARCHIVE_PATH]);
    ConfProperties.archivePin                           = toStdString(details[ConfProperties::ARCHIVE_PIN]);
    ConfProperties.deviceID                             = toStdString(details[ConfProperties::RING_DEVICE_ID]);
    ConfProperties.deviceName                           = toStdString(details[ConfProperties::RING_DEVICE_NAME]);
    ConfProperties.proxyEnabled                         = toBool(details[ConfProperties::PROXY_ENABLED]);
    ConfProperties.proxyServer                          = toStdString(details[ConfProperties::PROXY_SERVER]);
    ConfProperties.proxyPushToken                       = toStdString(details[ConfProperties::PROXY_PUSH_TOKEN]);
    // Audio
    ConfProperties.Audio.audioPortMax                   = toInt(details[ConfProperties::Audio::PORT_MAX]);
    ConfProperties.Audio.audioPortMin                   = toInt(details[ConfProperties::Audio::PORT_MIN]);
    // Video
    ConfProperties.Video.videoEnabled                   = toBool(details[ConfProperties::Video::ENABLED]);
    ConfProperties.Video.videoPortMax                   = toInt(details[ConfProperties::Video::PORT_MAX]);
    ConfProperties.Video.videoPortMin                   = toInt(details[ConfProperties::Video::PORT_MIN]);
    // STUN
    ConfProperties.STUN.server                          = toStdString(details[ConfProperties::STUN::SERVER]);
    ConfProperties.STUN.enable                          = toBool(details[ConfProperties::STUN::ENABLED]);
    // TURN
    ConfProperties.TURN.server                          = toStdString(details[ConfProperties::TURN::SERVER]);
    ConfProperties.TURN.enable                          = toBool(details[ConfProperties::TURN::ENABLED]);
    ConfProperties.TURN.username                        = toStdString(details[ConfProperties::TURN::SERVER_UNAME]);
    ConfProperties.TURN.password                        = toStdString(details[ConfProperties::TURN::SERVER_PWD]);
    ConfProperties.TURN.realm                           = toStdString(details[ConfProperties::TURN::SERVER_REALM]);
    // Presence
    ConfProperties.Presence.presencePublishSupported    = toBool(details[ConfProperties::Presence::SUPPORT_PUBLISH]);
    ConfProperties.Presence.presenceSubscribeSupported  = toBool(details[ConfProperties::Presence::SUPPORT_SUBSCRIBE]);
    ConfProperties.Presence.presenceEnabled             = toBool(details[ConfProperties::Presence::ENABLED]);
    // Ringtone
    ConfProperties.Ringtone.ringtonePath                = toStdString(details[ConfProperties::Ringtone::PATH]);
    ConfProperties.Ringtone.ringtoneEnabled             = toBool(details[ConfProperties::Ringtone::ENABLED]);
    // SRTP
    ConfProperties.SRTP.keyExchange                     = toStdString(details[ConfProperties::SRTP::KEY_EXCHANGE]);
    ConfProperties.SRTP.enable                          = toBool(details[ConfProperties::SRTP::ENABLED]);
    ConfProperties.SRTP.rtpFallback                     = toBool(details[ConfProperties::SRTP::RTP_FALLBACK]);
    // TLS
    ConfProperties.TLS.listenerPort                     = toInt(details[ConfProperties::TLS::LISTENER_PORT]);
    ConfProperties.TLS.enable                           = toBool(details[ConfProperties::TLS::ENABLED]);
    ConfProperties.TLS.port                             = toInt(details[ConfProperties::TLS::PORT]);
    ConfProperties.TLS.certificateListFile              = toStdString(details[ConfProperties::TLS::CA_LIST_FILE]);
    ConfProperties.TLS.certificateFile                  = toStdString(details[ConfProperties::TLS::CERTIFICATE_FILE]);
    ConfProperties.TLS.privateKeyFile                   = toStdString(details[ConfProperties::TLS::PRIVATE_KEY_FILE]);
    ConfProperties.TLS.password                         = toStdString(details[ConfProperties::TLS::PASSWORD]);
    ConfProperties.TLS.method                           = toStdString(details[ConfProperties::TLS::METHOD]);
    ConfProperties.TLS.ciphers                          = toStdString(details[ConfProperties::TLS::CIPHERS]);
    ConfProperties.TLS.serverName                       = toStdString(details[ConfProperties::TLS::SERVER_NAME]);
    ConfProperties.TLS.verifyServer                     = toBool(details[ConfProperties::TLS::VERIFY_SERVER]);
    ConfProperties.TLS.verifyClient                     = toBool(details[ConfProperties::TLS::VERIFY_CLIENT]);
    ConfProperties.TLS.requireClientCertificate         = toBool(details[ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE]);
    ConfProperties.TLS.negotiationTimeoutSec            = toInt(details[ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC]);
    // DHT
    ConfProperties.DHT.port                             = toInt(details[ConfProperties::DHT::PORT]);
    ConfProperties.DHT.PublicInCalls                    = toBool(details[ConfProperties::DHT::PUBLIC_IN_CALLS]);
    ConfProperties.DHT.AllowFromTrusted                 = toBool(details[ConfProperties::DHT::ALLOW_FROM_TRUSTED]);
    // RingNS
    ConfProperties.RingNS.uri                           = toStdString(details[ConfProperties::RingNS::URI]);
    ConfProperties.RingNS.account                       = toStdString(details[ConfProperties::RingNS::ACCOUNT]);
}

MapStringString
account::Info::toDetails()
{
    using namespace DRing::Account;
    MapStringString details;
    // General
    details[ConfProperties::ID]                         = toQString(id);
    details[ConfProperties::TYPE]                       = profileInfo.type == profile::Type::RING ? QString(ProtocolNames::RING) : QString(ProtocolNames::SIP);
    details[ConfProperties::ALIAS]                      = toQString(profileInfo.alias);
    details[ConfProperties::DISPLAYNAME]                = toQString(ConfProperties.displayName);
    details[ConfProperties::ENABLED]                    = toQString(enabled);
    details[ConfProperties::MAILBOX]                    = toQString(ConfProperties.mailbox);
    details[ConfProperties::DTMF_TYPE]                  = toQString(ConfProperties.dtmfType);
    details[ConfProperties::AUTOANSWER]                 = toQString(ConfProperties.autoAnswer);
    details[ConfProperties::ACTIVE_CALL_LIMIT]          = toQString(ConfProperties.activeCallLimit);
    details[ConfProperties::HOSTNAME]                   = toQString(ConfProperties.hostname);
    details[ConfProperties::USERNAME]                   = toQString(profileInfo.uri).prepend((profileInfo.type == profile::Type::RING) ? "ring:" : "");
    details[ConfProperties::ROUTE]                      = toQString(ConfProperties.routeset);
    details[ConfProperties::PASSWORD]                   = toQString(ConfProperties.password);
    details[ConfProperties::REALM]                      = toQString(ConfProperties.realm);
    details[ConfProperties::LOCAL_INTERFACE]            = toQString(ConfProperties.localInterface);
    details[ConfProperties::PUBLISHED_SAMEAS_LOCAL]     = toQString(ConfProperties.publishedSameAsLocal);
    details[ConfProperties::LOCAL_PORT]                 = toQString(ConfProperties.localPort);
    details[ConfProperties::PUBLISHED_PORT]             = toQString(ConfProperties.publishedPort);
    details[ConfProperties::PUBLISHED_ADDRESS]          = toQString(ConfProperties.publishedAddress);
    details[ConfProperties::USER_AGENT]                 = toQString(ConfProperties.userAgent);
    details[ConfProperties::UPNP_ENABLED]               = toQString(ConfProperties.upnpEnabled);
    details[ConfProperties::HAS_CUSTOM_USER_AGENT]      = toQString(ConfProperties.hasCustomUserAgent);
    details[ConfProperties::ALLOW_CERT_FROM_HISTORY]    = toQString(ConfProperties.allowIncomingFromHistory);
    details[ConfProperties::ALLOW_CERT_FROM_CONTACT]    = toQString(ConfProperties.allowIncomingFromContact);
    details[ConfProperties::ALLOW_CERT_FROM_TRUSTED]    = toQString(ConfProperties.allowIncomingFromTrusted);
    details[ConfProperties::ARCHIVE_PASSWORD]           = toQString(ConfProperties.archivePassword);
    details[ConfProperties::ARCHIVE_HAS_PASSWORD]       = toQString(ConfProperties.archiveHasPassword);
    details[ConfProperties::ARCHIVE_PATH]               = toQString(ConfProperties.archivePath);
    details[ConfProperties::ARCHIVE_PIN]                = toQString(ConfProperties.archivePin);
    details[ConfProperties::RING_DEVICE_ID]             = toQString(ConfProperties.deviceID);
    details[ConfProperties::RING_DEVICE_NAME]           = toQString(ConfProperties.deviceName);
    details[ConfProperties::PROXY_ENABLED]              = toQString(ConfProperties.proxyEnabled);
    details[ConfProperties::PROXY_SERVER]               = toQString(ConfProperties.proxyServer);
    details[ConfProperties::PROXY_PUSH_TOKEN]           = toQString(ConfProperties.proxyPushToken);
    // Audio
    details[ConfProperties::Audio::PORT_MAX]            = toQString(ConfProperties.Audio.audioPortMax);
    details[ConfProperties::Audio::PORT_MIN]            = toQString(ConfProperties.Audio.audioPortMin);
    // Video
    details[ConfProperties::Video::ENABLED]             = toQString(ConfProperties.Video.videoEnabled);
    details[ConfProperties::Video::PORT_MAX]            = toQString(ConfProperties.Video.videoPortMax);
    details[ConfProperties::Video::PORT_MIN]            = toQString(ConfProperties.Video.videoPortMin);
    // STUN
    details[ConfProperties::STUN::SERVER]               = toQString(ConfProperties.STUN.server);
    details[ConfProperties::STUN::ENABLED]              = toQString(ConfProperties.STUN.enable);
    // TURN
    details[ConfProperties::TURN::SERVER]               = toQString(ConfProperties.TURN.server);
    details[ConfProperties::TURN::ENABLED]              = toQString(ConfProperties.TURN.enable);
    details[ConfProperties::TURN::SERVER_UNAME]         = toQString(ConfProperties.TURN.username);
    details[ConfProperties::TURN::SERVER_PWD]           = toQString(ConfProperties.TURN.password);
    details[ConfProperties::TURN::SERVER_REALM]         = toQString(ConfProperties.TURN.realm);
    // Presence
    details[ConfProperties::Presence::SUPPORT_PUBLISH]  = toQString(ConfProperties.Presence.presencePublishSupported);
    details[ConfProperties::Presence::SUPPORT_SUBSCRIBE] = toQString(ConfProperties.Presence.presenceSubscribeSupported);
    details[ConfProperties::Presence::ENABLED]          = toQString(ConfProperties.Presence.presenceEnabled);
    // Ringtone
    details[ConfProperties::Ringtone::PATH]             = toQString(ConfProperties.Ringtone.ringtonePath);
    details[ConfProperties::Ringtone::ENABLED]          = toQString(ConfProperties.Ringtone.ringtoneEnabled);
    // SRTP
    details[ConfProperties::SRTP::KEY_EXCHANGE]         = toQString(ConfProperties.SRTP.keyExchange);
    details[ConfProperties::SRTP::ENABLED]              = toQString(ConfProperties.SRTP.enable);
    details[ConfProperties::SRTP::RTP_FALLBACK]         = toQString(ConfProperties.SRTP.rtpFallback);
    // TLS
    details[ConfProperties::TLS::LISTENER_PORT]         = toQString(ConfProperties.TLS.listenerPort);
    details[ConfProperties::TLS::ENABLED]               = toQString(ConfProperties.TLS.enable);
    details[ConfProperties::TLS::PORT]                  = toQString(ConfProperties.TLS.port);
    details[ConfProperties::TLS::CA_LIST_FILE]          = toQString(ConfProperties.TLS.certificateListFile);
    details[ConfProperties::TLS::CERTIFICATE_FILE]      = toQString(ConfProperties.TLS.certificateFile);
    details[ConfProperties::TLS::PRIVATE_KEY_FILE]      = toQString(ConfProperties.TLS.privateKeyFile);
    details[ConfProperties::TLS::PASSWORD]              = toQString(ConfProperties.TLS.password);
    details[ConfProperties::TLS::METHOD]                = toQString(ConfProperties.TLS.method);
    details[ConfProperties::TLS::CIPHERS]               = toQString(ConfProperties.TLS.ciphers);
    details[ConfProperties::TLS::SERVER_NAME]           = toQString(ConfProperties.TLS.serverName);
    details[ConfProperties::TLS::VERIFY_SERVER]         = toQString(ConfProperties.TLS.verifyServer);
    details[ConfProperties::TLS::VERIFY_CLIENT]         = toQString(ConfProperties.TLS.verifyClient);
    details[ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE] = toQString(ConfProperties.TLS.requireClientCertificate);
    details[ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC] = toQString(ConfProperties.TLS.negotiationTimeoutSec);
    // DHT
    details[ConfProperties::DHT::PORT]                  = toQString(ConfProperties.DHT.port);
    details[ConfProperties::DHT::PUBLIC_IN_CALLS]       = toQString(ConfProperties.DHT.PublicInCalls);
    details[ConfProperties::DHT::ALLOW_FROM_TRUSTED]    = toQString(ConfProperties.DHT.AllowFromTrusted);
    // RingNS
    details[ConfProperties::RingNS::URI]                = toQString(ConfProperties.RingNS.uri);
    details[ConfProperties::RingNS::ACCOUNT]            = toQString(ConfProperties.RingNS.account);

    return details;
}

} // namespace lrc

#include "api/moc_newaccountmodel.cpp"
#include "newaccountmodel.moc"

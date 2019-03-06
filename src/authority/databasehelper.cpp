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
#include "databasehelper.h"

#include "api/profile.h"
#include "api/datatransfer.h"
#include "uri.h"
#include "vcard.h"

#include <account_const.h>
#include <datatransfer_interface.h>

#include <QImage>
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>

#include <fstream>

namespace lrc
{

namespace authority
{

namespace storage
{

std::vector<std::string>
getConversationsWithPeer(Database& db, const std::string& participant_uri)
{
    return db.select("id",
                     "conversations",
                     "participant=:participant",
                     {{":participant", participant_uri}}).payloads;
}

std::vector<std::string>
getPeerParticipantsForConversation(Database& db, const std::string& conversationId)
{
    return db.select("participant",
                     "conversations",
                     "id=:id",
                     { {":id", conversationId} }).payloads;
}

void
setProfileForPeer(const std::string& profile_uri, const api::contact::Info& profile)
{
    qWarning() << "Not implemented";
    return;
}

api::contact::Info
buildContactFromProfile(const std::string& profile_uri)
{
    qWarning() << "Not implemented";
    return {};
}

std::vector<std::string>
getConversationsBetween(Database& db, const std::string& peer1_uri, const std::string& peer2_uri)
{
    auto conversationsForPeer1 = getConversationsWithPeer(db, peer1_uri);
    std::sort(conversationsForPeer1.begin(), conversationsForPeer1.end());
    auto conversationsForPeer2 = getConversationsWithPeer(db, peer2_uri);
    std::sort(conversationsForPeer2.begin(), conversationsForPeer2.end());
    std::vector<std::string> common;

    std::set_intersection(conversationsForPeer1.begin(), conversationsForPeer1.end(),
                          conversationsForPeer2.begin(), conversationsForPeer2.end(),
                          std::back_inserter(common));
    return common;
}

std::string
beginConversationWithPeer(Database& db, const std::string& peer_uri, const std::string& firstMessage)
{
    // Add conversation between account and profile
    auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1",
                                        "conversations",
                                        "1=1",
                                        {}).payloads[0];
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant", "participant"}},
                  {{":id", newConversationsId}, {":participant", peer_uri}});
    // Add first interaction
    if (!firstMessage.empty())
        try {
            db.insertInto("interactions", {
                    {":author", "author"},
                    {":conversation", "conversation"},
                    {":timestamp", "timestamp"},
                    {":body", "body"},
                    {":type", "type"},
                    {":status", "status"}
                }, {
                    {":author", peer_uri},
                    {":conversation", newConversationsId},
                    {":timestamp", std::to_string(std::time(nullptr))},
                    {":body", firstMessage},
                    {":type", "CONTACT"},
                    {":status", "SUCCEED"}
                });
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
        }
    return newConversationsId;
}

void
getHistory(Database& db, api::conversation::Info& conversation)
{
    auto interactionsResult = db.select("id, author, body, timestamp, duration, type, status",
                                        "interactions",
                                        "conversation=:conversation",
                                        {{":conversation_id", conversation.uid}});
    auto nCols = 7;
    if (interactionsResult.nbrOfCols == nCols) {
        auto payloads = interactionsResult.payloads;
        for (decltype(payloads.size()) i = 0; i < payloads.size(); i += nCols) {
            auto msg = api::interaction::Info({payloads[i + 1], payloads[i + 2],
                                         std::stoi(payloads[i + 3]),
                                         std::stoi(payloads[i + 4]),
                                         api::interaction::to_type(payloads[i + 5]),
                                         api::interaction::to_status(payloads[i + 6])});
            conversation.interactions.emplace(std::stoull(payloads[i]), std::move(msg));
            conversation.lastMessageUid = std::stoull(payloads[i]);
        }
    }
}

int
addMessageToConversation(Database& db,
                         const std::string& conversationId,
                         const api::interaction::Info& msg)
{
    int result = -1;
    try {
        result = db.insertInto("interactions", {
                {":author", "author"},
                {":conversation", "conversation"},
                {":timestamp", "timestamp"},
                {":body", "body"},
                {":type", "type"},
                {":status", "status"}
            }, {
                {":author", msg.authorUri},
                {":conversation", conversationId},
                {":timestamp", std::to_string(msg.timestamp)},
                {":body", msg.body},
                {":type", to_string(msg.type)},
                {":status", to_string(msg.status)}
            });
    } catch (const std::runtime_error& e) {
        qWarning() << e.what();
    }
    return result;
}

int
addOrUpdateMessage(Database& db,
                   const std::string& conversationId,
                   const api::interaction::Info& msg,
                   const std::string& daemonId)
{
    // Check if profile is already present.
    auto msgAlreadyExists = db.select("id",
                                      "interactions",
                                      "author=:author AND daemon_id=:daemon_id",
                                      { {":author", msg.authorUri},
                                      { ":daemon_id", daemonId } }).payloads;
    if (msgAlreadyExists.empty()) {
        int result = -1;
        try {
            result = db.insertInto("interactions", {
                    {":author", "author"},
                    {":conversation", "conversation"},
                    {":timestamp", "timestamp"},
                    {":body", "body"},
                    {":type", "type"},
                    {":status", "status"},
                    {":daemon_id", "daemon_id"}
                }, {
                    {":author", msg.authorUri},
                    {":conversation_id", conversationId},
                    {":timestamp", std::to_string(msg.timestamp)},
                    {":body", msg.body}, {":type", to_string(msg.type)}, {":daemon_id", daemonId},
                    {":status", to_string(msg.status)}
                });
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
        }
        return result;
    } else {
        // already exists @ id(msgAlreadyExists[0])
        auto id = msgAlreadyExists[0];
        try {
            db.update("interactions",
                      "body=:body",
                      {{":body", msg.body}},
                      "id=:id",
                      { {":id", id} });
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
        }
        return std::stoi(id);
    }

}
int
addDataTransferToConversation(Database& db,
                              const std::string& conversationId,
                              const api::datatransfer::Info& infoFromDaemon)
{
    int result = -1;
    try {
        result = db.insertInto("interactions", {
                {":author", "author"},
                {":conversation", "conversation"},
                {":timestamp", "timestamp"},
                {":body", "body"},
                {":type", "type"},
                {":status", "status"}
            }, {
                {infoFromDaemon.isOutgoing ? "" : ":author_id", infoFromDaemon.peerUri},
                {":conversation", conversationId},
                {":timestamp", std::to_string(std::time(nullptr))},
                {":body", infoFromDaemon.path},
                {":type", infoFromDaemon.isOutgoing ?
                        "OUTGOING_DATA_TRANSFER" :
                        "INCOMING_DATA_TRANSFER"},
                {":status", "TRANSFER_CREATED"}
            });
    } catch (const std::runtime_error& e) {
        qWarning() << e.what();
    }
    return result;
}

void addDaemonMsgId(Database& db,
                    const std::string& interactionId,
                    const std::string& daemonId)
{
    db.update("interactions",
              "daemon_id=:daemon_id",
              {{":daemon_id", daemonId}},
              "id=:id", {{":id", interactionId}});
}

std::string getDaemonIdByInteractionId(Database& db, const std::string& id)
{
    auto ids = db.select("daemon_id",
                         "interactions",
                         "id=:id",
                         {{":id", id}}).payloads;
    return ids.empty() ? "" : ids[0];
}

std::string getInteractionIdByDaemonId(Database& db, const std::string& id)
{
    auto ids = db.select("id",
                         "interactions",
                         "daemon_id=:daemon_id",
                         {{":daemon_id", id}}).payloads;
    return ids.empty() ? "" : ids[0];
}

void updateInteractionBody(Database& db, unsigned int id,
                           const std::string& newBody)
{
    db.update("interactions", "body=:body",
              {{":body", newBody}},
              "id=:id", {{":id", std::to_string(id)}});
}

void updateInteractionStatus(Database& db, unsigned int id,
                             api::interaction::Status newStatus)
{
    db.update("interactions", "status=:status",
              {{":status", api::interaction::to_string(newStatus)}},
              "id=:id", {{":id", std::to_string(id)}});
}

std::string
conversationIdFromInteractionId(Database& db, unsigned int interactionId)
{
    auto result = db.select("conversation_id",
                            "interactions",
                            "id=:interaction_id",
                            {{":interaction_id", std::to_string(interactionId)}});
    if (result.nbrOfCols == 1 && result.payloads.size()) {
        return result.payloads[0];
    }
    return {};
}

void clearHistory(Database& db,
                  const std::string& conversationId)
{
    db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
}

void clearInteractionFromConversation(Database& db,
                                      const std::string& conversationId,
                                      const uint64_t& interactionId)
{
    db.deleteFrom("interactions", "conversation_id=:conv_id AND id=:int_id",
                 {{":conv_id", conversationId}, {":int_id", std::to_string(interactionId)}});
}

void clearAllHistory(Database& db)
{
    db.truncateTable("interactions");
}

void
deleteObsoleteHistory(Database& db, long int date)
{
    db.deleteFrom("interactions", "timestamp<=:date", { {":date", std::to_string(date)} });
}

void
removeContact(Database& db, const std::string& contactUri)
{
    // Get common conversations
    auto conversations = getConversationsWithPeer(db, contactUri);
    // Remove conversations + interactions
    for (const auto& conversationId: conversations) {
        // Remove conversation
        db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
        // clear History
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
    }
}

void
removeAccount(const std::string& accountId)
{
    // TODO(atraczyk): is this required ?
    // remove all files in {accountId} ?
}

void
addContact(Database& db, const std::string& contactUri)
{
    // Get if conversation exists
    auto result = getConversationsWithPeer(db, contactUri);
    if (result.empty()) {
        // conversations doesn't exists, start it.
        beginConversationWithPeer(db, contactUri);
    }
}

int
countUnreadFromInteractions(Database& db, const std::string& conversationId)
{
    return db.count("status", "interactions", "status=:status AND conversation_id=:id",
           {{":status", "UNREAD"}, {":id", conversationId}});
}

uint64_t
getLastTimestamp(Database& db)
{
    auto timestamps = db.select("MAX(timestamp)", "interactions", "1=1", {}).payloads;
    auto result = std::time(nullptr);
    try {
        if (!timestamps.empty() && !timestamps[0].empty()) {
            result = std::stoull(timestamps[0]);
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "storage::getLastTimestamp, stoull throws an out_of_range exception: " << e.what();
    } catch (const std::invalid_argument& e) {
        qDebug() << "storage::getLastTimestamp, stoull throws an invalid_argument exception: " << e.what();
    }
    return result;
}

namespace {
QString JSONStringFromInitList(std::initializer_list<QPair<QString, QJsonValue>> args)
{
    QJsonObject jsonObject(args);
    QJsonDocument doc(jsonObject);
    return QString::fromLocal8Bit(doc.toJson(QJsonDocument::Compact));
}
QString
readJSONValue(const QJsonObject& json, const QString& key)
{
    if (json.contains(key) && json[key].isString()) {
        return json[key].toString();
    }
}

void
writeJSONValue(QJsonObject& json, const QString& key, const QString& value)
{
    json[key] = value;
}
}

//================================================================================
// This section provides migration helpers from ring.db
// to per-account databases yielding a file structure like:
//
// { local_storage } / jami
// └──{ account_id }
// ├── config.yml
// ├── contacts
// ├── export.gz
// ├── incomingTrustRequests
// ├── knownDevicesNames
// ├── history.db < --conversations and interactions database
//     ├── profile.vcf < --account vcard
//     ├── profiles < --account contact vcards
//     │   │──{ contact_uri }.vcf
//     │   └── ...
//     ├── ring_device.crt
//     └── ring_device.key
//================================================================================
namespace migration {

std::string
profileToVcard(const api::profile::Info& profileInfo,
    const std::string& accountId = {})
{
    using namespace api;
    // TODO: check if jpeg or png
    bool compressedImage = false;
    std::string vCardStr = vCard::Delimiter::BEGIN_TOKEN;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::VERSION;
    vCardStr += ":2.1";
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (!accountId.empty()) {
        vCardStr += vCard::Property::UID;
        vCardStr += ":";
        vCardStr += accountId;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    }
    vCardStr += vCard::Property::FORMATTED_NAME;
    vCardStr += ":";
    vCardStr += profileInfo.alias;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (profileInfo.type == profile::Type::RING) {
        vCardStr += vCard::Property::TELEPHONE;
        vCardStr += ":";
        vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
        vCardStr += "other:ring:";
        vCardStr += profileInfo.uri;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    } else {
        vCardStr += vCard::Property::TELEPHONE;
        vCardStr += profileInfo.uri;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    }
    vCardStr += vCard::Property::PHOTO;
    vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
    vCardStr += "ENCODING=BASE64";
    vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
    vCardStr += compressedImage ? "TYPE=JPEG:" : "TYPE=PNG:";
    vCardStr += profileInfo.avatar;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Delimiter::END_TOKEN;
    return vCardStr;
}

uint64_t
getTimeFromTimeStr(const std::string& str) noexcept
{
    uint64_t minutes = 0, seconds = 0;
    std::size_t delimiterPos = str.find(":");
    if (delimiterPos != std::string::npos) {
        try {
            minutes = std::stoull(str.substr(0, delimiterPos));
            seconds = std::stoull(str.substr(delimiterPos + 1));
        } catch (const std::exception&) {
            return 0;
        }
    }
    return minutes * 60 + seconds;
}

namespace legacy_enum {

static inline api::interaction::Type
to_type(const std::string& type)
{
    if (type == "TEXT")
        return Type::TEXT;
    else if (type == "CALL")
        return Type::CALL;
    else if (type == "CONTACT")
        return Type::CONTACT;
    else if (type == "OUTGOING_DATA_TRANSFER")
        return Type::OUTGOING_DATA_TRANSFER;
    else if (type == "INCOMING_DATA_TRANSFER")
        return Type::INCOMING_DATA_TRANSFER;
    else
        return Type::INVALID;
}

} // namespace legacy_enum

std::pair<std::string, uint64_t>
migrateMessageBody(const std::string& body, api::interaction::Type type)
{
    uint64_t duration{ 0 };
    // check in english and local to determine the direction of the call
    switch (type) {
    case api::interaction::Type::CALL:
        {
            bool en_missedInc   = body.find("Missed incoming call") != std::string::npos;
            bool en_inc         = body.find("Incoming call") != std::string::npos;
            bool loc_missedInc  = body.find(QObject::tr("Missed incoming call").toStdString()) != std::string::npos;
            bool loc_inc        = body.find(QObject::tr("Incoming call").toStdString()) != std::string::npos;
            bool incomingCall   = en_missedInc || en_inc || loc_missedInc || loc_inc;
            std::size_t dashPos = body.find("-");
            bool missedCall = dashPos == std::string::npos;
            if (!missedCall) {
                duration = getTimeFromTimeStr(body.substr(dashPos + 2));
            }
            return std::make_pair(std::to_string(incomingCall + 2 * missedCall), duration);
        }
        break;
    case api::interaction::Type::CONTACT:
        {
            if (body.find("Contact added") != std::string::npos ||
                body.find(QObject::tr("Contact added").toStdString()) != std::string::npos) {
                return std::make_pair(std::to_string((int)GeneratedMessageType::CONTACT_ADDED), 0);
            } else if (body.find("Invitation received") != std::string::npos ||
                       body.find(QObject::tr("Invitation received").toStdString()) != std::string::npos) {
                return std::make_pair(std::to_string((int)GeneratedMessageType::INVITATION_RECEIVED), 0);
            } else if (body.find("Invitation accepted") != std::string::npos ||
                       body.find(QObject::tr("Invitation accepted").toStdString()) != std::string::npos) {
                return std::make_pair(std::to_string((int)GeneratedMessageType::INVITATION_ACCEPTED), 0);
            }
        }
        break;
    default:
        return std::make_pair(body, 0);
    }
}

std::vector<std::string>
getPeerParticipantsForConversationId(Database& db, const std::string& profileId, const std::string& conversationId)
{
    return db.select("participant_id",
        "conversations",
        "id=:id AND participant_id!=:participant_id",
        { {":id", conversationId}, {":participant_id", profileId} }).payloads;
}

} // namespace migration

std::vector<std::shared_ptr<Database>>
migrateLegacyDatabaseIfNeeded(const QStringList& accountIds)
{
    using namespace lrc::api;
    using namespace migration;

    std::vector<std::shared_ptr<Database>> dbs(accountIds.size());

    if (!dbs.size()) {
        qDebug() << "No accounts to migrate";
        return {};
    }

    std::shared_ptr<Database> legacyDb;
    try {
        // create function should throw if there's no migration material
        legacyDb = lrc::DatabaseFactory::create<LegacyDatabase>();
    } catch (const std::runtime_error& e) {
        qDebug() << e.what();
        return dbs;
    }

    if (legacyDb == nullptr) {
        return dbs;
    }

    int index = 0;
    qDebug() << "Migrating...";
    for (auto accountId : accountIds) {
        qDebug() << "Migrating account: " << accountId << "...";
        try {
            auto dbName = accountId.toStdString() + "/jami";
            dbs.at(index) = lrc::DatabaseFactory::create<Database>(dbName);
            auto& db = *dbs.at(index++);

            auto accountLocalPath = Database::getPath() + "/" + accountId + "/";

            using namespace DRing::Account;
            MapStringString accountDetails = ConfigurationManager::instance().
                getAccountDetails(accountId.toStdString().c_str());
            bool isRingAccount = accountDetails[ConfProperties::TYPE] == "RING";
            std::map<std::string, std::string> profileIdUriMap;
            std::string accountProfileId;

            // 1. profiles_accounts
            // migrate account's avatar/alias from profiles table to {data_dir}/profile.vcf
            std::string accountUri;
            if (isRingAccount) {
                accountUri = accountDetails[DRing::Account::ConfProperties::USERNAME].contains("ring:") ?
                    accountDetails[DRing::Account::ConfProperties::USERNAME].toStdString().substr(std::string("ring:").size()) :
                    accountDetails[DRing::Account::ConfProperties::USERNAME].toStdString();
            } else {
                auto uri = URI(accountDetails[DRing::Account::ConfProperties::USERNAME]);
                uri.setSchemeType(URI::SchemeType::SIP);
                !uri.hasHostname() ? uri.setHostname(accountDetails[ConfProperties::HOSTNAME]) : (void)0;
                !uri.hasPort() ? uri.setPort(accountDetails[ConfProperties::PUBLISHED_PORT]) : (void)0;
                accountUri = uri.full().toStdString();
            }

            auto accountProfileIds = legacyDb->select(
                "profile_id", "profiles_accounts",
                "account_id=:account_id AND \
                 is_account=:is_account",
                { {":account_id", accountId.toStdString()},
                {":is_account", "true"} }).payloads;
            if (accountProfileIds.empty()) {
                continue;
            }
            accountProfileId = accountProfileIds[0];
            auto accountProfile = legacyDb->select(
                "photo, alias",
                "profiles", "uri=:uri",
                { {":uri", accountUri} }).payloads;
            profile::Info accountProfileInfo;
            // if we can not find the uri in the database
            // (in the case of poorly kept SIP account uris),
            // than we cannot migrate the conversations and vcard
            if (!accountProfile.empty()) {
                accountProfileInfo = { accountUri, accountProfile[0], accountProfile[1],
                    isRingAccount ? profile::Type::RING : profile::Type::SIP };
            }

            auto accountVcard = profileToVcard(accountProfileInfo, accountId.toStdString());
            auto profileFilePath = accountLocalPath + "profile" + ".vcf";
            QFile file(profileFilePath);
            if (!file.open(QIODevice::WriteOnly)) {
                throw std::runtime_error("Can't open file: " + profileFilePath.toStdString());
            }
            QTextStream(&file) << QString::fromStdString(accountVcard);

            // 2. profiles
            // migrate profiles from profiles table to {data_dir}/{uri}.vcf
            // - a partial uri with no scheme and no port will be used as
            //   the filename
            // - for SIP, if the hostname is not provided, the account's
            //   hostname will be added
            // - for JAMI, the hostname is omitted
            // e.g. 3d1112ab2bb089370c0744a44bbbb0786418d40b.vcf
            //      username@hostname.vcf

            // only select non-account profiles
            auto profileIds = legacyDb->select(
                "profile_id", "profiles_accounts",
                "account_id=:account_id AND \
                 is_account=:is_account",
                {{":account_id", accountId.toStdString()},
                {":is_account", "false"}}).payloads;
            for (const auto& profileId : profileIds) {
                auto profile = legacyDb->select(
                    "uri, alias, photo, type", "profiles",
                    "id=:id",
                    { {":id", profileId} }).payloads;
                if (profile.empty()) {
                    continue;
                }
                profile::Info profileInfo{ profile[0], profile[2], profile[1] };
                auto uri = URI(QString::fromStdString(profile[0]));
                auto uriScheme = uri.schemeType();
                if (uri.schemeType() == URI::SchemeType::NONE) {
                    // if uri has no scheme, default to current account scheme
                    // this is only for vcard generation
                    uri.setSchemeType(isRingAccount ? URI::SchemeType::RING : URI::SchemeType::SIP);
                    profileInfo.type = isRingAccount ? profile::Type::RING : profile::Type::SIP;
                } else {
                    profileInfo.type = uri.schemeType() == URI::SchemeType::RING ?
                        profile::Type::RING :
                        profile::Type::SIP;
                }
                // if the account is a SIP account, set the uri's hostname as the account's
                // if it doesn't already have one
                if (!isRingAccount) {
                    !uri.hasHostname() ? uri.setHostname(accountDetails[ConfProperties::HOSTNAME]) : (void)0;
                    !uri.hasPort() ? uri.setPort(accountDetails[ConfProperties::PUBLISHED_PORT]) : (void)0;
                }
                auto profileUri = uri.userinfo();
                if (!isRingAccount) {
                    profileUri += "@" + uri.hostname();
                }
                // insert into map for use during the conversations table migration
                // TODO: use uri.full() instead of profileUri ?
                profileIdUriMap.insert(std::make_pair(profileId, profileUri.toStdString()));
                auto vcard = profileToVcard(profileInfo);
                // make sure the directory exists
                QDir dir(accountLocalPath + "profiles");
                if (!dir.exists())
                    dir.mkpath(".");
                profileFilePath = accountLocalPath + "profiles/" + profileUri + ".vcf";
                QFile file(profileFilePath);
                // if we catch duplicates here, skip thh profile because
                // the previous db structure does not guarantee unique uris
                if (file.exists()) {
                    qWarning() << "Profile file already exits : " << profileFilePath;
                    continue;
                }
                if(!file.open(QIODevice::WriteOnly)) {
                    qWarning() << "Can't open file : " << profileFilePath;
                    continue;
                }
                QTextStream(&file) << QString::fromStdString(vcard);
            }

            // 3. conversations
            // migrate old conversations table ==> new conversations table
            // a) participant_id INTEGER becomes participant TEXT (the uri of the participant)
            //    use the selected non-account profiles
            auto conversationIds = legacyDb->select(
                "id", "conversations",
                "participant_id=:participant_id",
                { {":participant_id", accountProfileId} }).payloads;
            if (conversationIds.empty()) {
                continue;
            }
            for (auto conversationId : conversationIds) {
                // only one peer pre-groupchat
                auto peerProfileId = getPeerParticipantsForConversationId(*legacyDb, accountProfileId, conversationId);
                if (peerProfileId.empty()) {
                    continue;
                }
                auto it = profileIdUriMap.find(peerProfileId.at(0));
                // we cannot insert in the conversations table without a uri
                if (it == profileIdUriMap.end()) {
                    continue;
                }
                try {
                    db.insertInto("conversations",
                        {   {":id", "id"} ,
                            {":participant", "participant"} },
                        {   { ":id", conversationId } ,
                            { ":participant", it->second } });
                } catch (const std::runtime_error& e) {
                    qWarning() << e.what();
                }
            }

            // 4. interactions
            auto allInteractions = legacyDb->select(
                "account_id, author_id, conversation_id, \
                 timestamp, body, type, status, daemon_id",
                "interactions",
                "account_id=:account_id",
                { {":account_id", accountProfileId} });
            auto interactionIt = allInteractions.payloads.begin();
            while (interactionIt != allInteractions.payloads.end()) {
                auto type = api::interaction::to_type(*(interactionIt + 5));
                auto author_id = *(interactionIt + 1);
                auto it = profileIdUriMap.find(author_id);
                if (it == profileIdUriMap.end() && author_id != accountProfileId) {
                    std::advance(interactionIt, allInteractions.nbrOfCols);
                    continue;
                }
                auto migratedMsg = migrateMessageBody(*(interactionIt + 4), type);
                auto daemonId = *(interactionIt + 7);
                auto profileUri = it == profileIdUriMap.end() ? "" : it->second;
                std::string extra_data = migratedMsg.second == 0 ? "" :
                    JSONStringFromInitList(
                        { qMakePair("duration", migratedMsg.second) })
                        .toStdString();
                try {
                    db.insertInto("interactions", {
                        {":author", "author"},
                        {":conversation", "conversation"},
                        {":timestamp", "timestamp"},
                        {":body", "body"},
                        {":type", "type"},
                        {":status", "status"},
                        {":daemon_id", "daemon_id"},
                        {":extra_data", "extra_data"}
                    }, {
                        {profileUri.empty() ? "" : ":author", profileUri},
                        {":conversation", *(interactionIt + 2)},
                        {":timestamp", *(interactionIt + 3)},
                        {":body", migratedMsg.first},
                        {":type", *(interactionIt + 5)},
                        {":status", *(interactionIt + 6)},
                        {daemonId.empty() ? "" : ":daemon_id", daemonId},
                        {extra_data.empty() ? "" : ":extra_data", extra_data }
                    });
                } catch (const std::runtime_error& e) {
                    qWarning() << e.what();
                }
                std::advance(interactionIt, allInteractions.nbrOfCols);
            }
            qDebug() << "Done";
        } catch (const std::runtime_error& e) {
            qWarning().noquote()
                << "Could not migrate database for account: "
                << accountId << "\n " << e.what();
        }
    }

    return dbs;
}

} // namespace database

} // namespace authority

} // namespace lrc

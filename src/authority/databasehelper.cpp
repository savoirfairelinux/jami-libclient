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

QString getPath()
{
    QDir dataDir(QStandardPaths::writableLocation(
        QStandardPaths::AppLocalDataLocation));
    // Avoid to depends on the client name.
    dataDir.cdUp();
    return dataDir.absolutePath() + "/jami/";
}

std::string
prepareUri(const std::string& uri, api::profile::Type type)
{
    URI uriObject(QString::fromStdString(uri));
    switch (type) {
    case api::profile::Type::SIP:
        return uriObject.format(URI::Section::USER_INFO | URI::Section::HOSTNAME)
            .toStdString();
        break;
    case api::profile::Type::RING:
        return uriObject.format(URI::Section::USER_INFO)
            .toStdString();
        break;
    default:
        return uri;
    }
}

std::string
getFormattedCallString(const std::string& author_uri,
                       const std::time_t duration)
{
    if (duration < 0) {
        return QObject::tr("📞 Ongoing call").toStdString();
    } else if (author_uri.empty()) {
        if (duration) {
            return QObject::tr("📞 Outgoing call - ").toStdString()
                + getFormattedCallDuration(duration);
        } else {
            return QObject::tr("🕽 Missed outgoing call").toStdString();
        }
    } else {
        if (duration) {
            return QObject::tr("📞 Incoming call - ").toStdString()
                + getFormattedCallDuration(duration);
        } else {
            return QObject::tr("🕽 Missed incoming call").toStdString();
        }
    }
}

namespace vcard
{
std::string
compressedAvatar(const std::string& image)
{
    QImage qimage;
    const bool ret = qimage.loadFromData(QByteArray::fromBase64(image.c_str()), 0);
    if (!ret) {
        qDebug() << "vCard image loading failed";
        return image;
    }
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    qimage.scaled({ 128,128 }).save(&buffer, "JPEG", 90);
    auto b64Img = bArray.toBase64().trimmed();
    return std::string(b64Img.constData(), b64Img.length());
}

std::string
profileToVcard(const api::profile::Info& profileInfo,
               bool compressImage)
{
    using namespace api;
    bool compressedImage = std::strncmp(profileInfo.avatar.c_str(), "/9g=", 4) == 0;
    if (compressedImage && !compressImage) {
        compressImage = false;
    }
    std::string vCardStr = vCard::Delimiter::BEGIN_TOKEN;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::VERSION;
    vCardStr += ":2.1";
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::FORMATTED_NAME;
    vCardStr += ":";
    vCardStr += profileInfo.alias;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (profileInfo.type == profile::Type::RING) {
        vCardStr += vCard::Property::TELEPHONE;
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
    if (compressImage) {
        vCardStr += "TYPE=JPEG:";
        vCardStr += compressedImage ? profileInfo.avatar : compressedAvatar(profileInfo.avatar);
    } else {
        vCardStr += compressedImage ? "TYPE=JPEG:" : "TYPE=PNG:";
        vCardStr += profileInfo.avatar;
    }
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Delimiter::END_TOKEN;
    return vCardStr;
}

void
setProfile(const std::string& accountId,
           const api::profile::Info& profileInfo,
           const bool overwrite,
           const bool isPeer)
{
    auto vcard = vcard::profileToVcard(profileInfo);
    auto accountLocalPath = getPath() + QString::fromStdString(accountId) + "/";
    QString filePath;
    if (isPeer) {
        filePath = accountLocalPath + "profiles/" + QString::fromStdString(profileInfo.uri) + ".vcf";
    } else {
        filePath = accountLocalPath + "profile" + ".vcf";
    }
    QFile file(filePath);
    if (!overwrite && file.exists()) {
        return;
    }
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Can't open file: " << filePath;
        return;
    }
    QTextStream(&file) << QString::fromStdString(vcard);
    return;
}
} // namespace vcard

std::string
getFormattedCallDuration(const std::time_t duration)
{
    if (duration == 0) {
        return "00:00";
    }
    std::string formattedString;
    auto minutes = duration / 60;
    auto seconds = duration % 60;
    if (minutes > 0) {
        formattedString += std::to_string(minutes) + ":";
        if (formattedString.length() == 2) {
            formattedString = "0" + formattedString;
        }
    } else {
        formattedString += "00:";
    }
    if (seconds < 10) formattedString += "0";
    formattedString += std::to_string(seconds);
    return formattedString;
}

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
createProfile(const std::string & accountId,
              const api::profile::Info & profileInfo,
              const bool isPeer)
{
    vcard::setProfile(accountId, profileInfo, false, isPeer);
}

void
updateProfile(const std::string & accountId,
              const api::profile::Info & profileInfo,
              const bool isPeer)
{
    vcard::setProfile(accountId, profileInfo, true, isPeer);
}

api::contact::Info
buildContactFromProfile(const std::string & accountId,
                        const std::string& peer_uri)
{
    lrc::api::profile::Info profileInfo;
    profileInfo.uri = peer_uri;
    auto accountLocalPath = getPath() + QString::fromStdString(accountId) + "/";
    QString filePath;
    filePath = accountLocalPath + "profiles/" + QString::fromStdString(peer_uri) + ".vcf";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open file: " << filePath;
        return { profileInfo, {} };
    }
    QTextStream in(&file);
    QByteArray vcard = in.readAll().toUtf8();
    const auto vCard = lrc::vCard::utils::toHashMap(vcard);
    const auto alias = vCard["FN"];
    const auto photo = (vCard.find("PHOTO;ENCODING=BASE64;TYPE=PNG") == vCard.end()) ?
        vCard["PHOTO;ENCODING=BASE64;TYPE=JPEG"] : vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

    profileInfo.avatar = photo.toStdString();
    profileInfo.alias = alias.toStdString();
    return { profileInfo, {} };
}

std::vector<std::string> getAllConversations(Database & db)
{
    return db.select("id", "conversations", {}, {}).payloads;

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
    if (!firstMessage.empty()) {
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
    }
    return newConversationsId;
}

void
getHistory(Database& db, api::conversation::Info& conversation)
{
    auto interactionsResult = db.select("id, author, body, timestamp, type, status, is_read, extra_data",
                                        "interactions",
                                        "conversation=:conversation",
                                        {{":conversation", conversation.uid}});
    auto nCols = 8;
    if (interactionsResult.nbrOfCols == nCols) {
        auto payloads = interactionsResult.payloads;
        for (decltype(payloads.size()) i = 0; i < payloads.size(); i += nCols) {
            std::string durationString;
            auto extra_data_str = QString::fromStdString(payloads[i + 7]);
            if (!extra_data_str.isEmpty()) {
                durationString = readJSONValue(JSONFromString(extra_data_str), "duration")
                    .toStdString();
            }
            auto body = payloads[i + 2];
            auto type = api::interaction::to_type(payloads[i + 4]);
            std::time_t duration = durationString.empty() ? 0 : std::stoi(durationString);
            if (type == api::interaction::Type::CALL) {
                body = getFormattedCallString(payloads[i + 1], duration);
            }
            auto msg = api::interaction::Info({
                    payloads[i + 1],
                    body,
                    std::stoi(payloads[i + 3]),
                    duration,
                    type,
                    api::interaction::to_status(payloads[i + 5]),
                    (payloads[i + 6] == "1" ? true : false)
                });
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
    return db.insertInto("interactions", {
            { ":author", "author" },
            { ":conversation", "conversation" },
            { ":timestamp", "timestamp" },
            { ":body", "body" },
            { ":type", "type" },
            { ":status", "status" },
            { ":is_read", "is_read" }
        }, {
            { ":author", msg.authorUri},
            { ":conversation", conversationId},
            { ":timestamp", std::to_string(msg.timestamp)},
            { ":body", msg.body},
            { ":type", to_string(msg.type)},
            { ":status", to_string(msg.status)},
            { ":is_read", msg.isRead ? "1" : "0" }
        });
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
        return db.insertInto("interactions", {
                {":author", "author"},
                {":conversation", "conversation"},
                {":timestamp", "timestamp"},
                {":body", "body"},
                {":type", "type"},
                {":status", "status"},
                {":daemon_id", "daemon_id"}
            }, {
                {":author", msg.authorUri.empty() ? "" : msg.authorUri},
                {":conversation", conversationId},
                {":timestamp", std::to_string(msg.timestamp)},
                {msg.body.empty() ? "" : ":body", msg.body},
                {":type", to_string(msg.type)},
                {daemonId.empty() ? "" : ":daemon_id", daemonId},
                {":status", to_string(msg.status)}
            });
    } else {
        // already exists @ id(msgAlreadyExists[0])
        auto id = msgAlreadyExists[0];
        std::string extra_data;
        if (msg.type == api::interaction::Type::CALL) {
            auto duration = std::max(msg.duration, static_cast<std::time_t>(0));
            auto extra_data_str = getInteractionExtraDataById(db, id);
            auto extra_data_JSON = JSONFromString(QString::fromStdString(extra_data_str));
            writeJSONValue(extra_data_JSON, "duration", QString::number(duration));
            extra_data = stringFromJSON(extra_data_JSON).toStdString();
        }
        db.update("interactions",
                  { "body=:body, extra_data=:extra_data" },
                  { {msg.body.empty() ? "" : ":body", msg.body},
                  { extra_data.empty() ? "" : ":extra_data", extra_data } },
                  "id=:id", { {":id", id} });
        return std::stoi(id);
    }

}
int
addDataTransferToConversation(Database& db,
                              const std::string& conversationId,
                              const api::datatransfer::Info& infoFromDaemon)
{
    return db.insertInto("interactions", {
            {":author", "author"},
            {":conversation", "conversation"},
            {":timestamp", "timestamp"},
            {":body", "body"},
            {":type", "type"},
            {":status", "status"},
            {":is_read", "is_read"}
        }, {
            {":author", infoFromDaemon.isOutgoing ? "" : infoFromDaemon.peerUri},
            {":conversation", conversationId},
            {":timestamp", std::to_string(std::time(nullptr))},
            {":body", infoFromDaemon.path},
            {":type", infoFromDaemon.isOutgoing ?
                    "DATA_TRANSFER" :
                    "DATA_TRANSFER"},
            {":status", "TRANSFER_CREATED"},
            {":is_read", "0"}
        });
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

std::string getInteractionIdByDaemonId(Database& db, const std::string& daemon_id)
{
    auto ids = db.select("id",
                         "interactions",
                         "daemon_id=:daemon_id",
                         {{":daemon_id", daemon_id}}).payloads;
    return ids.empty() ? "" : ids[0];
}

std::string getInteractionExtraDataById(Database& db, const std::string& id,
                                        const std::string& key)
{
    auto extra_datas = db.select("extra_data",
                                 "interactions",
                                 "id=:id",
                                 { {":id", id} }).payloads;
    if (key.empty()) {
        return extra_datas.empty() ? "" : extra_datas[0];
    }
    std::string value;
    auto extra_data_str = QString::fromStdString(extra_datas[0]);
    if (!extra_data_str.isEmpty()) {
        value = readJSONValue(JSONFromString(extra_data_str), QString::fromStdString(key))
            .toStdString();
    }
    return value;
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
    db.update("interactions",
              { "status=:status" },
              {{":status", api::interaction::to_string(newStatus)}},
              "id=:id", {{":id", std::to_string(id)}});
}

void setInteractionRead(Database& db, unsigned int id)
{
    db.update("interactions",
              { "is_read=:is_read" },
              { {":is_read", "1"} },
              "id=:id", { {":id", std::to_string(id)} });
}

std::string
conversationIdFromInteractionId(Database& db, unsigned int interactionId)
{
    auto result = db.select("conversation",
                            "interactions",
                            "id=:id",
                            {{":id", std::to_string(interactionId)}});
    if (result.nbrOfCols == 1 && result.payloads.size()) {
        return result.payloads[0];
    }
    return {};
}

void clearHistory(Database& db,
                  const std::string& conversationId)
{
    db.deleteFrom("interactions",
                  "conversation=:conversation",
                  {{":conversation", conversationId}});
}

void clearInteractionFromConversation(Database& db,
                                      const std::string& conversationId,
                                      const uint64_t& interactionId)
{
    db.deleteFrom("interactions",
                  "conversation=:conversation AND id=:id",
                  {{":conversation", conversationId},
                  {":id", std::to_string(interactionId)}});
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
        db.deleteFrom("interactions", "conversation=:id", {{":id", conversationId}});
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
    return db.count("is_read",
                    "interactions",
                    "is_read=:is_read AND conversation=:id",
                    {{":is_read", "0"}, {":id", conversationId}});
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
QString stringFromJSON(const QJsonObject& json)
{
    QJsonDocument doc(json);
    return QString::fromLocal8Bit(doc.toJson(QJsonDocument::Compact));
}

QJsonObject
JSONFromString(const QString& str)
{
    QJsonObject json;
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());

    if (!doc.isNull()) {
        if (doc.isObject()) {
            json = doc.object();
        } else {
            qDebug() << "Document is not a JSON object: " << str;
        }
    } else {
        qDebug() << "Invalid JSON: " << str;
    }
    return json;
}

QString JSONStringFromInitList(const std::initializer_list<QPair<QString, QJsonValue>> args)
{
    QJsonObject jsonObject(args);
    return stringFromJSON(jsonObject);
}

QString
readJSONValue(const QJsonObject& json, const QString& key)
{
    if (!json.isEmpty() && json.contains(key) && json[key].isString()) {
        return json[key].toString();
    }
    return {};
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
    bool compressedImage = std::strncmp(profileInfo.avatar.c_str(), "/9g=", 4) == 0;;
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

enum class msgFlag {
    IS_INCOMING,
    IS_OUTGOING,
    IS_CONTACT_ADDED,
    IS_INVITATION_RECEIVED,
    IS_INVITATION_ACCEPTED,
    IS_TEXT
};

#pragma optimize( "", off )
std::pair<msgFlag, uint64_t>
migrateMessageBody(const std::string& body, api::interaction::Type type)
{
    uint64_t duration{ 0 };
    // check in english and local to determine the direction of the call
    switch (type) {
    case api::interaction::Type::CALL:
        {
            bool en_missedOut   = body.find("Missed outgoing call") != std::string::npos;
            bool en_out         = body.find("Outgoing call") != std::string::npos;
            bool loc_missedOut  = body.find(QObject::tr("Missed outgoing call").toStdString()) != std::string::npos;
            bool loc_out        = body.find(QObject::tr("Outgoing call").toStdString()) != std::string::npos;
            bool outgoingCall   = en_missedOut || en_out || loc_missedOut || loc_out;
            std::size_t dashPos = body.find("-");
            if (dashPos != std::string::npos) {
                duration = getTimeFromTimeStr(body.substr(dashPos + 2));
            }
            return std::make_pair(msgFlag(outgoingCall),
                                  duration);
        }
        break;
    case api::interaction::Type::CONTACT:
        {
            if (body.find("Contact added") != std::string::npos ||
                body.find(QObject::tr("Contact added").toStdString()) != std::string::npos) {
                return std::make_pair(msgFlag::IS_CONTACT_ADDED, 0);
            } else if (body.find("Invitation received") != std::string::npos ||
                       body.find(QObject::tr("Invitation received").toStdString()) != std::string::npos) {
                return std::make_pair(msgFlag::IS_INVITATION_RECEIVED, 0);
            } else if (body.find("Invitation accepted") != std::string::npos ||
                       body.find(QObject::tr("Invitation accepted").toStdString()) != std::string::npos) {
                return std::make_pair(msgFlag::IS_INVITATION_ACCEPTED, 0);
            }
        }
        break;
    default:
        return std::make_pair(msgFlag::IS_TEXT, 0);
    }
}
#pragma optimize( "", on )

std::vector<std::string>
getPeerParticipantsForConversationId(Database& db, const std::string& profileId, const std::string& conversationId)
{
    return db.select("participant_id",
        "conversations",
        "id=:id AND participant_id!=:participant_id",
        { {":id", conversationId}, {":participant_id", profileId} }).payloads;
}

} // namespace migration

void
migrateAccountDb(const QString& accountId,
                 std::shared_ptr<Database> db,
                 std::shared_ptr<Database> legacyDb)
{
    using namespace lrc::api;
    using namespace migration;

    auto accountLocalPath = getPath() + accountId + "/";

    using namespace DRing::Account;
    MapStringString accountDetails = ConfigurationManager::instance().
        getAccountDetails(accountId.toStdString().c_str());
    bool isRingAccount = accountDetails[ConfProperties::TYPE] == "RING";
    std::map<std::string, std::string> profileIdUriMap;
    std::map<std::string, std::string> convIdPeerUriMap;
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
        "account_id=:account_id AND is_account=:is_account",
        { {":account_id", accountId.toStdString()},
        {":is_account", "true"} }).payloads;
    if (accountProfileIds.size() != 1) {
        return;
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
        "account_id=:account_id AND is_account=:is_account",
        { {":account_id", accountId.toStdString()},
        {":is_account", "false"} }).payloads;
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
            qWarning() << "Profile file already exits: " << profileFilePath;
            continue;
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Can't open file: " << profileFilePath;
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
        return;
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
        convIdPeerUriMap.insert(std::make_pair(conversationId, it->second));
        try {
            db->insertInto("conversations",
                { {":id", "id"} ,
                {":participant", "participant"} },
                { { ":id", conversationId } ,
                { ":participant", it->second } });
        } catch (const std::runtime_error& e) {
            qWarning() << "Couldn't migrate conversation: " << e.what();
            continue;
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
        auto author_id = *(interactionIt + 1);
        auto it = profileIdUriMap.find(author_id);
        if (it == profileIdUriMap.end() && author_id != accountProfileId) {
            std::advance(interactionIt, allInteractions.nbrOfCols);
            continue;
        }
        auto daemonId = *(interactionIt + 7);
        auto status = *(interactionIt + 6);
        // migrate body+type ==> msgFlag+duration
        auto body = *(interactionIt + 4);
        auto type = *(interactionIt + 5);
        auto migratedMsg = migrateMessageBody(body,
            api::interaction::to_type(type));
        auto profileUri = it == profileIdUriMap.end() ? "" : it->second;
        // clear author uri if outgoing
        switch (migratedMsg.first) {
        case msgFlag::IS_OUTGOING:
        case msgFlag::IS_CONTACT_ADDED:
            profileUri.clear();
            break;
        case msgFlag::IS_INVITATION_RECEIVED:
        case msgFlag::IS_INVITATION_ACCEPTED:
            if (profileUri.empty()) {
                std::advance(interactionIt, allInteractions.nbrOfCols);
                continue;
            }
            break;
        case msgFlag::IS_INCOMING:
        {
            // try to set profile uri using the conversation id
            auto it = convIdPeerUriMap.find(*(interactionIt + 2));
            if (it == convIdPeerUriMap.end()) {
                std::advance(interactionIt, allInteractions.nbrOfCols);
                continue;
            }
            profileUri = it->second;
            break;
        }
        default:
            break;
        }
        // migrate type
        if (type == "OUTGOING_DATA_TRANSFER" || type == "INCOMING_DATA_TRANSFER") {
            type = "DATA_TRANSFER";
        }
        // Set all read, call and datatransfer, and contact added
        // interactions to a read state
        bool is_read = status == "READ"
            || type == "DATA_TRANSFER"
            || type == "CALL"
            || type == "CONTACT"
            || (type == "TEXT" && profileUri.empty());
        // migrate status
        if (status == "FAILED")
            status = "FAILURE";
        else if (status == "SUCCEED" || status == "READ" || status == "UNREAD")
            status = "SUCCESS";
        std::string extra_data = migratedMsg.second == 0 ? "" :
            JSONStringFromInitList({
                    qMakePair(QString("duration"),
                    QJsonValue((qint64)migratedMsg.second))
                })
            .toStdString();
        try {
            db->insertInto("interactions", {
                {":author", "author"},
                {":conversation", "conversation"},
                {":timestamp", "timestamp"},
                {":body", "body"},
                {":type", "type"},
                {":status", "status"},
                {":is_read", "is_read"},
                {":daemon_id", "daemon_id"},
                {":extra_data", "extra_data"}
            }, {
                {":author", profileUri},
                {":conversation", *(interactionIt + 2)},
                {":timestamp", *(interactionIt + 3)},
                {migratedMsg.first != msgFlag::IS_TEXT ? "" : ":body", *(interactionIt + 4)},
                {":type", type},
                {":status", status},
                {":is_read", is_read ? "1" : "0" },
                {daemonId.empty() ? "" : ":daemon_id", daemonId},
                {extra_data.empty() ? "" : ":extra_data", extra_data }
            });
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
        }
        std::advance(interactionIt, allInteractions.nbrOfCols);
    }
    qDebug() << "Done";
}

std::vector<std::shared_ptr<Database>>
migrateLegacyDatabaseIfNeeded(const QStringList& accountIds,
                              migrationCallback& willMigrateCb,
                              migrationCallback& didMigrateCb)
{
    using namespace lrc::api;
    using namespace migration;

    std::vector<std::shared_ptr<Database>> dbs(accountIds.size());

    if (!dbs.size()) {
        qDebug() << "No accounts to migrate";
        return {};
    }

    auto appPath = getPath();

    std::shared_ptr<Database> legacyDb;
    try {
        // create function should throw if there's no migration material
        legacyDb = lrc::DatabaseFactory::create<LegacyDatabase>(appPath);
    } catch (const std::runtime_error& e) {
        qDebug() << e.what();
        return dbs;
    }

    if (legacyDb == nullptr) {
        return dbs;
    }

    std::thread migrateThread(
        [&appPath, &accountIds, &dbs, &legacyDb, &didMigrateCb] {
            int index = 0;
            for (auto accountId : accountIds) {
                qDebug() << "Migrating account: " << accountId << "...";
                try {
                    auto dbName = QString::fromStdString(accountId.toStdString() + "/jami");
                    dbs.at(index) = lrc::DatabaseFactory::create<Database>(dbName, appPath);
                    auto& db = dbs.at(index++);
                    migrateAccountDb(accountId, db, legacyDb);
                } catch (const std::runtime_error& e) {
                    qWarning().noquote()
                        << "Could not migrate database for account: "
                        << accountId << "\n " << e.what();
                }
            }
            didMigrateCb();
        });

    willMigrateCb();
    migrateThread.join();

    return dbs;
}

} // namespace database

} // namespace authority

} // namespace lrc

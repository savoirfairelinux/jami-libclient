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
#include "storagehelper.h"

#include "api/profile.h"
#include "api/conversation.h"
#include "api/datatransfer.h"
#include "uri.h"
#include "vcard.h"

#include <account_const.h>
#include <datatransfer_interface.h>

#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>

#include <fstream>
#include <thread>
#include <cstring>

namespace lrc {

namespace authority {

namespace storage {

QString
getPath()
{
#ifdef Q_OS_WIN
    auto definedDataDir = qEnvironmentVariable("JAMI_DATA_HOME");
    if (!definedDataDir.isEmpty())
        return QDir(definedDataDir).absolutePath() + "/";
#endif
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    // Avoid to depends on the client name.
    dataDir.cdUp();
    return dataDir.absolutePath() + "/jami/";
}

QString
prepareUri(const QString& uri, api::profile::Type type)
{
    URI uriObject(uri);
    switch (type) {
    case api::profile::Type::SIP:
        return uriObject.format(URI::Section::USER_INFO | URI::Section::HOSTNAME);
        break;
    case api::profile::Type::JAMI:
        return uriObject.format(URI::Section::USER_INFO);
        break;
    case api::profile::Type::INVALID:
    case api::profile::Type::PENDING:
    case api::profile::Type::TEMPORARY:
    case api::profile::Type::COUNT__:
    default:
        return uri;
    }
}

QString
getFormattedCallDuration(const std::time_t duration)
{
    if (duration == 0)
        return {};
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
    if (seconds < 10)
        formattedString += "0";
    formattedString += std::to_string(seconds);
    return QString::fromStdString(formattedString);
}

QString
getCallInteractionString(const QString& authorUri, const std::time_t& duration)
{
    if (duration < 0) {
        if (authorUri.isEmpty()) {
            return QObject::tr("Outgoing call");
        } else {
            return QObject::tr("Incoming call");
        }
    } else if (authorUri.isEmpty()) {
        if (duration) {
            return QObject::tr("Outgoing call") + " - " + getFormattedCallDuration(duration);
        } else {
            return QObject::tr("Missed outgoing call");
        }
    } else {
        if (duration) {
            return QObject::tr("Incoming call") + " - " + getFormattedCallDuration(duration);
        } else {
            return QObject::tr("Missed incoming call");
        }
    }
}

QString
getContactInteractionString(const QString& authorUri, const api::interaction::Status& status)
{
    if (authorUri.isEmpty()) {
        return QObject::tr("Contact added");
    } else {
        if (status == api::interaction::Status::UNKNOWN) {
            return QObject::tr("Invitation received");
        } else if (status == api::interaction::Status::SUCCESS) {
            return QObject::tr("Invitation accepted");
        }
    }
    return {};
}

namespace vcard {
QString compressedAvatar(const QString& image);

QString
compressedAvatar(const QString& image)
{
    QImage qimage;
    // Avoid to use all formats. Some seems bugguy, like libpbf, asking
    // for a QGuiApplication for QFontDatabase
    auto ret = qimage.loadFromData(QByteArray::fromBase64(image.toUtf8()), "JPEG");
    if (!ret)
        ret = qimage.loadFromData(QByteArray::fromBase64(image.toUtf8()), "PNG");
    if (!ret) {
        qDebug() << "vCard image loading failed";
        return "";
    }
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);

    auto size = qMin(qimage.width(), qimage.height());
    auto rect = QRect((qimage.width() - size) / 2, (qimage.height() - size) / 2, size, size);
    qimage.copy(rect).scaled({size, size}, Qt::KeepAspectRatio).save(&buffer, "JPEG", 90);
    auto b64Img = bArray.toBase64().trimmed();
    return QString::fromLocal8Bit(b64Img.constData(), b64Img.length());
}

QString
profileToVcard(const api::profile::Info& profileInfo, bool compressImage)
{
    using namespace api;
    bool compressedImage = std::strncmp(profileInfo.avatar.toStdString().c_str(), "/9j/", 4) == 0;
    if (compressedImage && !compressImage) {
        compressImage = false;
    }
    QString vCardStr = vCard::Delimiter::BEGIN_TOKEN;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::VERSION;
    vCardStr += ":2.1";
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::FORMATTED_NAME;
    vCardStr += ":";
    vCardStr += profileInfo.alias;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (profileInfo.type == profile::Type::JAMI) {
        vCardStr += vCard::Property::TELEPHONE;
        vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
        vCardStr += "other:ring:";
        vCardStr += profileInfo.uri;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    } else {
        vCardStr += vCard::Property::TELEPHONE;
        vCardStr += ":";
        vCardStr += profileInfo.uri;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    }
    vCardStr += vCard::Property::PHOTO;
    vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
    vCardStr += vCard::Property::BASE64;
    vCardStr += vCard::Delimiter::SEPARATOR_TOKEN;
    if (compressImage) {
        vCardStr += vCard::Property::TYPE_JPEG;
        vCardStr += ":";
        vCardStr += compressedImage ? profileInfo.avatar : compressedAvatar(profileInfo.avatar);
    } else {
        vCardStr += compressedImage ? vCard::Property::TYPE_JPEG : vCard::Property::TYPE_PNG;
        vCardStr += ":";
        vCardStr += profileInfo.avatar;
    }
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Delimiter::END_TOKEN;
    return vCardStr;
}

void
setProfile(const QString& accountId, const api::profile::Info& profileInfo, const bool isPeer)
{
    auto vcard = vcard::profileToVcard(profileInfo);
    auto path = profileVcardPath(accountId, isPeer ? profileInfo.uri : "");
    QLockFile lf(path);
    QFile file(path);
    lf.lock();
    if (!file.open(QIODevice::WriteOnly)) {
        lf.unlock();
        qWarning().noquote() << "Can't open file for writing: " << file.fileName();
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    in << vcard;
    file.close();
    lf.unlock();
}
} // namespace vcard

VectorString
getConversationsWithPeer(Database& db, const QString& participant_uri)
{
    return db
        .select("id",
                "conversations",
                "participant=:participant",
                {{":participant", participant_uri}})
        .payloads;
}

VectorString
getPeerParticipantsForConversation(Database& db, const QString& conversationId)
{
    return db.select("participant", "conversations", "id=:id", {{":id", conversationId}}).payloads;
}

void
createOrUpdateProfile(const QString& accountId,
                      const api::profile::Info& profileInfo,
                      const bool isPeer)
{
    if (isPeer) {
        auto contact = storage::buildContactFromProfile(accountId,
                                                        profileInfo.uri,
                                                        profileInfo.type);
        if (!profileInfo.alias.isEmpty())
            contact.profileInfo.alias = profileInfo.alias;
        if (!profileInfo.avatar.isEmpty())
            contact.profileInfo.avatar = profileInfo.avatar;
        vcard::setProfile(accountId, contact.profileInfo, isPeer);
        return;
    }
    vcard::setProfile(accountId, profileInfo, isPeer);
}

void
removeProfile(const QString& accountId, const QString& peerUri)
{
    auto path = profileVcardPath(accountId, peerUri);
    if (!QFile::remove(path)) {
        qWarning() << "Couldn't remove vcard for" << peerUri << "at" << path;
    }
}

QString
getAccountAvatar(const QString& accountId)
{
    auto accountLocalPath = getPath() + accountId + "/";
    QString filePath;
    filePath = accountLocalPath + "profile.vcf";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can't open file: " << filePath;
        return {};
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    const auto vCard = lrc::vCard::utils::toHashMap(in.readAll().toUtf8());
    const auto photo = (vCard.find(vCard::Property::PHOTO_PNG) == vCard.end())
                           ? vCard[vCard::Property::PHOTO_JPEG]
                           : vCard[vCard::Property::PHOTO_PNG];
    return photo;
}

api::contact::Info
buildContactFromProfile(const QString& accountId,
                        const QString& peer_uri,
                        const api::profile::Type& type)
{
    lrc::api::profile::Info profileInfo;
    profileInfo.uri = peer_uri;
    profileInfo.type = type;
    auto accountLocalPath = getPath() + accountId + "/";
    QString b64filePath;
    b64filePath = profileVcardPath(accountId, peer_uri);
    QFile file(b64filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        // try non-base64 path
        QString filePath = accountLocalPath + "profiles/" + peer_uri + ".vcf";
        file.setFileName(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning().noquote() << "Can't open file: " << filePath;
            return {profileInfo, "", true, false};
        }
        // rename it
        qWarning().noquote() << "Renaming profile: " << filePath;
        file.rename(b64filePath);
        // reopen it
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning().noquote() << "Can't open file: " << b64filePath;
            return {profileInfo, "", true, false};
        }
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QByteArray vcard = in.readAll().toUtf8();
    const auto vCard = lrc::vCard::utils::toHashMap(vcard);
    const auto alias = vCard[vCard::Property::FORMATTED_NAME];
    for (const auto& key : vCard.keys()) {
        if (key.contains("PHOTO"))
            profileInfo.avatar = vCard[key];
    }
    profileInfo.alias = alias;
    return {profileInfo, "", type == api::profile::Type::JAMI, false};
}

VectorString
getAllConversations(Database& db)
{
    return db.select("id", "conversations", {}, {}).payloads;
}

VectorString
getConversationsBetween(Database& db, const QString& peer1_uri, const QString& peer2_uri)
{
    auto conversationsForPeer1 = getConversationsWithPeer(db, peer1_uri);
    std::sort(conversationsForPeer1.begin(), conversationsForPeer1.end());
    auto conversationsForPeer2 = getConversationsWithPeer(db, peer2_uri);
    std::sort(conversationsForPeer2.begin(), conversationsForPeer2.end());
    VectorString common;

    std::set_intersection(conversationsForPeer1.begin(),
                          conversationsForPeer1.end(),
                          conversationsForPeer2.begin(),
                          conversationsForPeer2.end(),
                          std::back_inserter(common));
    return common;
}

QString
beginConversationWithPeer(Database& db,
                          const QString& peer_uri,
                          const bool isOutgoing,
                          time_t timestamp)
{
    // Add conversation between account and profile
    auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1", "conversations", "1=1", {})
                                  .payloads[0];
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant", "participant"}},
                  {{":id", newConversationsId}, {":participant", peer_uri}});
    api::interaction::Info msg {isOutgoing ? "" : peer_uri,
                                {},
                                timestamp ? timestamp : std::time(nullptr),
                                0,
                                api::interaction::Type::CONTACT,
                                isOutgoing ? api::interaction::Status::SUCCESS
                                           : api::interaction::Status::UNKNOWN,
                                isOutgoing};
    // Add first interaction
    addMessageToConversation(db, newConversationsId, msg);
    return newConversationsId;
}

void
getHistory(Database& db, api::conversation::Info& conversation)
{
    auto interactionsResult
        = db.select("id, author, body, timestamp, type, status, is_read, extra_data",
                    "interactions",
                    "conversation=:conversation",
                    {{":conversation", conversation.uid}});
    auto nCols = 8;
    if (interactionsResult.nbrOfCols == nCols) {
        auto payloads = interactionsResult.payloads;
        for (decltype(payloads.size()) i = 0; i < payloads.size(); i += nCols) {
            QString durationString;
            auto extra_data_str = payloads[i + 7];
            if (!extra_data_str.isEmpty()) {
                auto jsonData = JSONFromString(extra_data_str);
                durationString = readJSONValue(jsonData, "duration");
            }
            auto body = payloads[i + 2];
            auto type = api::interaction::to_type(payloads[i + 4]);
            std::time_t duration = durationString.isEmpty()
                                       ? 0
                                       : std::stoi(durationString.toStdString());
            auto status = api::interaction::to_status(payloads[i + 5]);
            if (type == api::interaction::Type::CALL) {
                body = getCallInteractionString(payloads[i + 1], duration);
            } else if (type == api::interaction::Type::CONTACT) {
                body = getContactInteractionString(payloads[i + 1], status);
            }
            auto msg = api::interaction::Info({payloads[i + 1],
                                               body,
                                               std::stoi(payloads[i + 3].toStdString()),
                                               duration,
                                               type,
                                               status,
                                               (payloads[i + 6] == "1" ? true : false)});
            conversation.interactions.emplace(payloads[i], std::move(msg));
            conversation.lastMessageUid = payloads[i];
            if (status != api::interaction::Status::DISPLAYED || !payloads[i + 1].isEmpty()) {
                continue;
            }
            auto messageId = conversation.lastDisplayedMessageUid.find(
                conversation.participants.front());
            if (messageId == conversation.lastDisplayedMessageUid.end()) {
                conversation.lastDisplayedMessageUid.emplace(conversation.participants.front(),
                                                             payloads[i]);
                continue;
            }
            auto lastReadInteraction = conversation.interactions.find(messageId->second);
            auto timestamp = std::stoi(payloads[i + 3].toStdString());
            if (lastReadInteraction == conversation.interactions.end()
                || lastReadInteraction->second.timestamp < timestamp) {
                conversation.lastDisplayedMessageUid.at(conversation.participants.front())
                    = std::stoull(payloads[i].toStdString());
            }
        }
    }
}

QString
addMessageToConversation(Database& db,
                         const QString& conversationId,
                         const api::interaction::Info& msg)
{
    return db.insertInto("interactions",
                         {{":author", "author"},
                          {":conversation", "conversation"},
                          {":timestamp", "timestamp"},
                          {":body", "body"},
                          {":type", "type"},
                          {":status", "status"},
                          {":is_read", "is_read"}},
                         {{":author", msg.authorUri},
                          {":conversation", conversationId},
                          {":timestamp", toQString(msg.timestamp)},
                          {":body", msg.body},
                          {":type", to_string(msg.type)},
                          {":status", to_string(msg.status)},
                          {":is_read", msg.isRead ? "1" : "0"}});
}

QString
addOrUpdateMessage(Database& db,
                   const QString& conversationId,
                   const api::interaction::Info& msg,
                   const QString& daemonId)
{
    // Check if profile is already present.
    auto msgAlreadyExists = db.select("id",
                                      "interactions",
                                      "author=:author AND daemon_id=:daemon_id",
                                      {{":author", msg.authorUri}, {":daemon_id", daemonId}})
                                .payloads;
    if (msgAlreadyExists.empty()) {
        auto extra_data_JSON = JSONFromString("");
        writeJSONValue(extra_data_JSON, "duration", QString::number(msg.duration));
        auto extra_data = stringFromJSON(extra_data_JSON);
        return db.insertInto("interactions",
                             {{":author", "author"},
                              {":conversation", "conversation"},
                              {":timestamp", "timestamp"},
                              {":body", "body"},
                              {":type", "type"},
                              {":status", "status"},
                              {":daemon_id", "daemon_id"},
                              {":extra_data", "extra_data"}},
                             {{":author", msg.authorUri.isEmpty() ? "" : msg.authorUri},
                              {":conversation", conversationId},
                              {":timestamp", toQString(msg.timestamp)},
                              {msg.body.isEmpty() ? "" : ":body", msg.body},
                              {":type", to_string(msg.type)},
                              {daemonId.isEmpty() ? "" : ":daemon_id", daemonId},
                              {":status", to_string(msg.status)},
                              {extra_data.isEmpty() ? "" : ":extra_data", extra_data}});
    } else {
        // already exists @ id(msgAlreadyExists[0])
        auto id = msgAlreadyExists[0];
        QString extra_data;
        if (msg.type == api::interaction::Type::CALL) {
            auto duration = std::max(msg.duration, static_cast<std::time_t>(0));
            auto extra_data_str = getInteractionExtraDataById(db, id);
            auto extra_data_JSON = JSONFromString(extra_data_str);
            writeJSONValue(extra_data_JSON, "duration", QString::number(duration));
            extra_data = stringFromJSON(extra_data_JSON);
        }
        db.update("interactions",
                  {"body=:body, extra_data=:extra_data"},
                  {{msg.body.isEmpty() ? "" : ":body", msg.body},
                   {extra_data.isEmpty() ? "" : ":extra_data", extra_data}},
                  "id=:id",
                  {{":id", id}});
        return id;
    }
}

QString
addDataTransferToConversation(Database& db,
                              const QString& conversationId,
                              const api::datatransfer::Info& infoFromDaemon)
{
    auto convId = conversationId.isEmpty() ? NULL : conversationId;
    return db.insertInto("interactions",
                         {{":author", "author"},
                          {":conversation", "conversation"},
                          {":timestamp", "timestamp"},
                          {":body", "body"},
                          {":type", "type"},
                          {":status", "status"},
                          {":is_read", "is_read"},
                          {":daemon_id", "daemon_id"}},
                         {{":author", infoFromDaemon.isOutgoing ? "" : infoFromDaemon.peerUri},
                          {":conversation", convId},
                          {":timestamp", toQString(std::time(nullptr))},
                          {":body", infoFromDaemon.path},
                          {":type", "DATA_TRANSFER"},
                          {":status", "TRANSFER_CREATED"},
                          {":is_read", "0"},
                          {":daemon_id", infoFromDaemon.uid}});
}

void
addDaemonMsgId(Database& db, const QString& interactionId, const QString& daemonId)
{
    db.update("interactions",
              "daemon_id=:daemon_id",
              {{":daemon_id", daemonId}},
              "id=:id",
              {{":id", interactionId}});
}

QString
getDaemonIdByInteractionId(Database& db, const QString& id)
{
    auto ids = db.select("daemon_id", "interactions", "id=:id", {{":id", id}}).payloads;
    return ids.empty() ? "" : ids[0];
}

QString
getInteractionIdByDaemonId(Database& db, const QString& daemon_id)
{
    auto ids = db.select("id", "interactions", "daemon_id=:daemon_id", {{":daemon_id", daemon_id}})
                   .payloads;
    return ids.empty() ? "" : ids[0];
}

void
updateDataTransferInteractionForDaemonId(Database& db,
                                         const QString& daemonId,
                                         api::interaction::Info& interaction)
{
    auto result = db.select("body, status",
                            "interactions",
                            "daemon_id=:daemon_id",
                            {{":daemon_id", daemonId}})
                      .payloads;
    if (result.size() < 2) {
        return;
    }
    auto body = result[0];
    auto status = api::interaction::to_status(result[1]);
    interaction.body = body;
    interaction.status = status;
}

QString
getInteractionExtraDataById(Database& db, const QString& id, const QString& key)
{
    auto extra_datas = db.select("extra_data", "interactions", "id=:id", {{":id", id}}).payloads;
    if (key.isEmpty()) {
        return extra_datas.empty() ? "" : extra_datas[0];
    }
    QString value;
    if (!extra_datas[0].isEmpty()) {
        value = readJSONValue(JSONFromString(extra_datas[0]), key);
    }
    return value;
}

void
updateInteractionBody(Database& db, const QString& id, const QString& newBody)
{
    db.update("interactions", "body=:body", {{":body", newBody}}, "id=:id", {{":id", id}});
}

void
updateInteractionStatus(Database& db, const QString& id, api::interaction::Status newStatus)
{
    db.update("interactions",
              {"status=:status"},
              {{":status", api::interaction::to_string(newStatus)}},
              "id=:id",
              {{":id", id}});
}

void
setInteractionRead(Database& db, const QString& id)
{
    db.update("interactions", {"is_read=:is_read"}, {{":is_read", "1"}}, "id=:id", {{":id", id}});
}

QString
conversationIdFromInteractionId(Database& db, const QString& interactionId)
{
    auto result = db.select("conversation", "interactions", "id=:id", {{":id", interactionId}});
    if (result.nbrOfCols == 1 && result.payloads.size()) {
        return result.payloads[0];
    }
    return {};
}

void
clearHistory(Database& db, const QString& conversationId)
{
    try {
        db.deleteFrom("interactions",
                      "conversation=:conversation",
                      {{":conversation", conversationId}});
    } catch (Database::QueryDeleteError& e) {
        qWarning() << "deleteFrom error: " << e.details();
    }
}

void
clearInteractionFromConversation(Database& db,
                                 const QString& conversationId,
                                 const QString& interactionId)
{
    try {
        db.deleteFrom("interactions",
                      "conversation=:conversation AND id=:id",
                      {{":conversation", conversationId}, {":id", interactionId}});
    } catch (Database::QueryDeleteError& e) {
        qWarning() << "deleteFrom error: " << e.details();
    }
}

void
clearAllHistory(Database& db)
{
    try {
        db.deleteFrom("interactions", "1=1", {});
    } catch (Database::QueryDeleteError& e) {
        qWarning() << "deleteFrom error: " << e.details();
    }
}

void
deleteObsoleteHistory(Database& db, long int date)
{
    try {
        db.deleteFrom("interactions", "timestamp<=:date", {{":date", toQString(date)}});
    } catch (Database::QueryDeleteError& e) {
        qWarning() << "deleteFrom error: " << e.details();
    }
}

void
removeContactConversations(Database& db, const QString& contactUri)
{
    // Get common conversations
    auto conversations = getConversationsWithPeer(db, contactUri);
    // Remove conversations + interactions
    try {
        for (const auto& conversationId : conversations) {
            // Remove conversation
            db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
            // clear History
            db.deleteFrom("interactions", "conversation=:id", {{":id", conversationId}});
        }
    } catch (Database::QueryDeleteError& e) {
        qWarning() << "deleteFrom error: " << e.details();
    }
}

int
countUnreadFromInteractions(Database& db, const QString& conversationId)
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
        if (!timestamps.empty() && !timestamps[0].isEmpty()) {
            result = std::stoull(timestamps[0].toStdString());
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "storage::getLastTimestamp, stoull throws an out_of_range exception: "
                 << e.what();
    } catch (const std::invalid_argument& e) {
        qDebug() << "storage::getLastTimestamp, stoull throws an invalid_argument exception: "
                 << e.what();
    }
    return result;
}

namespace {
QString
profileVcardPath(const QString& accountId, const QString& uri)
{
    auto accountLocalPath = getPath() + accountId + QDir::separator();
    if (uri.isEmpty())
        return accountLocalPath + "profile.vcf";

    auto fileName = QString(uri.toUtf8().toBase64());
    return accountLocalPath + "profiles" + QDir::separator() + fileName + ".vcf";
}

QString
stringFromJSON(const QJsonObject& json)
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

QString
JSONStringFromInitList(const std::initializer_list<QPair<QString, QJsonValue>> args)
{
    QJsonObject jsonObject(args);
    return stringFromJSON(jsonObject);
}

QString
readJSONValue(const QJsonObject& json, const QString& key)
{
    if (!json.isEmpty() && json.contains(key) && json[key].isString()) {
        if (json[key].isString()) {
            return json[key].toString();
        }
    }
    return {};
}

void
writeJSONValue(QJsonObject& json, const QString& key, const QString& value)
{
    json[key] = value;
}
} // namespace

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
// ├── profile.vcf < --account vcard
// ├── profiles < --account contact vcards
// │   │──{ contact_uri }.vcf
// │   └── ...
// ├── ring_device.crt
// └── ring_device.key
//================================================================================
namespace migration {

enum class msgFlag {
    IS_INCOMING,
    IS_OUTGOING,
    IS_CONTACT_ADDED,
    IS_INVITATION_RECEIVED,
    IS_INVITATION_ACCEPTED,
    IS_TEXT
};

QString profileToVcard(const lrc::api::profile::Info&, const QString&);
uint64_t getTimeFromTimeStr(const QString&) noexcept;
std::pair<msgFlag, uint64_t> migrateMessageBody(const QString&, const lrc::api::interaction::Type&);
VectorString getPeerParticipantsForConversationId(lrc::Database&, const QString&, const QString&);
void migrateAccountDb(const QString&,
                      std::shared_ptr<lrc::Database>,
                      std::shared_ptr<lrc::Database>);

namespace interaction {

static inline api::interaction::Type
to_type(const QString& type)
{
    if (type == "TEXT")
        return api::interaction::Type::TEXT;
    else if (type == "CALL")
        return api::interaction::Type::CALL;
    else if (type == "CONTACT")
        return api::interaction::Type::CONTACT;
    else if (type == "OUTGOING_DATA_TRANSFER")
        return api::interaction::Type::DATA_TRANSFER;
    else if (type == "INCOMING_DATA_TRANSFER")
        return api::interaction::Type::DATA_TRANSFER;
    else
        return api::interaction::Type::INVALID;
}

static inline QString
to_migrated_status_string(const QString& status)
{
    if (status == "FAILED")
        return "FAILURE";
    else if (status == "SUCCEED")
        return "SUCCESS";
    else if (status == "READ")
        return "SUCCESS";
    else if (status == "UNREAD")
        return "SUCCESS";
    else
        return status;
}

} // namespace interaction

QString
profileToVcard(const api::profile::Info& profileInfo, const QString& accountId = {})
{
    using namespace api;
    bool compressedImage = std::strncmp(profileInfo.avatar.toStdString().c_str(), "/9g=", 4) == 0;
    ;
    QString vCardStr = vCard::Delimiter::BEGIN_TOKEN;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Property::VERSION;
    vCardStr += ":2.1";
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (!accountId.isEmpty()) {
        vCardStr += vCard::Property::UID;
        vCardStr += ":";
        vCardStr += accountId;
        vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    }
    vCardStr += vCard::Property::FORMATTED_NAME;
    vCardStr += ":";
    vCardStr += profileInfo.alias;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    if (profileInfo.type == profile::Type::JAMI) {
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
getTimeFromTimeStr(const QString& str) noexcept
{
    uint64_t minutes = 0, seconds = 0;
    std::string timeStr = str.toStdString();
    std::size_t delimiterPos = timeStr.find(":");
    if (delimiterPos != std::string::npos) {
        try {
            minutes = std::stoull(timeStr.substr(0, delimiterPos));
            seconds = std::stoull(timeStr.substr(delimiterPos + 1));
        } catch (const std::exception&) {
            return 0;
        }
    }
    return minutes * 60 + seconds;
}

std::pair<msgFlag, uint64_t>
migrateMessageBody(const QString& body, const api::interaction::Type& type)
{
    uint64_t duration {0};
    // check in english and local to determine the direction of the call
    static QString emo = "Missed outgoing call";
    static QString lmo = QObject::tr("Missed outgoing call");
    static QString eo = "Outgoing call";
    static QString lo = QObject::tr("Outgoing call");
    static QString eca = "Contact added";
    static QString lca = QObject::tr("Contact added");
    static QString eir = "Invitation received";
    static QString lir = QObject::tr("Invitation received");
    static QString eia = "Invitation accepted";
    static QString lia = QObject::tr("Invitation accepted");
    auto strBody = body.toStdString();
    switch (type) {
    case api::interaction::Type::CALL: {
        bool en_missedOut = body.contains(emo);
        bool en_out = body.contains(eo);
        bool loc_missedOut = body.contains(lmo);
        bool loc_out = body.contains(lo);
        bool outgoingCall = en_missedOut || en_out || loc_missedOut || loc_out;
        std::size_t dashPos = strBody.find("-");
        if (dashPos != std::string::npos) {
            duration = getTimeFromTimeStr(toQString(strBody.substr(dashPos + 2)));
        }
        return std::make_pair(msgFlag(outgoingCall), duration);
    } break;
    case api::interaction::Type::CONTACT:
        if (body.contains(eca) || body.contains(lca)) {
            return std::make_pair(msgFlag::IS_CONTACT_ADDED, 0);
        } else if (body.contains(eir) || body.contains(lir)) {
            return std::make_pair(msgFlag::IS_INVITATION_RECEIVED, 0);
        } else if (body.contains(eia) || body.contains(lia)) {
            return std::make_pair(msgFlag::IS_INVITATION_ACCEPTED, 0);
        }
        break;
    case api::interaction::Type::INVALID:
    case api::interaction::Type::TEXT:
    case api::interaction::Type::DATA_TRANSFER:
    case api::interaction::Type::COUNT__:
    default:
        return std::make_pair(msgFlag::IS_TEXT, 0);
    }
    return std::make_pair(msgFlag::IS_OUTGOING, 0);
}

VectorString
getPeerParticipantsForConversationId(Database& db,
                                     const QString& profileId,
                                     const QString& conversationId)
{
    return db
        .select("participant_id",
                "conversations",
                "id=:id AND participant_id!=:participant_id",
                {{":id", conversationId}, {":participant_id", profileId}})
        .payloads;
}

void
migrateAccountDb(const QString& accountId,
                 std::shared_ptr<Database> db,
                 std::shared_ptr<Database> legacyDb)
{
    using namespace lrc::api;
    using namespace migration;

    auto accountLocalPath = getPath() + accountId + "/";

    using namespace DRing::Account;
    MapStringString accountDetails = ConfigurationManager::instance().getAccountDetails(
        accountId.toStdString().c_str());
    bool isRingAccount = accountDetails[ConfProperties::TYPE] == "RING";
    std::map<QString, QString> profileIdUriMap;
    std::map<QString, QString> convIdPeerUriMap;
    QString accountProfileId;

    // 1. profiles_accounts
    // migrate account's avatar/alias from profiles table to {data_dir}/profile.vcf
    QString accountUri;
    if (isRingAccount) {
        accountUri = accountDetails[DRing::Account::ConfProperties::USERNAME].contains("ring:")
                         ? QString(accountDetails[DRing::Account::ConfProperties::USERNAME])
                               .remove(QString("ring:"))
                         : accountDetails[DRing::Account::ConfProperties::USERNAME];
    } else {
        accountUri = accountDetails[DRing::Account::ConfProperties::USERNAME];
    }

    auto accountProfileIds = legacyDb
                                 ->select("profile_id",
                                          "profiles_accounts",
                                          "account_id=:account_id AND is_account=:is_account",
                                          {{":account_id", accountId}, {":is_account", "true"}})
                                 .payloads;
    if (accountProfileIds.size() != 1) {
        return;
    }
    accountProfileId = accountProfileIds[0];
    auto accountProfile
        = legacyDb->select("photo, alias", "profiles", "id=:id", {{":id", accountProfileId}})
              .payloads;
    profile::Info accountProfileInfo;
    // if we can not find the uri in the database
    // (in the case of poorly kept SIP account uris),
    // than we cannot migrate the conversations and vcard
    if (!accountProfile.empty()) {
        accountProfileInfo = {accountUri,
                              accountProfile[0],
                              accountProfile[1],
                              isRingAccount ? profile::Type::JAMI : profile::Type::SIP};
    }
    auto accountVcard = profileToVcard(accountProfileInfo, accountId);
    QDir dir;
    if (!dir.exists(accountLocalPath)) {
        dir.mkpath(accountLocalPath);
    }
    auto profileFilePath = accountLocalPath + "profile.vcf";
    QFile file(profileFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Can't open file: " + profileFilePath.toStdString());
    }
    QTextStream(&file) << accountVcard;

    // 2. profiles
    // migrate profiles from profiles table to {data_dir}/{uri}.vcf
    // - for JAMI, the scheme and the hostname is omitted
    // - for SIP, the uri is must be stripped of prefix and port
    // e.g. 3d1112ab2bb089370c0744a44bbbb0786418d40b.vcf
    //      username.vcf or username@hostname.vcf

    // only select non-account profiles
    auto profileIds = legacyDb
                          ->select("profile_id",
                                   "profiles_accounts",
                                   "account_id=:account_id AND is_account=:is_account",
                                   {{":account_id", accountId}, {":is_account", "false"}})
                          .payloads;
    for (const auto& profileId : profileIds) {
        auto profile = legacyDb
                           ->select("uri, alias, photo, type",
                                    "profiles",
                                    "id=:id",
                                    {{":id", profileId}})
                           .payloads;
        if (profile.empty()) {
            continue;
        }
        profile::Info profileInfo {profile[0], profile[2], profile[1]};
        auto uri = URI(profile[0]);
        auto profileUri = uri.userinfo();
        if (!isRingAccount && uri.hasHostname()) {
            profileUri += "@" + uri.hostname();
        }
        // insert into map for use during the conversations table migration
        profileIdUriMap.insert(std::make_pair(profileId, profileUri));
        auto vcard = profileToVcard(profileInfo);
        // make sure the directory exists
        QDir dir(accountLocalPath + "profiles");
        if (!dir.exists())
            dir.mkpath(".");
        profileFilePath = accountLocalPath + "profiles/" + profileUri + ".vcf";
        QFile file(profileFilePath);
        // if we catch duplicates here, skip the profile because
        // the previous db structure does not guarantee unique uris
        if (file.exists()) {
            qWarning() << "Profile file already exits: " << profileFilePath;
            continue;
        }
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "Can't open file: " << profileFilePath;
            continue;
        }
        QTextStream(&file) << vcard;
    }

    // 3. conversations
    // migrate old conversations table ==> new conversations table
    // a) participant_id INTEGER becomes participant TEXT (the uri of the participant)
    //    use the selected non-account profiles
    auto conversationIds = legacyDb
                               ->select("id",
                                        "conversations",
                                        "participant_id=:participant_id",
                                        {{":participant_id", accountProfileId}})
                               .payloads;
    if (conversationIds.empty()) {
        return;
    }
    for (auto conversationId : conversationIds) {
        // only one peer pre-groupchat
        auto peerProfileId = getPeerParticipantsForConversationId(*legacyDb,
                                                                  accountProfileId,
                                                                  conversationId);
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
                           {{":id", "id"}, {":participant", "participant"}},
                           {{":id", conversationId}, {":participant", it->second}});
        } catch (const std::runtime_error& e) {
            qWarning() << "Couldn't migrate conversation: " << e.what();
            continue;
        }
    }

    // 4. interactions
    auto allInteractions = legacyDb->select("account_id, author_id, conversation_id, \
         timestamp, body, type, status, daemon_id",
                                            "interactions",
                                            "account_id=:account_id",
                                            {{":account_id", accountProfileId}});
    auto interactionIt = allInteractions.payloads.begin();
    while (interactionIt != allInteractions.payloads.end()) {
        auto author_id = *(interactionIt + 1);
        auto convId = *(interactionIt + 2);
        auto timestamp = *(interactionIt + 3);
        auto body = *(interactionIt + 4);
        auto type = interaction::to_type(*(interactionIt + 5));
        auto statusStr = *(interactionIt + 6);
        auto daemonId = *(interactionIt + 7);

        auto it = profileIdUriMap.find(author_id);
        if (it == profileIdUriMap.end() && author_id != accountProfileId) {
            std::advance(interactionIt, allInteractions.nbrOfCols);
            continue;
        }
        // migrate body+type ==> msgFlag+duration
        auto migratedMsg = migrateMessageBody(body, type);
        QString profileUri = it == profileIdUriMap.end() ? "" : it->second;
        // clear author uri if outgoing
        switch (migratedMsg.first) {
        case msgFlag::IS_OUTGOING:
        case msgFlag::IS_CONTACT_ADDED:
            profileUri.clear();
            break;
        case msgFlag::IS_INCOMING:
        case msgFlag::IS_INVITATION_RECEIVED:
        case msgFlag::IS_INVITATION_ACCEPTED: {
            // try to set profile uri using the conversation id
            auto it = convIdPeerUriMap.find(convId);
            if (it == convIdPeerUriMap.end()) {
                std::advance(interactionIt, allInteractions.nbrOfCols);
                continue;
            }
            profileUri = it->second;
            break;
        }
        case msgFlag::IS_TEXT:
        default:
            break;
        }
        // Set all read, call and datatransfer, and contact added
        // interactions to a read state
        bool is_read = statusStr != "UNREAD" || type == api::interaction::Type::CALL
                       || type == api::interaction::Type::CONTACT;
        // migrate status
        if (migratedMsg.first == msgFlag::IS_INVITATION_RECEIVED) {
            statusStr = "UNKNOWN";
        }
        QString extra_data = migratedMsg.second == 0
                                 ? ""
                                 : JSONStringFromInitList(
                                     {qMakePair(QString("duration"),
                                                QJsonValue(QString::number(migratedMsg.second)))});
        if (accountUri == profileUri)
            profileUri.clear();
        auto typeStr = api::interaction::to_string(type);
        try {
            db->insertInto("interactions",
                           {{":author", "author"},
                            {":conversation", "conversation"},
                            {":timestamp", "timestamp"},
                            {":body", "body"},
                            {":type", "type"},
                            {":status", "status"},
                            {":is_read", "is_read"},
                            {":daemon_id", "daemon_id"},
                            {":extra_data", "extra_data"}},
                           {{":author", profileUri},
                            {":conversation", convId},
                            {":timestamp", timestamp},
                            {migratedMsg.first != msgFlag::IS_TEXT ? "" : ":body", body},
                            {":type", api::interaction::to_string(type)},
                            {":status", interaction::to_migrated_status_string(statusStr)},
                            {":is_read", is_read ? "1" : "0"},
                            {daemonId.isEmpty() ? "" : ":daemon_id", daemonId},
                            {extra_data.isEmpty() ? "" : ":extra_data", extra_data}});
        } catch (const std::runtime_error& e) {
            qWarning() << e.what();
        }
        std::advance(interactionIt, allInteractions.nbrOfCols);
    }
    qDebug() << "Done";
}

} // namespace migration

std::vector<std::shared_ptr<Database>>
migrateIfNeeded(const QStringList& accountIds, MigrationCb& willMigrateCb, MigrationCb& didMigrateCb)
{
    using namespace lrc::api;
    using namespace migration;

    std::vector<std::shared_ptr<Database>> dbs(accountIds.size());

    if (!accountIds.size()) {
        qDebug() << "No accounts to migrate";
        return dbs;
    }

    auto appPath = getPath();

    // ring -> jami path migration
    QDir dataDir(appPath);
    // create data directory if not created yet
    dataDir.mkpath(appPath);
    QDir oldDataDir(appPath);
    oldDataDir.cdUp();
    oldDataDir = oldDataDir.absolutePath()
#if defined(_WIN32)
                 + "/Savoir-faire Linux/Ring";
#elif defined(__APPLE__)
                 + "/ring";
#else
                 + "/gnome-ring";
#endif
    QStringList filesList = oldDataDir.entryList();
    QString filename;
    QDir dir;
    bool success = true;
    foreach (filename, filesList) {
        qDebug() << "Migrate " << oldDataDir.absolutePath() << "/" << filename << " to "
                 << dataDir.absolutePath() + "/" + filename;
        if (filename != "." && filename != "..") {
            success &= dir.rename(oldDataDir.absolutePath() + "/" + filename,
                                  dataDir.absolutePath() + "/" + filename);
        }
    }
    if (success) {
        // Remove old directory if the migration is successful.
#if defined(_WIN32)
        oldDataDir.cdUp();
#endif
        oldDataDir.removeRecursively();
    }

    bool needsMigration = false;
    std::map<QString, bool> hasMigratedData;
    for (const auto& accountId : accountIds) {
        auto hasMigratedDb = QFile(appPath + accountId + "/history.db").exists()
                             && !QFile(appPath + accountId + "/history.db-journal").exists();
        hasMigratedData.insert(std::make_pair(accountId, hasMigratedDb));
        needsMigration |= !hasMigratedDb;
    }
    if (!needsMigration) {
        // if there's any lingering pre-migration data, remove it
        QFile(dataDir.absoluteFilePath("ring.db")).remove();
        QDir(dataDir.absoluteFilePath("text/")).removeRecursively();
        QDir(dataDir.absoluteFilePath("profiles/")).removeRecursively();
        QDir(dataDir.absoluteFilePath("peer_profiles/")).removeRecursively();
        qDebug() << "No migration required";
        return dbs;
    }

    // A fairly long migration may now occur
    std::thread migrateThread(
        [&appPath, &accountIds, &dbs, &didMigrateCb, &dataDir, &hasMigratedData] {
            // 1. migrate old lrc -> new lrc if needed
            // 2. migrate new lrc db version 1 -> db version 1.1 if needed
            // the destructor of LegacyDatabase will remove 'ring.db' and clean out
            // old lrc files
            std::shared_ptr<Database> legacyDb;
            try {
                legacyDb = lrc::DatabaseFactory::create<LegacyDatabase>(appPath);
            } catch (const std::runtime_error& e) {
                qDebug() << "Exception while attempting to load legacy database: " << e.what();
                if (didMigrateCb)
                    didMigrateCb();
                return;
            }

            // attempt to make a backup of ring.db
            {
                QFile dbFile(dataDir.absoluteFilePath("ring.db"));
                if (dbFile.open(QIODevice::ReadOnly)) {
                    dbFile.copy(appPath + "ring.db.bak");
                }
            }

            // 3. migrate db version 1.1 -> per account dbs version 1
            int index = 0;
            for (const auto& accountId : accountIds) {
                if (hasMigratedData.at(accountId)) {
                    index++;
                    continue;
                }
                qDebug() << "Migrating account: " << accountId << "...";
                // try to remove the transaction journal from a failed migration
                QFile(appPath + accountId + "/history.db-journal").remove();
                try {
                    QSqlDatabase::database().transaction();
                    auto dbName = QString::fromStdString(accountId.toStdString() + "/history");
                    dbs.at(index) = lrc::DatabaseFactory::create<Database>(dbName, appPath);
                    auto& db = dbs.at(index++);
                    migration::migrateAccountDb(accountId, db, legacyDb);
                    QSqlDatabase::database().commit();
                } catch (const std::runtime_error& e) {
                    qWarning().noquote() << "Could not migrate database for account: " << accountId
                                         << "\n " << e.what();
                    QSqlDatabase::database().rollback();
                }
            }

            // done
            if (didMigrateCb)
                didMigrateCb();
        });

    // if willMigrateCb blocks, it must be unblocked by didMigrateCb
    if (willMigrateCb)
        willMigrateCb();

    migrateThread.join();

    return dbs;
}

} // namespace storage

} // namespace authority

} // namespace lrc

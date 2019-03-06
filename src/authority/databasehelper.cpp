/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                             *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
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

#include <fstream>

namespace lrc
{

namespace authority
{

namespace database
{

std::string
compressedAvatar(const std::string& img)
{
    QImage image;
    const bool ret = image.loadFromData(QByteArray::fromBase64(img.c_str()), 0);
    if (!ret) {
        qDebug() << "vCard image loading failed";
        return img;
    }
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    image.scaled({ 128, 128 }).save(&buffer, "JPEG", 90);
    auto b64Img = bArray.toBase64().trimmed();
    return std::string(b64Img.constData(), b64Img.length());
}

std::string
profileToVcard(const api::profile::Info& profileInfo,
               const std::string& accountId = {},
               bool compressImage = true)
{
    using namespace api;
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
    vCardStr += compressImage ? "TYPE=JPEG:" : "TYPE=PNG:";
    vCardStr += compressImage ? compressedAvatar(profileInfo.avatar) : profileInfo.avatar;
    vCardStr += vCard::Delimiter::END_LINE_TOKEN;
    vCardStr += vCard::Delimiter::END_TOKEN;
    return vCardStr;
}

std::string
getProfileId(Database& db,
            const std::string& accountId,
            const std::string& isAccount,
            const std::string& uri)
{
    auto accountProfiles = db.select("profile_id", "profiles_accounts",
                                     "account_id=:account_id AND is_account=:is_account",
                                     {{":account_id", accountId},
                                     {":is_account", isAccount}}).payloads;
    if (accountProfiles.empty() && (isAccount == "true")) {
        return "";
    }
    if (isAccount == "true") return accountProfiles[0];

    // we may have many contacts profiles for one account id,
    // and need to check uri in addition to account id
    auto profiles = db.select("id", "profiles", "uri=:uri", {{":uri", uri}}).payloads;

    if (profiles.empty()) return "";
    std::sort(accountProfiles.begin(), accountProfiles.end());
    std::sort(profiles.begin(), profiles.end());

    std::vector<std::string> common;
    std::set_intersection(accountProfiles.begin(), accountProfiles.end(),
                          profiles.begin(), profiles.end(),
                          std::back_inserter(common));
    //if profile exists but not linked with account id,
    // update profiles_accounts. Except empty uri for SIP accounts
    if(common.empty()) {
        if(!uri.empty()) {
            db.insertInto("profiles_accounts",
                         {{":profile_id", "profile_id"}, {":account_id", "account_id"},
                         {":is_account", "is_account"}},
                         {{":profile_id", profiles[0]}, {":account_id", accountId},
                         {":is_account", isAccount}});
        }
        return profiles[0];
    }
    return  common[0];
}

std::string
getOrInsertProfile(Database& db,
                   const std::string& contactUri,
                   const std::string& accountId,
                   bool  isAccount,
                   const std::string& type,
                   const std::string& alias,
                   const std::string& avatar)
{
    // Check if profile is already present.
    const std::string isAccountStr = isAccount ? "true" : "false";
    auto profileAlreadyExists = getProfileId(db, accountId, isAccountStr, contactUri);
    if (profileAlreadyExists.empty()) {
       // Doesn't exists, add profile to the database
       auto row = db.insertInto("profiles",
       {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
       {":status", "status"}},
       {{":uri", contactUri}, {":alias", alias}, {":photo", avatar}, {":type", type},
       {":status", "TRUSTED"}});

        if (row == -1) {
            qDebug() << "contact not added to the database";
            return "";
        }
        // link profile id to account id
        auto profiles = db.select("profile_id", "profiles_accounts",
                                  "profile_id=:profile_id AND \
                                  account_id=:account_id AND  \
                                  is_account=:is_account",
                                  {{":profile_id", std::to_string(row)},
                                  {":account_id", accountId},
                                  {":is_account", isAccountStr}})
                                  .payloads;

       if (profiles.empty()) {
            db.insertInto("profiles_accounts",
                          {{":profile_id", "profile_id"},
                          {":account_id", "account_id"},
                          {":is_account", "is_account"}},
                          {{":profile_id", std::to_string(row)},
                          {":account_id", accountId},
                          {":is_account", isAccountStr}});
       }

      return std::to_string(row);
    } else {
       // Exists, update and retrieve it.
       if (!avatar.empty() && !alias.empty()) {
           db.update("profiles",
                     "alias=:alias, photo=:photo",
                     {{":alias", alias}, {":photo", avatar}},
                     "id=:id", {{":id", profileAlreadyExists}});
       } else if (!avatar.empty()) {
           db.update("profiles",
                     "photo=:photo",
                     {{":photo", avatar}},
                     "id=:id", {{":id", profileAlreadyExists}});
       }
       return profileAlreadyExists;
    }
}

std::vector<std::string>
getConversationsForProfile(Database& db, const std::string& profileId)
{
    return db.select("id",
                     "conversations",
                     "participant_id=:participant_id",
                     {{":participant_id", profileId}}).payloads;
}

std::vector<std::string>
getPeerParticipantsForConversation(Database& db, const std::string& profileId, const std::string& conversationId)
{
    return db.select("participant_id",
                     "conversations",
                     "id=:id AND participant_id!=:participant_id",
                     {{":id", conversationId}, {":participant_id", profileId}}).payloads;
}

std::string
getAvatarForProfileId(Database& db, const std::string& profileId)
{
    auto returnFromDb = db.select("photo",
                                  "profiles",
                                  "id=:id",
                                  {{":id", profileId}});
    if (returnFromDb.nbrOfCols == 1 && returnFromDb.payloads.size() >= 1) {
      auto payloads = returnFromDb.payloads;
      return payloads[0];
    }
    return "";
}

std::string
getAliasForProfileId(Database& db, const std::string& profileId)
{
    auto returnFromDb = db.select("alias",
                                  "profiles",
                                  "id=:id",
                                  {{":id", profileId}});
    if (returnFromDb.nbrOfCols == 1 && returnFromDb.payloads.size() >= 1) {
      auto payloads = returnFromDb.payloads;
      return payloads[0];
    }
    return "";
}

bool
profileCouldBeRemoved(Database& db, const std::string& profileId)
{
    auto returnFromDb = db.select("account_id",
                                  "profiles_accounts",
                                  "profile_id=:profile_id",
                                  {{":profile_id", profileId}});
    if (returnFromDb.nbrOfCols == 1 && returnFromDb.payloads.size() >= 1) {
        return false;
    }
    return true;
}

void
setAliasForProfileId(Database& db, const std::string& profileId, const std::string& alias)
{
    db.update("profiles",
              "alias=:alias",
              {{":alias", alias}},
              "id=:id",
              {{":id", profileId}});
}

void
setAvatarForProfileId(Database& db, const std::string& profileId, const std::string& avatar)
{
    db.update("profiles",
              "photo=:photo",
              {{":photo", avatar}},
              "id=:id",
              {{":id", profileId}});
}

api::contact::Info
buildContactFromProfileId(Database& db, const std::string& profileId)
{
    auto returnFromDb = db.select("uri, photo, alias, type",
                                  "profiles",
                                  "id=:id",
                                  {{":id", profileId}});
    if (returnFromDb.nbrOfCols == 4 && returnFromDb.payloads.size() >= 4) {
      auto payloads = returnFromDb.payloads;

      api::profile::Info profileInfo = {payloads[0], payloads[1], payloads[2], api::profile::to_type(payloads[3])};

      return {profileInfo, "", true, false};
    }
    return api::contact::Info();
}

std::vector<std::string>
getConversationsBetween(Database& db, const std::string& accountProfile, const std::string& contactProfile)
{
    auto conversationsForAccount = getConversationsForProfile(db, accountProfile);
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    auto conversationsForContact = getConversationsForProfile(db, contactProfile);
    std::sort(conversationsForContact.begin(), conversationsForContact.end());
    std::vector<std::string> common;

    std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                       conversationsForContact.begin(), conversationsForContact.end(),
                       std::back_inserter(common));
    return common;
}

std::string
beginConversationsBetween(Database& db, const std::string& accountProfile, const std::string& contactProfile, const std::string& firstMessage)
{
    // Add conversation between account and profile
    auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1",
                                        "conversations",
                                        "1=1",
                                        {}).payloads[0];
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", accountProfile}});
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", contactProfile}});
    // Add first interaction
    if (!firstMessage.empty())
        db.insertInto("interactions",
                      {{":account_id", "account_id"}, {":author_id", "author_id"},
                      {":conversation_id", "conversation_id"}, {":timestamp", "timestamp"},
                      {":body", "body"}, {":type", "type"},
                      {":status", "status"}},
                      {{":account_id", accountProfile}, {":author_id", accountProfile},
                      {":conversation_id", newConversationsId},
                      {":timestamp", std::to_string(std::time(nullptr))},
                      {":body", firstMessage}, {":type", "CONTACT"},
                      {":status", "SUCCEED"}});
    return newConversationsId;
}

void
getHistory(Database& db, api::conversation::Info& conversation)
{
    auto accountProfile = getProfileId(db, conversation.accountId, "true");
    auto interactionsResult = db.select("id, author_id, body, timestamp, type, status",
                                    "interactions",
                                    "conversation_id=:conversation_id AND account_id=:account_id",
                                    {{":conversation_id", conversation.uid}, {":account_id", accountProfile}});
    if (interactionsResult.nbrOfCols == 6) {
        auto payloads = interactionsResult.payloads;
        for (decltype(payloads.size()) i = 0; i < payloads.size(); i += 6) {
            auto msg = api::interaction::Info({payloads[i + 1], payloads[i + 2],
                                         std::stoi(payloads[i + 3]),
                                         api::interaction::to_type(payloads[i + 4]),
                                         api::interaction::to_status(payloads[i + 5])});
            conversation.interactions.emplace(std::stoull(payloads[i]), std::move(msg));
            conversation.lastMessageUid = std::stoull(payloads[i]);
        }
    }
}

int
addMessageToConversation(Database& db,
                         const std::string& accountProfile,
                         const std::string& conversationId,
                         const api::interaction::Info& msg)
{
    return db.insertInto("interactions",
                          {{":account_id", "account_id"}, {":author_id", "author_id"},
                          {":conversation_id", "conversation_id"}, {":timestamp", "timestamp"},
                          {":body", "body"}, {":type", "type"},
                          {":status", "status"}},
                          {{":account_id", accountProfile}, {":author_id", msg.authorUri},
                          {":conversation_id", conversationId},
                          {":timestamp", std::to_string(msg.timestamp)},
                          {":body", msg.body}, {":type", to_string(msg.type)},
                          {":status", to_string(msg.status)}});
}

int
addDataTransferToConversation(Database& db,
                              const std::string& accountProfileId,
                              const std::string& conversationId,
                              const api::datatransfer::Info& infoFromDaemon)
{
    auto peerProfileId = getProfileId(db, infoFromDaemon.accountId, "false",
        infoFromDaemon.peerUri);

    return db.insertInto("interactions", {
            {":account_id", "account_id"},
            {":author_id", "author_id"},
            {":conversation_id", "conversation_id"},
            {":timestamp", "timestamp"},
            {":body", "body"},
            {":type", "type"},
            {":status", "status"}
        }, {
            {":account_id", accountProfileId},
            {":author_id", infoFromDaemon.isOutgoing? accountProfileId : peerProfileId},
            {":conversation_id", conversationId},
            {":timestamp", std::to_string(std::time(nullptr))},
            {":body", infoFromDaemon.path},
            {":type", infoFromDaemon.isOutgoing ?
                    "OUTGOING_DATA_TRANSFER" :
                    "INCOMING_DATA_TRANSFER"},
            {":status", "TRANSFER_CREATED"}
        });
}

int
addOrUpdateMessage(Database& db,
                         const std::string& accountProfile,
                         const std::string& conversationId,
                         const api::interaction::Info& msg,
                         const std::string& daemonId)
{
    // Check if profile is already present.
    auto msgAlreadyExists = db.select("id",
                                      "interactions",
                                      "daemon_id=:daemon_id",
                                       {{":daemon_id", daemonId}});
    if (msgAlreadyExists.payloads.empty()) {
        return db.insertInto("interactions",
                              {{":account_id", "account_id"}, {":author_id", "author_id"},
                              {":conversation_id", "conversation_id"}, {":timestamp", "timestamp"},
                              {":body", "body"}, {":type", "type"}, {":daemon_id", "daemon_id"},
                              {":status", "status"}},
                              {{":account_id", accountProfile}, {":author_id", msg.authorUri},
                              {":conversation_id", conversationId},
                              {":timestamp", std::to_string(msg.timestamp)},
                              {":body", msg.body}, {":type", to_string(msg.type)}, {":daemon_id", daemonId},
                              {":status", to_string(msg.status)}});
    } else {
        // already exists
        db.update("interactions",
                  "body=:body",
                  {{":body", msg.body}},
                  "daemon_id=:daemon_id",
                   {{":daemon_id", daemonId}});
        return std::stoi(msgAlreadyExists.payloads[0]);
    }

}

void addDaemonMsgId(Database& db,
                    const std::string& interactionId,
                    const std::string& daemonId)
{
    db.update("interactions", "daemon_id=:daemon_id",
              {{":daemon_id", daemonId}},
              "id=:id", {{":id", interactionId}});
}

std::string getDaemonIdByInteractionId(Database& db, const std::string& id)
{
    auto ids = db.select("daemon_id", "interactions", "id=:id", {{":id", id}}).payloads;
    return ids.empty() ? "" : ids[0];
}

std::string getInteractionIdByDaemonId(Database& db, const std::string& id)
{
    auto ids = db.select("id", "interactions", "daemon_id=:daemon_id", {{":daemon_id", id}}).payloads;
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
    if (result.nbrOfCols == 1) {
        auto payloads = result.payloads;
        return payloads[0];
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

void clearAllHistoryFor(Database& db, const std::string& accountId)
{
    auto profileId = getProfileId(db, accountId, "true");

    if (profileId.empty())
        return;

    db.deleteFrom("interactions", "account_id=:account_id", {{":account_id", profileId}});
}

void
removeContact(Database& db, const std::string& contactUri, const std::string& accountId)
{
    // Get profile for contact
    auto contactId = getProfileId(db, accountId, "false", contactUri);
    if (contactId.empty()) return; // No profile
    auto accountProfileId = getProfileId(db, accountId, "true");
    // Get common conversations
    auto conversations = getConversationsBetween(db, accountProfileId, contactId);
    // Remove conversations + interactions
    for (const auto& conversationId: conversations) {
        // Remove conversation
        db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
        // clear History
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
    }
    // Get conversations for this contact.
    conversations = getConversationsForProfile(db, contactId);
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles_accounts",
        "profile_id=:profile_id AND account_id=:account_id AND is_account=:is_account",
        {{":profile_id", contactId},
        {":account_id", accountId},
        {":is_account", "false"}});
        if (profileCouldBeRemoved(db, contactId))
        db.deleteFrom("profiles", "id=:id", {{":id", contactId}});
    }
}

void
removeAccount(Database& db, const std::string& accountId)
{
    auto accountProfileId = database::getProfileId(db, accountId, "true");
    auto conversationsForAccount = getConversationsForProfile(db, accountProfileId);
    for (const auto& convId: conversationsForAccount) {
        auto peers = getPeerParticipantsForConversation(db, accountProfileId, convId);
        db.deleteFrom("conversations", "id=:id", {{":id", convId}});
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", convId}});
        for (const auto& peerId: peers) {
            auto otherConversationsForProfile = getConversationsForProfile(db, peerId);
            if (otherConversationsForProfile.empty()) {
                db.deleteFrom("profiles_accounts",
                "profile_id=:profile_id AND account_id=:account_id AND is_account=:is_account",
                {{":profile_id", peerId},
                {":account_id", accountId},
                {":is_account", "false"}});
                if (profileCouldBeRemoved(db, peerId)) {
                    db.deleteFrom("profiles", "id=:id", {{":id", peerId}});
                }
            }
        }
    }
    db.deleteFrom("profiles_accounts",
    "profile_id=:profile_id AND account_id=:account_id AND is_account=:is_account",
    {{":profile_id", accountProfileId},
    {":account_id", accountId},
    {":is_account", "true"}});
    db.deleteFrom("profiles", "id=:id", {{":id", accountProfileId}});
}

void
addContact(Database& db, const std::string& contactUri, const std::string& accountId)
{
    // Get profile for contact
    auto row = getOrInsertProfile(db, contactUri, accountId, false, "", "");
    if (row.empty()) {
        qDebug() << "database::addContact, no profile for contact. abort";
        return;
    }
    // Get profile of the account linked
    auto accountProfileId = getProfileId(db, accountId, "true");
    // Get if conversation exists
    auto common = getConversationsBetween(db, accountProfileId, row);
    if (common.empty()) {
        // conversations doesn't exists, start it.
        beginConversationsBetween(db, accountProfileId, row);
    }
}

int
countUnreadFromInteractions(Database& db, const std::string& conversationId)
{
    return db.count("status", "interactions", "status=:status AND conversation_id=:id",
           {{":status", "UNREAD"}, {":id", conversationId}});
}

void
deleteObsoleteHistory(Database& db, long int date)
{
    db.deleteFrom("interactions", "timestamp<=:date", {{":date", std::to_string(date)}});
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
        qDebug() << "database::getLastTimestamp, stoull throws an out_of_range exception: " << e.what();
    } catch (const std::invalid_argument& e) {
        qDebug() << "database::getLastTimestamp, stoull throws an invalid_argument exception: " << e.what();
    }
    return result;
}

std::vector<std::shared_ptr<Database>>
migrateLegacyDatabaseIfNeeded(const QStringList& accountIds)
{
    using namespace lrc::api;

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
    for (auto accountId : accountIds) {
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

            auto accountProfile = legacyDb->select(
                "photo, alias, id",
                "profiles", "uri=:uri",
                { {":uri", accountUri} }).payloads;
            profile::Info accountProfileInfo;
            // if we can not find the uri in the database
            // (in the case of poorly kept SIP account uris),
            // than we cannot migrate the conversations and vcard
            if (accountProfile.empty()) {
                continue;
            }
            accountProfileInfo = { accountUri, accountProfile[0], accountProfile[1],
                    isRingAccount ? profile::Type::RING : profile::Type::SIP };
            std::string accountProfileId = accountProfile[2];
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
            // the full canonical uri CAN be constructed and used for
            // the 'participant' column of the database, but provides
            // no further distinction

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
                auto peerProfileId = getPeerParticipantsForConversation(*legacyDb, accountProfileId, conversationId);
                if (peerProfileId.empty()) {
                    continue;
                }
                auto profileUriIter = profileIdUriMap.find(peerProfileId.at(0));
                // we cannot insert in the conversations table without a uri
                if (profileUriIter == profileIdUriMap.end()) {
                    continue;
                }
                try {
                    db.insertInto("conversations",
                        {   {":id", "id"} ,
                            {":participant", "participant"} },
                        {   { ":id", conversationId } ,
                            { ":participant", profileUriIter->second } });
                } catch (const std::runtime_error& e) {
                    qWarning() << e.what();
                }
            }

            // 4. interactions
            auto account_profile_id = legacyDb->select(
                "id", "accounts_profiles",
                "account_id=:account_id",
                { {":account_id", accountId.toStdString()} }).payloads;
            if (account_profile_id.empty()) {
                // this should never happen, but will
                continue;
            }
            auto allInteractions = legacyDb->select(
                "account_id, author_id, conversation_id, \
                 timestamp, body, type, status, daemon_id",
                "interactions",
                "account_id=:account_id",
                { {":account_id", account_profile_id[0]} });
            auto interactionIt = allInteractions.payloads.begin();
            while (interactionIt != allInteractions.payloads.end()) {
                std::string duration{ "42" }, profileUri{ "cedric" }, isIncoming{ "true" };
                try {
                    db.insertInto("interactions", {
                        {":author", "author"},
                        {":conversation", "conversation"},
                        {":timestamp", "timestamp"},
                        {":duration", "duration"},
                        {":body", "body"},
                        {":type", "type"},
                        {":status", "status"},
                        {":is_incoming", "is_incoming"},
                        {":daemon_id", "daemon_id"}
                    }, {
                        {":author", profileUri},
                        {":conversation", *(interactionIt + 2)},
                        {":timestamp", *(interactionIt + 3)},
                        {":duration", duration},
                        {":body", *(interactionIt + 4)},
                        {":type", *(interactionIt + 5)},
                        {":status", *(interactionIt + 6)},
                        {":is_incoming", isIncoming},
                        {":daemon_id", *(interactionIt + 7)}
                    });
                } catch (const std::runtime_error& e) {
                    qWarning() << e.what();
                }
                std::advance(interactionIt, allInteractions.nbrOfCols);
            }
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

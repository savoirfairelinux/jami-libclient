/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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
#include "databasehelper.h"
#include "api/profile.h"
#include "api/datatransfer.h"

#include <datatransfer_interface.h>

namespace lrc
{

namespace authority
{

namespace database
{

std::string
getProfileId(Database& db, const std::string& uri)
{
    auto ids = db.select("id", "profiles","uri=:uri", {{":uri", uri}}).payloads;
    return ids.empty() ? "" : ids[0];
}

std::string
getOrInsertProfile(Database& db,
                   const std::string& contactUri,
                   const std::string& alias,
                   const std::string& avatar,
                   const std::string& type)
{
    // Check if profile is already present.
    auto profileAlreadyExists = db.select("id",
                                           "profiles",
                                           "uri=:uri",
                                           {{":uri", contactUri}});
    if (profileAlreadyExists.payloads.empty()) {
       // Doesn't exists, add contact to the database
       auto row = db.insertInto("profiles",
       {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
       {":status", "status"}},
       {{":uri", contactUri}, {":alias", alias}, {":photo", avatar}, {":type", type},
       {":status", "TRUSTED"}});

       if (row == -1) {
           qDebug() << "contact not added to the database";
           return "";
       }

       return std::to_string(row);
    } else {
       // Exists, update and retrieve it.
       if (!avatar.empty() && !alias.empty()) {
           db.update("profiles",
                     "alias=:alias, photo=:photo",
                     {{":alias", alias}, {":photo", avatar}},
                     "uri=:uri", {{":uri", contactUri}});
       } else if (!avatar.empty()) {
           db.update("profiles",
                     "photo=:photo",
                     {{":photo", avatar}},
                     "uri=:uri", {{":uri", contactUri}});
       }
       return profileAlreadyExists.payloads[0];
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
    auto interactionsResult = db.select("id, author_id, body, timestamp, type, status",
                                    "interactions",
                                    "conversation_id=:conversation_id",
                                    {{":conversation_id", conversation.uid}});
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
    auto peerProfileId = getProfileId(db, infoFromDaemon.peerUri);

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

void clearAllHistoryFor(Database& db, const std::string& accountUri)
{
    auto accountId = db.select("id", "profiles","uri=:uri", {{":uri", accountUri}}).payloads;

    if (accountId.empty())
        return;

    db.deleteFrom("interactions", "account_id=:account_id", {{":account_id", accountId[0]}});
}

void
removeContact(Database& db, const std::string& accountUri, const std::string& contactUri)
{
    // Get profile for contact
    auto contactId = db.select("id", "profiles","uri=:uri", {{":uri", contactUri}}).payloads;
    if (contactId.empty()) return; // No profile
    // Get common conversations
    auto accountProfileId = getProfileId(db, accountUri);
    auto conversations = getConversationsBetween(db, accountProfileId, contactId[0]);
    // Remove conversations + interactions
    for (const auto& conversationId: conversations) {
        // Remove conversation
        db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
        // clear History
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
    }
    // Get conversations for this contact.
    conversations = getConversationsForProfile(db, contactId[0]);
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles", "id=:id", {{":id", contactId[0]}});
    }
}

void
removeAccount(Database& db, const std::string& accountUri)
{
    auto accountProfileId = database::getProfileId(db, accountUri);
    auto conversationsForAccount = getConversationsForProfile(db, accountProfileId);
    for (const auto& convId: conversationsForAccount) {
        auto peers = getPeerParticipantsForConversation(db, accountProfileId, convId);
        db.deleteFrom("conversations", "id=:id", {{":id", convId}});
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", convId}});
        for (const auto& peerId: peers) {
            auto otherConversationsForProfile = getConversationsForProfile(db, peerId);
            if (otherConversationsForProfile.empty()) {
                db.deleteFrom("profiles", "id=:id", {{":id", peerId}});
            }
        }
    }
    db.deleteFrom("profiles", "id=:id", {{":id", accountProfileId}});
}

void
addContact(Database& db, const std::string& accountUri, const std::string& contactUri)
{
    // Get profile for contact
    auto row = getOrInsertProfile(db, contactUri);
    if (row.empty()) {
        qDebug() << "database::addContact, no profile for contact. abort";
        return;
    }
    // Get profile of the account linked
    auto accountProfileId = getProfileId(db, accountUri);
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
    return db.count("status", "interactions", "status='UNREAD' AND conversation_id='" + conversationId + "'");
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

} // namespace database

} // namespace authority

} // namespace lrc

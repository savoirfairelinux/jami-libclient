#pragma once

// Lrc
#include "api/account.h"
#include "api/contact.h"
#include "database.h"
#include "dbus/configurationmanager.h"

namespace lrc
{

namespace authority
{

namespace database
{

/**
 * @note the profile must be in the database or it will fails
 * @return the id in the database for the current profile
 */
std::string getProfileId(Database& db, const std::string& uri)
{
    return db.select("id", "profiles","uri=:uri", {{":uri", uri}}).payloads[0];
}

/**
 * @param db
 * @param contactUri
 * @param alias
 * @param avatar
 * @return the id in the database for the current profile
 */
std::string getOrInsertProfile(Database& db,
                               const std::string& contactUri,
                               const std::string& alias = "",
                               const std::string& avatar = "",
                               const std::string& type = "INVALID")
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
       if (!avatar.empty() || !alias.empty()) {
           db.update("profiles",
                     "alias=:alias, photo=:photo",
                     {{":alias", alias}, {":photo", avatar}},
                     "uri=:uri", {{":uri", contactUri}});
       }
       return profileAlreadyExists.payloads[0];
    }
}

/**
 * @param db
 * @param accountProfile the id of the account in the database
 * @param contactProfile the id of the contact in the database
 * @return conversations id for conversations between account and contact
 */
std::vector<std::string> getConversationsBetween(Database& db,
                                                 const std::string& accountProfile,
                                                 const std::string& contactProfile)
{
    auto conversationsForAccount = db.select("id",
                                             "conversations",
                                             "participant_id=:participant_id",
                                            {{":participant_id", accountProfile}}).payloads;
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    auto conversationsForContact = db.select("id",
                                             "conversations",
                                             "participant_id=:participant_id",
                                             {{":participant_id", contactProfile}}).payloads;

    std::sort(conversationsForContact.begin(), conversationsForContact.end());
    std::vector<std::string> common;

    std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                       conversationsForContact.begin(), conversationsForContact.end(),
                       std::back_inserter(common));
    return common;
}

/**
 * Start a conversation between account and contact. Creates an entry in the conversations table
 * and an entry in the interactions table.
 * @param db
 * @param accountProfile the id of the account in the database
 * @param contactProfile the id of the contact in the database
 * @return conversation_id of the new conversation.
 */
std::string beginConversationsBetween(Database& db,
                                      const std::string& accountProfile,
                                      const std::string& contactProfile)
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
    // Add "Conversation started" message
    db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountProfile}, {":author_id", accountProfile},
                  {":conversation_id", newConversationsId}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", "0"},
                  {":body", "Conversation started"}, {":type", "CONTACT"},
                  {":status", "SUCCEED"}});
    return newConversationsId;
}

/**
 * Remove a conversation between current account and a contact. Remove corresponding entries in
 * the conversations table and profiles if the profile is linked to no conversations.
 * @param db
 * @param contactUri
 */
void removeContact(Database& db, const std::string& accountUri, const std::string& contactUri)
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
    conversations = db.select("id", "conversations","participant_id=:id", {{":id", contactId[0]}}).payloads;
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles", "id=:id", {{":id", contactId[0]}});
    }
}

/**
 * insert profiles and begin conversations
 * @param db
 * @param accountUri
 * @param contactUri
 */
void addContact(Database& db, const std::string& accountUri, const std::string& contactUri)
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


} // namespace database

} // namespace authority

} // namespace lrc

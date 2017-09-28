// [jn] add copyright
#include "databasehelper.h"
#include "api/profile.h"

namespace lrc
{

namespace authority
{

namespace database
{

std::string getProfileId(Database& db, const std::string& uri)
{
    auto ids = db.select("id", "profiles","uri=:uri", {{":uri", uri}}).payloads;
    return ids.empty() ? "" : ids[0];
}

std::string getOrInsertProfile(Database& db,
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
       if (!avatar.empty() || !alias.empty()) {
           db.update("profiles",
                     "alias=:alias, photo=:photo",
                     {{":alias", alias}, {":photo", avatar}},
                     "uri=:uri", {{":uri", contactUri}});
       }
       return profileAlreadyExists.payloads[0];
    }
}

std::vector<std::string> getConversationsForProfile(Database& db,
                                                    const std::string& profileId)
{
    return db.select("id",
                     "conversations",
                     "participant_id=:participant_id",
                     {{":participant_id", profileId}}).payloads;
}

std::vector<std::string> getPeerParticipantsForConversation(Database& db,
                                                            const std::string& profileId,
                                                            const std::string& conversationId)
{
    return db.select("participant_id",
                     "conversations",
                     "id=:id AND participant_id!=:participant_id",
                     {{":id", conversationId}, {":participant_id", profileId}}).payloads;
}

api::contact::Info buildContactFromProfileId(Database& db, const std::string& profileId)
{
    auto returnFromDb = db.select("uri, photo, alias, type",
                                  "profiles",
                                  "id=:id",
                                  {{":id", profileId}});
    if (returnFromDb.nbrOfCols == 4 && returnFromDb.payloads.size() >= 4) {
      auto payloads = returnFromDb.payloads;

      api::profile::Info profileInfo = {payloads[0], payloads[1], payloads[2], api::profile::StringToType(payloads[3])};

      return {profileInfo ,"" ,true, false};
    }
    return api::contact::Info();
}

std::vector<std::string> getConversationsBetween(Database& db,
                                                 const std::string& accountProfile,
                                                 const std::string& contactProfile)
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
    // Add "Conversation started" interaction
    db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountProfile}, {":author_id", accountProfile},
                  {":conversation_id", newConversationsId}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", std::to_string(std::time(nullptr))},
                  {":body", "Conversation started"}, {":type", "CONTACT"},
                  {":status", "SUCCEED"}});
    return newConversationsId;
}

void getHistory(Database& db, api::conversation::Info& conversation)
{
    auto interactionsResult = db.select("id, author_id, body, timestamp, type, status",
                                    "interactions",
                                    "conversation_id=:conversation_id",
                                    {{":conversation_id", conversation.uid}});
    if (interactionsResult.nbrOfCols == 6) {
        auto payloads = interactionsResult.payloads;
        for (auto i = 0; i < payloads.size(); i += 6) {
            auto msg = api::interaction::Info({payloads[i + 1], payloads[i + 2],
                                         std::stoi(payloads[i + 3]),
                                         api::interaction::StringToType(payloads[i + 4]),
                                         api::interaction::StringToStatus(payloads[i + 5])});
            conversation.interactions.emplace(std::stoull(payloads[i]), std::move(msg));
            conversation.lastMessageUid = std::stoull(payloads[i]);
        }
    }
}

int addMessageToConversation(Database& db,
                              const std::string& accountProfile,
                              const std::string& conversationId,
                              const api::interaction::Info& msg)
{
    return db.insertInto("interactions",
                          {{":account_id", "account_id"}, {":author_id", "author_id"},
                          {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                          {":group_id", "group_id"}, {":timestamp", "timestamp"},
                          {":body", "body"}, {":type", "type"},
                          {":status", "status"}},
                          {{":account_id", accountProfile}, {":author_id", msg.authorUri},
                          {":conversation_id", conversationId}, {":device_id", "0"},
                          {":group_id", "0"}, {":timestamp", std::to_string(msg.timestamp)},
                          {":body", msg.body}, {":type", TypeToString(msg.type)},
                          {":status", StatusToString(msg.status)}});
}

void changeInteractionId(Database& db, unsigned int oldId, unsigned int newId)
{
    db.update("interactions", "id=:id", {{":id", std::to_string(newId)}},
              "id=:oldid", {{":oldid",  std::to_string(oldId)}});
}

void updateInteractionStatus(Database& db, unsigned int id,
                             api::interaction::Status& newStatus)
{
    db.update("interactions", "status=:status",
              {{":status", api::interaction::StatusToString(newStatus)}},
              "id=:id", {{":id",  std::to_string(id)}});
}

void clearHistory(Database& db,
                  const std::string& conversationId)
{
    db.deleteFrom("interactions", "conversation_id=:id AND type!='CONTACT'", {{":id", conversationId}});
}

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
    conversations = getConversationsForProfile(db, contactId[0]);
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles", "id=:id", {{":id", contactId[0]}});
    }
}

void removeAccount(Database& db, const std::string& accountUri)
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

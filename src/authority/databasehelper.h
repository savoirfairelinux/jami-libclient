#pragma once

// Lrc
#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/interaction.h"
#include "database.h"
#include "dbus/configurationmanager.h"

namespace lrc
{

namespace authority
{

namespace database
{

/**
 * @return the id in the database for the current profile ("" if no id)
 */
std::string getProfileId(Database& db, const std::string& uri);

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
                               const std::string& type = "INVALID");

/**
 * Get conversations for a given profile
 * @param db
 * @param profileId
 */
std::vector<std::string> getConversationsForProfile(Database& db,
                                                    const std::string& profileId);

/**
 * Get peer participant for a conversation with profileId
 * NOTE: we don't verify if profileId is in the conversation
 * @param db
 * @param profileId
 * @param conversationId
 */
std::vector<std::string> getPeerParticipantsForConversation(Database& db,
                                                            const std::string& profileId,
                                                            const std::string& conversationId);

api::contact::Info buildContactFromProfileId(Database& db, const std::string& profileId);

/**
 * @param db
 * @param accountProfile the id of the account in the database
 * @param contactProfile the id of the contact in the database
 * @return conversations id for conversations between account and contact
 */
std::vector<std::string> getConversationsBetween(Database& db,
                                                 const std::string& accountProfile,
                                                 const std::string& contactProfile);

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
                                      const std::string& contactProfile);

/**
 * Return interactions for a conversation
 * @param  db
 * @param  conversation to modify
 */
void getHistory(Database& db, api::conversation::Info& conversation);

/**
 * Add a interaction into a conversation
 * @param  db
 * @param  accountProfile
 * @param  conversationId
 * @param  msg
 * @return the id of the inserted interaction
 */
int addMessageToConversation(Database& db,
                              const std::string& accountProfile,
                              const std::string& conversationId,
                              const api::interaction::Info& msg);

/**
 * Clear history but not the conversation started interaction
 * @param  db
 * @param  conversationId
 */
void clearHistory(Database& db,
                  const std::string& conversationId);

/**
 * Remove a conversation between current account and a contact. Remove corresponding entries in
 * the conversations table and profiles if the profile is linked to no conversations.
 * @param db
 * @param contactUri
 */
void removeContact(Database& db, const std::string& accountUri, const std::string& contactUri);

/**
 * Remove conversations and profiles linked to an account
 * @param db
 * @param accountUri
 */
void removeAccount(Database& db, const std::string& accountUri);

/**
 * insert profiles and begin conversations
 * @param db
 * @param accountUri
 * @param contactUri
 */
void addContact(Database& db, const std::string& accountUri, const std::string& contactUri);


} // namespace database

} // namespace authority

} // namespace lrc

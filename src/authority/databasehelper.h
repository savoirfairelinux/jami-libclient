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

namespace api { namespace datatransfer {
struct Info;
}}

namespace authority
{

namespace database
{

/**
 * Get id from database for a given uri
 * @param db
 * @param uri
 * @return the id
 * @note "" if no id
 */
std::string getProfileId(Database& db, const std::string& uri);

/**
 * Get id for a profile. If the profile doesn't exist, create it.
 * @param db
 * @param contactUri
 * @param alias
 * @param avatar
 * @return the id
 */
std::string getOrInsertProfile(Database& db,
                               const std::string& contactUri,
                               const std::string& alias = "",
                               const std::string& avatar = "",
                               const std::string& type = "INVALID");

/**
 * Get conversations for a given profile.
 * @param db
 * @param profileId
 */
std::vector<std::string> getConversationsForProfile(Database& db,
                                                    const std::string& profileId);

/**
 * Get peer participant for a conversation linked to a profile.
 * @param db
 * @param profileId
 * @param conversationId
 * @note we don't verify if profileId is in the conversation
 */
std::vector<std::string> getPeerParticipantsForConversation(Database& db,
                                                            const std::string& profileId,
                                                            const std::string& conversationId);

/**
 * @param  db
 * @param  profileId
 * @return the avatar in the database for a profile
 */
std::string getAvatarForProfileId(Database& db, const std::string& profileId);

api::contact::Info buildContactFromProfileId(Database& db, const std::string& profileId);

/**
 * Get conversations shared between an account and a contact.
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
 * @param firstMessage the body of the first message
 * @return conversation_id of the new conversation.
 */
std::string beginConversationsBetween(Database& db,
                                      const std::string& accountProfile,
                                      const std::string& contactProfile,
                                      const std::string& firstMessage = "");

/**
 * Return interactions from a conversation
 * @param  db
 * @param  conversation to modify
 */
void getHistory(Database& db, api::conversation::Info& conversation);

/**
 * Add an entry into interactions linked to a conversation.
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
* Add or update an entry into interactions linked to a conversation.
* @param  db
* @param  accountProfile
* @param  conversationId
* @param  msg
* @param  daemonId
* @return the id of the inserted interaction
*/
int addOrUpdateMessage(Database& db,
                       const std::string& accountProfile,
                       const std::string& conversationId,
                       const api::interaction::Info& msg,
                       const std::string& daemonId);

/**
 * Change the daemon_id column for an interaction
 * @param db
 * @param interactionId
 * @param daemonId
 */
void addDaemonMsgId(Database& db,
                    const std::string& interactionId,
                    const std::string& daemonId);

/**
 * @param  db
 * @param  id
 * @return the daemon id for an interaction else an empty string
 */
std::string getDaemonIdByInteractionId(Database& db, const std::string& id);

/**
 * @param  db
 * @param  id
 * @return the interaction id for a daemon id else an empty string
 */
std::string getInteractionIdByDaemonId(Database& db, const std::string& id);

/**
 * Change the body of an interaction
 * @param db
 * @param id
 * @param newBody
 */
void updateInteractionBody(Database& db, unsigned int id,
                           const std::string& newBody);

/**
 * Change the status of an interaction
 * @param db
 * @param id
 * @param newStatus
 */
void updateInteractionStatus(Database& db, unsigned int id,
                             api::interaction::Status newStatus);

/**
 * Clear history but not the conversation started interaction
 * @param  db
 * @param  conversationId
 */
void clearHistory(Database& db,
                  const std::string& conversationId);

/**
 * Clear interaction from history
 * @param  db
 * @param  conversationId
 * @param  interactionId
 */
void clearInteractionFromConversation(Database& db,
                                      const std::string& conversationId,
                                      const uint64_t& interactionId);

/**
 * Clear all history stored in the database for the account uri
 * @param  db
 * @param accountUri
 */
void clearAllHistoryFor(Database& db, const std::string& accountUri);

/**
 * delete obsolete histori from the database
 * @param db
 * @param date in second since epoch. Below this date, interactions will be deleted
 */
void deleteObsoleteHistory(Database& db, long int date);

/**
 * Remove a conversation between an account and a contact. Remove corresponding entries in
 * the conversations table and profiles if the profile is not present in conversations.
 * @param db
 * @param contactUri
 */
void removeContact(Database& db, const std::string& accountUri, const std::string& contactUri);

/**
 * Remove from conversations and profiles linked to an account.
 * @param db
 * @param accountUri
 */
void removeAccount(Database& db, const std::string& accountUri);

/**
 * insert into profiles and conversations.
 * @param db
 * @param accountUri
 * @param contactUri
 */
void addContact(Database& db, const std::string& accountUri, const std::string& contactUri);

/**
 * count number of 'UNREAD' from 'interactions' table.
 */
int countUnreadFromInteractions(Database& db, const std::string& conversationId);

int addDataTransferToConversation(Database& db,
                                  const std::string& accountProfileId,
                                  const std::string& conversationId,
                                  const api::datatransfer::Info& infoFromDaemon);

std::string conversationIdFromInteractionId(Database& db, unsigned int interactionId);

} // namespace database

} // namespace authority

} // namespace lrc

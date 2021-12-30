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
#pragma once

// Lrc
#include "api/account.h"
#include "api/contact.h"
#include "api/conversation.h"
#include "api/interaction.h"
#include "database.h"
#include "dbus/configurationmanager.h"

namespace lrc {

namespace api {
namespace datatransfer {
struct Info;
}
} // namespace api

namespace authority {

namespace storage {

/**
 * Get the base path for the application's local storage
 * @return local storage path
 */
QString getPath();

/**
 * Get a formatted for local storage
 * @param uri that may have a scheme prefixed
 * @param type of account for which to transform the uri
 * @return formatted uri
 */
QString prepareUri(const QString& uri, api::profile::Type type);

/**
 * Get a formatted string for a call interaction's body
 * @param author_uri
 * @param duration of the call
 * @return the formatted and translated call message string
 */
QString getCallInteractionString(const QString& authorUri, const std::time_t& duration);

/**
 * Get a formatted string for a contact interaction's body
 * @param author_uri
 * @param status
 * @return the formatted and translated call message string
 */
QString getContactInteractionString(const QString& authorUri,
                                    const api::interaction::Status& status);

namespace vcard {

/**
 * Build the vCard for a profile
 * @param profileInfo
 * @param compressImage
 * @return vcard string of the profile
 */
QString profileToVcard(const api::profile::Info& profileInfo, bool compressImage = false);

/**
 * Set a profile vCard
 * @param accountId
 * @param profileInfo
 * @param isPeer
 */
void setProfile(const QString& accountId,
                const api::profile::Info& profileInfo,
                const bool isPeer = false);

} // namespace vcard

/**
 * @param  duration
 * @return a human readable call duration (M:ss)
 */
QString getFormattedCallDuration(const std::time_t duration);

/**
 * Get all conversations with a given participant's URI
 * @param db
 * @param participant_uri
 */
VectorString getConversationsWithPeer(Database& db, const QString& participant_uri);

/**
 * Get all peer participant(s) URIs for a given conversation id
 * @param db
 * @param conversationId
 */
VectorString getPeerParticipantsForConversation(Database& db, const QString& conversationId);

/**
 * Creates or updates a contact or account vCard file with profile data.
 * @param  accountId
 * @param  profileInfo the contact info containing peer profile information
 * @param  isPeer indicates that a the profileInfo is that of a peer
 */
void createOrUpdateProfile(const QString& accountId,
                           const api::profile::Info& profileInfo,
                           const bool isPeer = false);

/**
 * Remove a profile vCard
 * @param accountId
 * @param peerUri
 */
void removeProfile(const QString& accountId, const QString& peerUri);

/**
 * Gets the account's avatar from the profile.vcf file
 * @param  accountId
 * @return the account's base64 avatar
 */
QString getAccountAvatar(const QString& accountId);

/**
 * Build a contact info struct from a vCard
 * @param  accountId
 * @param  peer_uri
 * @param  type of contact to build
 * @return the contact info containing peer profile information
 */
api::contact::Info buildContactFromProfile(const QString& accountId,
                                           const QString& peer_uri,
                                           const api::profile::Type& type);

/**
 * Get all conversations for an account in the database.
 * @param db
 * @return conversations id for all conversations
 */
VectorString getAllConversations(Database& db);

/**
 * Get conversations shared between an account and a contact.
 * @param db
 * @param accountProfile the id of the account in the database
 * @param contactProfile the id of the contact in the database
 * @return conversations id for conversations between account and contact
 */
VectorString getConversationsBetween(Database& db,
                                     const QString& accountProfile,
                                     const QString& contactProfile);

/**
 * Start a conversation between account and contact. Creates an entry in the conversations table
 * and an entry in the interactions table.
 * @param db
 * @param peer_uri the URI of the peer
 * @param isOutgoing
 * @param timestamp
 * @return conversation_id of the new conversation.
 */
QString beginConversationWithPeer(Database& db,
                                  const QString& peer_uri,
                                  const bool isOutgoing = true,
                                  time_t timestamp = 0);

/**
 * Return interactions from a conversation
 * @param  db
 * @param  conversation to modify
 */
void getHistory(Database& db, api::conversation::Info& conversation);

/**
 * Add an entry into interactions linked to a conversation.
 * @param  db
 * @param  conversationId
 * @param  msg
 * @return the id of the inserted interaction
 */
QString addMessageToConversation(Database& db,
                                 const QString& conversationId,
                                 const api::interaction::Info& msg);

/**
 * Add or update an entry into interactions linked to a conversation.
 * @param  db
 * @param  conversationId
 * @param  msg
 * @param  daemonId
 * @return the id of the inserted interaction
 */
QString addOrUpdateMessage(Database& db,
                           const QString& conversationId,
                           const api::interaction::Info& msg,
                           const QString& daemonId);

/**
 * Add a data transfer entry into interactions linked to a conversation.
 * @param  db
 * @param  conversationId
 * @param  daemonId
 * @return the id of the inserted interaction
 */
QString addDataTransferToConversation(Database& db,
                                      const QString& conversationId,
                                      const api::datatransfer::Info& infoFromDaemon);

/**
 * Change the daemon_id column for an interaction
 * @param db
 * @param interactionId
 * @param daemonId
 */
void addDaemonMsgId(Database& db, const QString& interactionId, const QString& daemonId);

/**
 * @param  db
 * @param  id
 * @return the daemon id for an interaction else an empty string
 */
QString getDaemonIdByInteractionId(Database& db, const QString& id);

/**
 * Obtain the id of an interaction of a given daemon_id
 * @param  db
 * @param  daemon id
 * @return the interaction id for a daemon id else an empty string
 */
QString getInteractionIdByDaemonId(Database& db, const QString& daemon_id);

/**
 * Obtain the extra_data column of an interaction of a given id
 * @note if a key is provided and exists, the value will be returned
 * @param db
 * @param id
 * @param key
 */
QString getInteractionExtraDataById(Database& db, const QString& id, const QString& key = {});

/**
 * update interaction
 * @param db
 * @param daemon id
 * @param interaction
 */
void updateDataTransferInteractionForDaemonId(Database& db,
                                              const QString& daemonId,
                                              api::interaction::Info& interaction);

/**
 * Change the body of an interaction
 * @param db
 * @param id
 * @param newBody
 */
void updateInteractionBody(Database& db, const QString& id, const QString& newBody);

/**
 * Change the status of an interaction
 * @param db
 * @param id
 * @param newStatus
 * @param isRead
 */
void updateInteractionStatus(Database& db, const QString& id, api::interaction::Status newStatus);

/**
 * Set interaction to the read state
 * @param db
 * @param id
 */
void setInteractionRead(Database& db, const QString& id);

/**
 * Clear history but not the conversation started interaction
 * @param  db
 * @param  conversationId
 */
void clearHistory(Database& db, const QString& conversationId);

/**
 * Clear interaction from history
 * @param  db
 * @param  conversationId
 * @param  interactionId
 */
void clearInteractionFromConversation(Database& db,
                                      const QString& conversationId,
                                      const QString& interactionId);

/**
 * Clear all history stored in the interactions table of the database
 * @param  db
 */
void clearAllHistory(Database& db);

/**
 * delete obsolete history from the database
 * @param db
 * @param date in second since epoch. Below this date, interactions will be deleted
 */
void deleteObsoleteHistory(Database& db, long int date);

/**
 * Remove all conversation with a contact. Remove corresponding entries in
 * the conversations table.
 * @param accountId
 * @param db
 * @param contactUri
 */
void removeContactConversations(Database& db, const QString& contactUri);

/**
 * count number of 'UNREAD' from 'interactions' table.
 * @param db
 * @param conversationId
 */
int countUnreadFromInteractions(Database& db, const QString& conversationId);

/**
 * Retrieve an interaction's conversation id
 * @param db
 * @param conversationId
 */
QString conversationIdFromInteractionId(Database& db, const QString& interactionId);

/**
 * Retrieve the last timestamp from the interactions table
 * is used for ConfigurationManager::getLastMessages
 * @param db
 */
uint64_t getLastTimestamp(Database& db);

/**
 * Retrieve a list of account database via a migration
 * procedure from the legacy "ring.db", if it exists
 * @param accountIds of the accounts to attempt migration upon
 * @param willMigrateCb to invoke when migration will occur
 * @param didMigrateCb to invoke when migration has completed
 */
std::vector<std::shared_ptr<Database>> migrateIfNeeded(const QStringList& accountIds,
                                                       MigrationCb& willMigrateCb,
                                                       MigrationCb& didMigrateCb);

} // namespace storage

} // namespace authority

} // namespace lrc

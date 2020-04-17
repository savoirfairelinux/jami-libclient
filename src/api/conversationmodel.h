/****************************************************************************
 *    Copyright (C) 2017-2020 Savoir-faire Linux Inc.                       *
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

#include "typedefs.h"

#include "api/conversation.h"
#include "api/profile.h"
#include "api/datatransfer.h"

#include <QObject>
#include <QVector>
#include <QMap>

#include <memory>
#include <deque>

namespace lrc
{

class CallbacksHandler;
class ConversationModelPimpl;
class Database;

namespace api
{

namespace account { struct Info; }
namespace interaction { struct Info; }

class Lrc;
class BehaviorController;
class NewAccountModel;

enum class ConferenceableItem {
    CALL,
    CONTACT
};

struct AccountConversation
{
    QString convId;
    QString accountId;
};

/*
  * vector of conversationId and accountId.
  * for calls and contacts contain only one element
  * for conferences contains multiple entries
  */

typedef QVector<QVector<AccountConversation>> ConferenceableValue;

/**
  *  @brief Class that manages conversation informations.
  */
class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
public:
    using ConversationQueue = std::deque<conversation::Info>;

    const account::Info& owner;

    ConversationModel(const account::Info& owner,
                      Lrc& lrc,
                      Database& db,
                      const CallbacksHandler& callbacksHandler,
                      const api::BehaviorController& behaviorController);
    ~ConversationModel();

    /**
     * Get conversations which should be shown client side
     * @return conversations filtered with the current filter
     */
    const ConversationQueue& allFilteredConversations() const;

    /**
     * Get conversation for a given identifier
     * @param  conversation id
     * @return conversations with given id
     */
    conversation::Info getConversationForUID(const QString& uid) const;

    /**
     * Get conversations that could be added to conference
     * @param  current conversation id
     * @param  search name filter
     * @return filtered conversations
     */
    QMap<ConferenceableItem, ConferenceableValue> getConferenceableConversations(const QString& convId, const QString& filter = {}) const;
    /**
     * Get a custom filtered set of conversations
     * @return conversations filtered
     */
    const ConversationQueue& getFilteredConversations(const profile::Type& filter = profile::Type::INVALID, bool forceUpdate = false, const bool includeBanned = false) const;
    /**
     * Get the conversation at row in the filtered conversations
     * @param  row
     * @return a copy of the conversation
     */
    conversation::Info filteredConversation(unsigned int row) const;
    /**
     * Emit a filterChanged signal to force the client to refresh the filter. For instance
     * this is required when a contact was banned or un-banned.
     */
    void refreshFilter();
    /**
     * Make permanent a temporary contact or a pending request.
     * Ensure that given conversation is stored permanently into the system.
     * @param uid of the conversation to change.
     * @exception std::out_of_range if uid doesn't correspond to an existing conversation
     */
    void makePermanent(const QString& uid);
    /**
     * Remove a conversation and the contact if it's a dialog
     * @param uid of the conversation
     * @param banned if we want to ban the contact.
     */
    void removeConversation(const QString& uid, bool banned=false);
    /**
     * Get the action wanted by the user when they click on the conversation
     * @param uid of the conversation
     */
    void selectConversation(const QString& uid) const;
    /**
     * Call contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeCall(const QString& uid);
    /**
     * Perform an audio call with contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeAudioOnlyCall(const QString& uid);
    /**
     * Send a message to the conversation
     * @param uid of the conversation
     * @param body of the message
     */
    void sendMessage(const QString& uid, const QString& body);
    /**
     * Modify the current filter (will change the result of getFilteredConversations)
     * @param filter the new filter
     */
    void setFilter(const QString& filter);
    /**
     * Modify the current filter (will change the result of getFilteredConversations)
     * @param filter the new filter (example: PENDING, RING)
     */
    void setFilter(const profile::Type& filter = profile::Type::INVALID);
    /**
     * Join participants from A to B and vice-versa.
     * @note conversations must be in a call.
     * @param uidA uid of the conversation A
     * @param uidB uid of the conversation B
     */
    void joinConversations(const QString& uidA, const QString& uidB);
    /**
     * Clear the history of a conversation
     * @param uid of the conversation
     */
    void clearHistory(const QString& uid);
    /**
     * change the status of the interaction from UNREAD to READ
     * @param convId, id of the conversation
     * @param msgId, id of the interaction
     */
    void setInteractionRead(const QString& convId, const uint64_t& msgId);
    /**
     * Clears the unread text messages of a conversation
     * @param convId, uid of the conversation
     */
    void clearUnreadInteractions(const QString& convId);
    /**
     * clear all history
     */
    void clearAllHistory();
    /**
     * Clear one interaction from the history
     * @param convId
     * @param interactionId
     */
    void clearInteractionFromConversation(const QString& convId, const uint64_t& interactionId);
    /**
     * Retry to send a message. In fact, will delete the previous interaction and resend a new one.
     * @param convId
     * @param interactionId
     */
    void retryInteraction(const QString& convId, const uint64_t& interactionId);
    
    /**
     * @param convId
     * @param interactionId
     * @return whether the interaction is last displayed for the conversation
     */
    bool isLastDisplayed(const QString& convId, const uint64_t& interactionId);
    /**
     * delete obsolete history from the database
     * @param days, number of days from today. Below this date, interactions will be deleted
     */
    void deleteObsoleteHistory(int date);

    void sendFile(const QString& convUid, const QString& path, const QString& filename);

    void acceptTransfer(const QString & convUid, uint64_t interactionId);

    void acceptTransfer(const QString& convUid, uint64_t interactionId, const QString& path);

    void cancelTransfer(const QString& convUid, uint64_t interactionId);

    void getTransferInfo(uint64_t interactionId, api::datatransfer::Info& info);
    /**
     * @param convUid, uid of the conversation
     * @return the number of unread messages for the conversation
     */
    int getNumberOfUnreadMessagesFor(const QString& convUid);
    /**
     * Send a composing status
     * @param uid           conversation's id
     * @param isComposing   if is composing
     */
    void setIsComposing(const QString& uid, bool isComposing);

Q_SIGNALS:
    /**
     * Emitted when a conversation receives a new interaction
     * @param uid of conversation
     * @param interactionId
     * @param interactionInfo
     */
    void newInteraction(const QString& uid, uint64_t interactionId, const interaction::Info& interactionInfo) const;
    /**
     * Emitted when an interaction got a new status
     * @param convUid conversation which owns the interaction
     * @param interactionId
     * @param msg
     */
    void interactionStatusUpdated(const QString& convUid,
                                  uint64_t interactionId,
                                  const api::interaction::Info& msg) const;
    /**
     * Emitted when an interaction got removed from the conversation
     * @param convUid conversation which owns the interaction
     * @param interactionId
     */
    void interactionRemoved(const QString& convUid,
                            uint64_t interactionId) const;
    /**
     * Emitted when user clear the history of a conversation
     * @param uid
     */
    void conversationCleared(const QString& uid) const;
    /**
     * Emitted when conversation's participant has been updated
     * @param uid
     */
    void conversationUpdated(const QString& uid) const;
    /**
     * Emitted when conversations are sorted by last interaction
     */
    void modelSorted() const;
    /**
     * Emitted when filter has changed
     */
    void filterChanged() const;
    /**
     * Emitted when a conversation has been added
     * @param uid
     */
    void newConversation(const QString& uid) const;
    /**
     * Emitted when a conversation has been removed
     * @param uid
     */
    void conversationRemoved(const QString& uid) const;
    /**
     * Emitted after all history were cleared
     * @note the client must connect this signal to know when update the view of the list
     */
    void allHistoryCleared() const;
    /**
     * Emitted at the end of slotContactAdded to notify that an existing conversation can
     * be modified
     * @param uid
     */
    void conversationReady(QString uid) const;
    /**
     * Emitted when a contact in a conversation is composing a message
     * @param uid           conversation's id
     * @param contactUri    contact's uri
     * @param isComposing   if contact is composing a message
     */
    void composingStatusChanged(const QString& uid, const QString& contactUri, bool isComposing) const;

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

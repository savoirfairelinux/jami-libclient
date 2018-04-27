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

// Std
#include <memory>
#include <map>
#include <string>
#include <deque>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

// Data
#include "api/conversation.h"
#include "api/profile.h"
#include "api/datatransfer.h"

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
     * Get a custom filtered set of conversations
     * @return conversations filtered
     */
    const ConversationQueue& getFilteredConversations(const profile::Type& filter = profile::Type::INVALID, const bool includeBanned = false) const;
    /**
     * Get the conversation at row in the filtered conversations
     * @param  row
     * @return a copy of the conversation
     */
    conversation::Info filteredConversation(unsigned int row) const;
    /**
     * Make permanent a temporary contact or a pending request.
     * Ensure that given conversation is stored permanently into the system.
     * @param uid of the conversation to change.
     * @exception std::out_of_range if uid doesn't correspond to an existing conversation
     */
    void makePermanent(const std::string& uid);
    /**
     * Remove a conversation and the contact if it's a dialog
     * @param uid of the conversation
     * @param banned if we want to ban the contact.
     */
    void removeConversation(const std::string& uid, bool banned=false);
    /**
     * Get the action wanted by the user when they click on the conversation
     * @param uid of the conversation
     */
    void selectConversation(const std::string& uid) const;
    /**
     * Call contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeCall(const std::string& uid);
    /**
     * Perform an audio call with contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeAudioOnlyCall(const std::string& uid);
    /**
     * Send a message to the conversation
     * @param uid of the conversation
     * @param body of the message
     */
    void sendMessage(const std::string& uid, const std::string& body);
    /**
     * Modify the current filter (will change the result of getFilteredConversations)
     * @param filter the new filter
     */
    void setFilter(const std::string& filter);
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
    void joinConversations(const std::string& uidA, const std::string& uidB);
    /**
     * Clear the history of a conversation
     * @param uid of the conversation
     */
    void clearHistory(const std::string& uid);
    /**
     * change the status of the interaction from UNREAD to READ
     * @param convId, id of the conversation
     * @param msgId, id of the interaction
     */
    void setInteractionRead(const std::string& convId, const uint64_t& msgId);
    /**
     * Clears the unread text messages of a conversation
     * @param convId, uid of the conversation
     */
    void clearUnreadInteractions(const std::string& convId);
    /**
     * clear all history
     */
     void clearAllHistory();
    /**
     * delete obsolete history from the database
     * @param days, number of days from today. Below this date, interactions will be deleted
     */
    void deleteObsoleteHistory(int date);

    void sendFile(const std::string& convUid, const std::string& path, const std::string& filename);

    void acceptTransfer(const std::string& convUid, uint64_t interactionId, const std::string& path);

    void cancelTransfer(const std::string& convUid, uint64_t interactionId);

    void getTransferInfo(uint64_t interactionId, api::datatransfer::Info& info);
    /**
     * @param convUid, uid of the conversation
     * @return the number of unread messages for the conversation
     */
    int getNumberOfUnreadMessagesFor(const std::string& convUid);

Q_SIGNALS:
    /**
     * Emitted when a conversation receives a new interaction
     * @param uid of conversation
     * @param interactionId
     * @param interactionInfo
     */
    void newInteraction(const std::string& uid, uint64_t interactionId, const interaction::Info& interactionInfo) const;
    /**
     * Emitted when an interaction got a new status
     * @param convUid conversation which owns the interaction
     * @param msgId
     * @param msg
     */
    void interactionStatusUpdated(const std::string& convUid,
                                  uint64_t msgId,
                                  const api::interaction::Info& msg) const;
    /**
     * Emitted when user clear the history of a conversation
     * @param uid
     */
    void conversationCleared(const std::string& uid) const;
    /**
     * Emitted when conversation's participant has been updated
     * @param uid
     */
    void conversationUpdated(const std::string& uid) const;
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
    void newConversation(const std::string& uid) const;
    /**
     * Emitted when a conversation has been removed
     * @param uid
     */
    void conversationRemoved(const std::string& uid) const;
    /**
     * Emitted after all history were cleared
     * @note the client must connect this signal to know when update the view of the list
     */
    void allHistoryCleared() const;

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

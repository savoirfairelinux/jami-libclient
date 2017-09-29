/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
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

namespace lrc
{

class CallbacksHandler;
class ConversationModelPimpl;
class Database;

namespace api
{

namespace account { struct Info; }
namespace interaction { struct Info; }
class NewAccountModel;

/**
  *  @brief Class that manages conversation informations.
  */
class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
public:
    using ConversationQueue = std::deque<conversation::Info>;

    const account::Info& owner;

    ConversationModel(const account::Info& owner, Database& db, const CallbacksHandler& callbacksHandler);
    ~ConversationModel();

    /**
     * Get conversations which should be shown client side
     * @return conversations filtered with the current filter
     */
    const ConversationQueue& getFilteredConversations() const;
    /**
     * Get the conversation at row in the filtered conversations
     * @param  row
     * @return a copy of the conversation
     */
    conversation::Info getConversation(unsigned int row) const;
    /**
     * Add a conversation. If it's with a temporary contact or a pending request
     * the contact will be added and the conversation will be updated
     * @param uid of the conversation to add
     */
    void addConversation(const std::string& uid) const;
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
    void placeCall(const std::string& uid) const;
    /**
     * Send a message to the conversation
     * @param uid of the conversation
     * @param body of the message
     */
    void sendMessage(const std::string& uid, const std::string& body) const;
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
     * Add a new participant to a conversation
     * @param uid conversation linked
     * @param uri peer to add
     */
    void addParticipant(const std::string& uidSrc, const std::string& uidDest);
    /**
     * Clear the history of a conversation
     * @param uid of the conversation
     */
    void clearHistory(const std::string& uid);

Q_SIGNALS:
    /**
     * Emitted when a conversation receives a new interaction
     */
    void newUnreadMessage(const std::string& uid, const interaction::Info& msg) const;
    /**
     * Emiited when user clear the history of a conversation
     */
    void conversationCleared(const std::string& uid) const;
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
     */
    void newConversation(const std::string& uid) const;
    /**
     * Emitted when a conversation has been removed
     */
    void conversationRemoved(const std::string& uid) const;
    void incomingCallFromItem(const unsigned int row);

    void showChatView(const conversation::Info& conversationInfo) const; // TO MOVE
    void showCallView(const conversation::Info& conversationInfo) const;// TO MOVE
    void showIncomingCallView(const conversation::Info& conversationInfo) const;// TO MOVE

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

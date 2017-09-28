/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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

class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
public:
    using ConversationQueue = std::deque<conversation::Info>;

    const account::Info& owner;

    ConversationModel(const account::Info& owner, Database& db, const CallbacksHandler& callbacksHandler);
    ~ConversationModel();

    const ConversationQueue& getFilteredConversations() const;
    conversation::Info getConversation(unsigned int row) const;
    void addConversation(const std::string& uid) const;
    void removeConversation(const std::string& uid, bool banned=false);
    void selectConversation(const std::string& uid) const;
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string& filter);
    void setFilter(const profile::Type& filter = profile::Type::INVALID);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void clearHistory(const std::string& uid);

Q_SIGNALS:
    /**
     * Emitted when a conversation receives a new interaction
     */
    void newUnreadMessage(const std::string& uid, uint64_t msgId, const interaction::Info& msg) const;
    void interactionStatusUpdated(const std::string& convUid,
                                  uint64_t msgId,
                                  const api::interaction::Info& msg) const;
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

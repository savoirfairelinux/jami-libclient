/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>      *
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
#include <string>
#include <deque>

// Qt
#include <qobject.h>

// Data
#include "api/conversation.h"

namespace lrc
{

namespace api
{

using ConversationQueue = std::deque<conversation::Info>;

class ConversationModelI : public QObject {
    Q_OBJECT
public:
    virtual const ConversationQueue& getFilteredConversations() const = 0;
    virtual conversation::Info getConversation(const unsigned int row) const = 0;
    virtual void addConversation(const std::string& uri) const = 0;
    virtual void removeConversation(const std::string& uid) = 0;
    virtual void selectConversation(const std::string& uid) = 0;
    virtual void placeCall(const std::string& uid) const = 0;
    virtual void sendMessage(const std::string& uid, const std::string& body) const = 0;
    virtual void setFilter(const std::string& filter) = 0;
    virtual void addParticipant(const std::string& uid, const::std::string& uri) = 0;
    virtual void clearHistory(const std::string& uid) = 0;

Q_SIGNALS:
    virtual void newMessageAdded(const std::string& uid, const message::Info& msg) = 0;
    virtual void conversationUpdated(unsigned int row) = 0;
    virtual void modelUpdated() const = 0;
    virtual void newContactAdded(const std::string& uid) = 0;
    virtual void incomingCallFromItem(const unsigned int row) = 0;

    virtual void showChatView(const conversation::Info& conversationInfo) = 0;
    virtual void showCallView(const conversation::Info& conversationInfo) = 0;
    virtual void showIncomingCallView(const conversation::Info& conversationInfo) = 0;
};

} // namepsace api
} // namespace lrc

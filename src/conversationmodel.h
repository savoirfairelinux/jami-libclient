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
#include <deque>
#include <map>

// Qt
#include <qobject.h>

// Data
#include "conversation.h"

// Lrc
#include "typedefs.h"

class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
    public:
    explicit ConversationModel(QObject* parent = nullptr);
    ~ConversationModel();

    const Conversations& getConversations() const;
    const Conversation::Info& getConversation(const unsigned int row) const;
    const Conversation::Info& addConversation(const std::string& uri);
    void removeConversation(const std::string& uid);
    /**
     * Select a conversation
     * @param uid the conversation to select
     */
    void selectConversation(const std::string& uid);
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string&);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void cleanHistory(const std::string& uid);

    // signals
    Q_SIGNALS:
    void conversationUpdated(unsigned int row);
    void modelUpdated();
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(const Conversation::Info& conversation);
    void showCallView(const Conversation::Info& conversation);
    void showIncomingCallView(const Conversation::Info& conversation);

    private:
    /**
     * Search a conversation in conversations_
     * @param uid the contact to search
     * @return the contact if found else nullptr
     */
    std::shared_ptr<Conversation::Info> find(const std::string& uid);

    Conversations conversations_;
    mutable Conversations filteredConversations_;
    std::string filter_;

};

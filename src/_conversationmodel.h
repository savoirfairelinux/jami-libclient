/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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

// Qt
#include <qobject.h>

// Data
#include "_conversation.h"

// Std
#include <memory>
#include <deque>
#include <map>

// Lrc
#include "typedefs.h"

//~ typedef std::deque<std::shared_ptr<Conversation>> Conversations;
//~ typedef std::map<std::string, std::shared_ptr<Conversation>> Conversations;
typedef std::deque<std::pair<std::string, std::shared_ptr<Conversation>>> Conversations;


class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
    public:
    explicit ConversationModel(QObject* parent = nullptr);
    ~ConversationModel();

    const Conversations& getConversations() const;
    const Conversation& getConversation(const unsigned int row) const;
    const Conversation& addConversation(const std::string& uri);
    void removeConversation(const std::string& uid) const;
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string&);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void cleanHistory(const std::string& uid);

    // signals
    Q_SIGNALS:
    void itemChanged(unsigned int row);
    void modelUpdated();
    void conversationItemUpdated(const unsigned int row);
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(const Conversation& conversation);
    void showCallView(const Conversation& conversation);
    void showIncomingCallView(const Conversation& conversation);

    private:
    Conversations conversations_;
    mutable Conversations filteredConversations_;
    std::string filter_;

};

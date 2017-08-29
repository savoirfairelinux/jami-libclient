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
#include "data/conversation.h"

// Lrc
#include "contactmodel.h"
#include "typedefs.h"
#include "newcallmodel.h"

namespace lrc
{

class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
public:
    explicit ConversationModel(NewCallModel& callModel,
                               ContactModel& contactModel,
                               const Database& database);
    ~ConversationModel();

    ContactModel& getContactModel() const;
    const ConversationsList& getFilteredConversations() const;
    conversation::Info getConversation(const unsigned int row) const;
    void addConversation(const std::string& uri);
    void removeConversation(const std::string& uid);
    void selectConversation(const std::string& uid);
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string& filter);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void clearHistory(const std::string& uid);

Q_SIGNALS:
    void newMessageAdded(const std::string& uid, message::Info msg);
    void conversationUpdated(unsigned int row);
    void modelUpdated() const;
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(conversation::Info conversationInfo);
    void showCallView(conversation::Info conversationInfo);
    void showIncomingCallView(conversation::Info conversationInfo);

private Q_SLOTS:
    void slotMessageAdded(int uid, const std::string& account, message::Info msg);
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);

private:
    /**
     * Search a conversation in conversations_
     * @param uid the contact to search
     * @return the index in conversations_
     */
    int find(const std::string& uid) const;
    /**
     * Initialize conversations_ and filteredConversations_
     */
    void initConversations();
    /**
     * Sort conversation by last action
     */
    void sortConversations();
    void search();

    NewCallModel& callModel_;
    ContactModel& contactModel_;
    const Database& database_;

    ConversationsList conversations_;
    mutable ConversationsList filteredConversations_;
    std::string filter_;

};

} // namespace lrc

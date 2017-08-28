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
#include "contactmodel.h"
#include "typedefs.h"
#include "newcallmodel.h"

namespace lrc
{

class LIB_EXPORT ConversationModel : public QObject {
    Q_OBJECT
public:
    explicit ConversationModel(std::shared_ptr<NewCallModel>& callModel,
                               std::shared_ptr<ContactModel>& contactModel,
                               const DatabaseManager& dbManager);
    ~ConversationModel();

    std::shared_ptr<ContactModel> getContactModel();
    const ConversationsList& getConversations() const;
    std::shared_ptr<conversation::Info> getConversation(const unsigned int row) const;
    void addConversation(const std::string& uri);
    void removeConversation(const std::string& uid);
    void selectConversation(const std::string& uid);
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string&);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void cleanHistory(const std::string& uid);

    // signals
Q_SIGNALS:
    void newMessageAdded(const std::string& uid, message::Info msg);
    void conversationUpdated(unsigned int row);
    void modelUpdated() const;
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(std::shared_ptr<conversation::Info> conversation);
    void showCallView(std::shared_ptr<conversation::Info> conversation);
    void showIncomingCallView(std::shared_ptr<conversation::Info> conversation);

private Q_SLOTS:
    void slotMessageAdded(int uid, const std::string& account, message::Info msg);
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);
    void newAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads);

private:
    /**
     * Search a conversation in conversations_
     * @param uid the contact to search
     * @return the contact if found else nullptr
     */
    std::shared_ptr<conversation::Info> find(const std::string& uid) const;
    /**
     * Initialize conversations_ and filteredConversations_
     */
    void initConversations();
    /**
     * Sort conversation by last action
     */
    void sortConversations();
    void search();

    std::shared_ptr<NewCallModel> callModel_;
    std::shared_ptr<ContactModel> contactModel_;
    const DatabaseManager& dbManager_;

    ConversationsList conversations_;
    mutable ConversationsList filteredConversations_;
    std::string filter_;

};

} // namespace lrc

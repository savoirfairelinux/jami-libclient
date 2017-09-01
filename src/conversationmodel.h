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

// Interface
#include "api/conversationmodeli.h"

// Data
#include "api/account.h"
#include "api/conversation.h"

// Lrc
#include "typedefs.h"
#include "account.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

namespace lrc
{

class Database;
class NewAccountModel;
class NewCallModel;

class LIB_EXPORT ConversationModel : public api::ConversationModelI {
public:
    const api::account::Info& owner;

    ConversationModel(const NewAccountModel& parent,
                      const Database& database,
                      const api::account::Info& info);
    ~ConversationModel();

    const api::ConversationQueue& getFilteredConversations() const override;
    api::conversation::Info getConversation(const unsigned int row) const override;
    void addConversation(const std::string& uri) const override;
    void removeConversation(const std::string& uid) override;
    void selectConversation(const std::string& uid) override;
    void placeCall(const std::string& uid) const override;
    void sendMessage(const std::string& uid, const std::string& body) const override;
    void setFilter(const std::string& filter) override;
    void addParticipant(const std::string& uid, const::std::string& uri) override;
    void clearHistory(const std::string& uid) override;

Q_SIGNALS:
    void newMessageAdded(const std::string& uid, const api::message::Info& msg) override;
    void conversationUpdated(unsigned int row) override;
    void modelUpdated() const override;
    void newContactAdded(const std::string& uid) override;
    void incomingCallFromItem(const unsigned int row) override;

    void showChatView(const api::conversation::Info& conversationInfo) override;
    void showCallView(const api::conversation::Info& conversationInfo) override;
    void showIncomingCallView(const api::conversation::Info& conversationInfo) override;

private Q_SLOTS:
    void slotContactsChanged();
    void slotMessageAdded(int uid, const std::string& accountId, const api::message::Info& msg);
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);

private:
    // shortcuts in owner
    NewCallModel& callModel_;

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

    const NewAccountModel& parent_;
    const Database& database_;

    api::ConversationQueue conversations_;
    mutable api::ConversationQueue filteredConversations_;
    std::string filter_;
};

} // namespace lrc

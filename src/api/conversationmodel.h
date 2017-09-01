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

// Data
#include "api/account.h"
#include "api/conversation.h"

// Lrc
#include "typedefs.h"
#include "account.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"
#include "namedirectory.h"

namespace lrc
{

class Database;

namespace api
{

using ConversationQueue = std::deque<conversation::Info>;

class NewAccountModel;
class NewCallModel;
class ConversationModelPimpl;

class LIB_EXPORT ConversationModel : public QObject {
public:
    const account::Info& owner;

    ConversationModel(const NewAccountModel& parent,
                      const Database& database,
                      const account::Info& info);
    ~ConversationModel();

    const ConversationQueue& getFilteredConversations() const;
    conversation::Info getConversation(const unsigned int row) const;
    void addConversation(const std::string& uri) const;
    void removeConversation(const std::string& uid);
    void selectConversation(const std::string& uid);
    void placeCall(const std::string& uid) const;
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string& filter);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void clearHistory(const std::string& uid);

Q_SIGNALS:
    void newMessageAdded(const std::string& uid, const message::Info& msg);
    void conversationUpdated(unsigned int row);
    void modelUpdated() const;
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(const conversation::Info& conversationInfo);
    void showCallView(const conversation::Info& conversationInfo);
    void showIncomingCallView(const conversation::Info& conversationInfo);

private Q_SLOTS:
    void slotContactsChanged();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

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
#include "api/contact.h"

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
    Q_OBJECT
public:
    const account::Info& owner;

    ConversationModel(const NewAccountModel& parent,
                      const Database& database,
                      const account::Info& info);
    ~ConversationModel();

    const ConversationQueue& getFilteredConversations() const;
    conversation::Info getConversation(unsigned int row) const;
    void addConversation(const std::string& uid) const;
    void removeConversation(const std::string& uid, bool banned=false);
    void selectConversation(const std::string& uid);
    void placeCall(const std::string& uid);
    void sendMessage(const std::string& uid, const std::string& body) const;
    void setFilter(const std::string& filter);
    void setFilter(const lrc::api::contact::Type& filter = lrc::api::contact::Type::INVALID);
    void addParticipant(const std::string& uid, const::std::string& uri);
    void clearHistory(const std::string& uid);

Q_SIGNALS:
    void newMessageAdded(const std::string& uid, const message::Info& msg) const;
    void conversationUpdated(unsigned int row);
    void modelUpdated() const;
    void newContactAdded(const std::string& uri);
    void incomingCallFromItem(const unsigned int row);

    void showChatView(const conversation::Info& conversationInfo); // TO MOVE
    void showCallView(const conversation::Info& conversationInfo);// TO MOVE
    void showIncomingCallView(const conversation::Info& conversationInfo);// TO MOVE

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

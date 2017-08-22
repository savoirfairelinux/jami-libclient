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
#include "conversationmodel.h"

// LRC
#include "callinfo.h"

// std
#include <algorithm>

ConversationModel::ConversationModel(QObject* parent)
:QObject(parent)
{

}

ConversationModel::~ConversationModel()
{

}

const Conversations&
ConversationModel::getConversations() const
{
    return conversations_;
}

const Conversation::Info&
ConversationModel::getConversation(const unsigned int row) const
{
    return Conversation::Info(nullptr, Contact::Info());
}

const Conversation::Info&
ConversationModel::addConversation(const std::string& uri)
{
    return Conversation::Info(nullptr, Contact::Info());
}

void
ConversationModel::selectConversation(const std::string& uid)
{
    // Get conversation
    auto conversation = find(uid);
    if (!conversation) return;
    auto participants = conversation->participants_;
    // Check if conversation has a valid contact.
    if (participants.empty() || participants.front().uri_.empty())
        return;
    switch (conversation->call_.status_) {
        case NewCall::Status::INCOMING_RINGING:
        case NewCall::Status::OUTGOING_RINGING:
        case NewCall::Status::CONNECTING:
        case NewCall::Status::SEARCHING:
            // We are currently in a call
            emit showIncomingCallView(*conversation);
            break;
        case NewCall::Status::IN_PROGRESS:
            // We are currently receiving a call
            emit showCallView(*conversation);
            break;
        case NewCall::Status::NONE:
        default:
            // We are not in a call, show the chatview
            emit showChatView(*conversation);
    }
}

std::shared_ptr<Conversation::Info>
ConversationModel::find(const std::string& uid)
{
    std::shared_ptr<Conversation::Info> result = nullptr;
    auto i = std::find_if(conversations_.begin(), conversations_.end(),
    [uid](const std::pair<std::string, std::shared_ptr<Conversation::Info>>& conversation) {
        return conversation.first == uid;
    });
    if (i != conversations_.end()) result = i->second;
    return result;
}

void
ConversationModel::removeConversation(const std::string& uid)
{

}

void
ConversationModel::placeCall(const std::string& uid) const
{

}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{

}

void
ConversationModel::setFilter(const std::string&)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{

}

void
ConversationModel::cleanHistory(const std::string& uid)
{

}

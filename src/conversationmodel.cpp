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
#include "availableaccountmodel.h"
#include "callinfo.h"

// std
#include <algorithm>

ConversationModel::ConversationModel(std::shared_ptr<ContactModel> contactModel,
  std::shared_ptr<DatabaseManager> dbManager, QObject* parent):
  contactModel_(contactModel), dbManager_(dbManager_), QObject(parent)
{
    initConversations();
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
    [uid](const ConversationEntry& conversation) {
        return conversation.first == uid;
    });
    if (i != conversations_.end()) result = i->second;
    return result;
}

void
ConversationModel::removeConversation(const std::string& uid)
{
    auto conversation = find(uid);
    if (!conversation || conversation->participants_.empty()) return;

    // TODO group chat?
    auto contact = conversation->participants_.front();
    contactModel_->removeContact(contact.uri_);

    // Remove conversation
    auto it = conversations_.begin();
    std::advance(it, conversation->index_);
    it = conversations_.erase(it);
    // Update indexes
    std::for_each(it, conversations_.end(), [](ConversationEntry conversation) {
        conversation.second->index_--;
    });

    // The model has changed
    emit modelUpdated();
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
    auto conversation = find(uid);
    if (!conversation || conversation->participants_.empty()) return;
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    dbManager_->removeHistory(account->id().toStdString(), uid);
}

void
ConversationModel::initConversations()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    // Fill conversations_
    for(auto const& contact : contactModel_->getContacts())
    {
        auto contactinfo = contact.second;
        // TODO change uid when group chat
        auto conversation = std::make_shared<Conversation::Info>(
        Conversation::Info(account, contactinfo->uri_,
        *contactinfo.get(),
        dbManager_->getMessages(account->id().toStdString(), contactinfo->uri_)));
        ConversationEntry item(conversation->uid_, conversation);
        conversations_.emplace_front(item);
    }
    sortConversations();
    filteredConversations_ = conversations_;
}

void
ConversationModel::sortConversations()
{
    std::sort(conversations_.begin(), conversations_.end(),
    [](const ConversationEntry& conversationA, const ConversationEntry& conversationB)
    {
        auto historyA = conversationA.second->messages_;
        auto historyB = conversationB.second->messages_;
        // A or B is a new conversation (without INVITE message)
        if (historyA.empty()) return true;
        if (historyB.empty()) return false;
        // Sort by last Interaction
        try
        {
            auto lastMessageA = historyA.at(conversationA.second->lastMessageUid_);
            auto lastMessageB = historyB.at(conversationB.second->lastMessageUid_);
            return lastMessageA.timestamp_ > lastMessageB.timestamp_;
        }
        catch (const std::exception& e)
        {
            qDebug() << "ConversationModel::sortConversations(), can't get lastMessage";
            return true;
        }
    });
}

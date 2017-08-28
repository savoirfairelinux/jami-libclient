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
#include "dbus/configurationmanager.h"

namespace lrc
{

ConversationModel::ConversationModel(std::shared_ptr<NewCallModel>& callModel,
                                     std::shared_ptr<ContactModel>& contactModel,
                                     const DatabaseManager& dbManager)
: callModel_(callModel), contactModel_(contactModel), dbManager_(dbManager), QObject()
{
    initConversations();
    connect(&dbManager_, &DatabaseManager::messageAdded, this, &ConversationModel::slotMessageAdded);
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingAccountMessage,
            this,
            &ConversationModel::newAccountMessage);
}

ConversationModel::~ConversationModel()
{

}

std::shared_ptr<ContactModel>
ConversationModel::getContactModel()
{
    return contactModel_;
}


const ConversationsList&
ConversationModel::getConversations() const
{
    return ConversationsList();
}

std::shared_ptr<conversation::Info>
ConversationModel::getConversation(const unsigned int row) const
{
    if (row >= filteredConversations_.size())
        return nullptr;
    return filteredConversations_.at(row);
}

void
ConversationModel::addConversation(const std::string& uri)
{
    auto conversation = find(uri);
    if (!conversation || conversation->participants.empty()) return;
    auto contact = conversation->participants.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account
    if (!account) return;

    // Send contact request if non used
    if(!conversation->isUsed) {
        if (contact->uri.length() == 0) return;
        contactModel_->addContact(uri);

        // TODO just this item
        emit modelUpdated();
    }
}

void
ConversationModel::selectConversation(const std::string& uid)
{
    // Get conversation
    auto conversation = find(uid);
    if (!conversation) return;
    auto participants = conversation->participants;
    // Check if conversation has a valid contact.
    if (participants.empty() || participants.front()->uri.empty())
        return;
    if (!conversation->call) {
        emit showChatView(conversation);
        return;
    }
    switch (conversation->call->status) {
    case call::Status::INCOMING_RINGING:
    case call::Status::OUTGOING_RINGING:
    case call::Status::CONNECTING:
    case call::Status::SEARCHING:
            // We are currently in a call
            emit showIncomingCallView(conversation);
            break;
        case call::Status::IN_PROGRESS:
            // We are currently receiving a call
            emit showCallView(conversation);
            break;
        case call::Status::NONE:
        default:
            // We are not in a call, show the chatview
            emit showChatView(conversation);
    }
}

void
ConversationModel::removeConversation(const std::string& uid)
{
    auto conversation = find(uid);
    if (!conversation || conversation->participants.empty()) return;

    auto contact = conversation->participants.front();
    contactModel_->removeContact(contact->uri);

    // Remove conversation
    auto it = conversations_.begin();
    std::advance(it, conversation->index);
    it = conversations_.erase(it);

    // The model has changed
    emit modelUpdated();
}

void
ConversationModel::placeCall(const std::string& uid) const
{
    // TODO
}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{
    auto conversation = find(uid);
    if (!conversation || conversation->participants.empty()) return;
    auto contact = conversation->participants.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account
    if (!account) return;

    // Send contact request if non used
    if(!conversation->isUsed) {
        if (contact->uri.length() == 0) return;
        contactModel_->addContact(contact->uri);
        emit modelUpdated();
    }
    // Send message to contact.
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    // TODO change this for group messages
    auto id = ConfigurationManager::instance().sendTextMessage(account->id(),
    contact->uri.c_str(), payloads);

    message::Info msg;
    msg.uid = contact->uri.c_str();
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.isOutgoing = true;
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;

    dbManager_.addMessage(account->id().toStdString(), msg);

}

void
ConversationModel::setFilter(const std::string&)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{
    // TODO
}

void
ConversationModel::cleanHistory(const std::string& uid)
{
    auto conversation = find(uid);
    if (!conversation || conversation->participants.empty()) return;
    dbManager_.removeHistory(conversation->account->id().toStdString(), uid);
}

void
ConversationModel::initConversations()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    // Fill conversations_
    for(auto const& contact : contactModel_->getContacts())
    {
        auto contactinfo = contact;
        auto conversation = std::make_shared<conversation::Info>();
        conversation->uid = contactinfo.second->uri;
        conversation->participants.emplace_back(contactinfo.second);
        auto messages = dbManager_.getMessages(account->id().toStdString(), contactinfo.second->uri);
        conversation->messages = messages;
        if(!messages.empty()) {
            conversation->lastMessageUid = (--messages.end())->first;
            conversation->isUsed = true;
        }
        conversation->account = account;
        conversations_.emplace_front(conversation);
    }
    sortConversations();
    filteredConversations_ = conversations_;
}

void
ConversationModel::sortConversations()
{
    std::sort(conversations_.begin(), conversations_.end(),
    [](const std::shared_ptr<conversation::Info>& conversationA, const std::shared_ptr<conversation::Info>& conversationB)
    {
        auto historyA = conversationA->messages;
        auto historyB = conversationB->messages;
        // A or B is a new conversation (without INVITE message)
        if (historyA.empty()) return true;
        if (historyB.empty()) return false;
        // Sort by last Interaction
        try
        {
            auto lastMessageA = historyA.at(conversationA->lastMessageUid);
            auto lastMessageB = historyB.at(conversationB->lastMessageUid);
            return lastMessageA.timestamp > lastMessageB.timestamp;
        }
        catch (const std::exception& e)
        {
            qDebug() << "ConversationModel::sortConversations(), can't get lastMessage";
            return true;
        }
    });
}

void
ConversationModel::slotMessageAdded(int uid, const std::string& account, message::Info msg)
{
    auto conversation = find(msg.uid);
    if (!conversation || conversation->participants.empty()) return;
    if (!conversation->isUsed) conversation->isUsed = true;
    // Add message to conversation
    conversation->messages.insert(std::pair<int, message::Info>(uid, msg));
    conversation->lastMessageUid = uid;
    emit newMessageAdded(msg.uid, msg);
    sortConversations();
    emit modelUpdated();
}

void
ConversationModel::newAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads)
{
    message::Info msg;
    msg.uid = from.toStdString();
    msg.body = payloads["text/plain"].toStdString();
    msg.timestamp = std::time(nullptr);
    msg.isOutgoing = false;
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SUCCEED;
    dbManager_.addMessage(accountId.toStdString(), msg);
}

std::shared_ptr<conversation::Info>
ConversationModel::find(const std::string& uid) const
{
    std::shared_ptr<conversation::Info> result = nullptr;
    auto i = std::find_if(conversations_.begin(), conversations_.end(),
    [uid](const std::shared_ptr<conversation::Info>& conversation) {
        return conversation->uid == uid;
    });
    if (i != conversations_.end()) result = *i;
    return result;
}

}

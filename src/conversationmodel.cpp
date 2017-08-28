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

// std
#include <regex>

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
    filteredConversations_ = conversations_;

    if (filter_.length() == 0) return filteredConversations_;

    auto filter = filter_;
    auto it = std::copy_if(conversations_.begin(), conversations_.end(), filteredConversations_.begin(),
    [&filter, this] (const std::shared_ptr<lrc::conversation::Info>& entry) {
        auto isUsed = entry->isUsed;
        if (!isUsed) return true;
        auto contact = entry->participants.front();
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(contact->uri, regexFilter)
            | std::regex_search(contact->alias, regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring of the title or the alias.
            return contact->alias.find(filter) != std::string::npos
            && contact->uri.find(filter) != std::string::npos;
        }
    });
    filteredConversations_.resize(std::distance(filteredConversations_.begin(), it));

    return filteredConversations_;
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
ConversationModel::setFilter(const std::string& filter)
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    auto participant = std::make_shared<lrc::contact::Info>();
    participant->uri = "";
    participant->avatar = "";
    participant->registeredName = "";
    participant->alias = "Searching...";
    participant->isTrusted = false;
    participant->isPresent = false;

    filter_ = filter;
    std::shared_ptr<lrc::conversation::Info> newConversationItem;
    if (!filter_.empty()) {
        // add the first item, wich is the NewConversationItem
        auto conversation = std::make_shared<lrc::conversation::Info>();
        conversation->uid = participant->uri;
        conversation->participants.emplace_back(participant);
        conversation->account = account;
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front()->isUsed;
            if (isUsed) {
                // No newConversationItem, create one
                newConversationItem = conversation;
                conversations_.emplace_front(std::shared_ptr<lrc::conversation::Info>(newConversationItem));
            } else {
                // The item already exists
                conversations_.pop_front();
                newConversationItem = conversation;
                conversations_.emplace_front(std::shared_ptr<lrc::conversation::Info>(newConversationItem));
            }
        } else {
            newConversationItem = conversation;
            conversations_.emplace_front(std::shared_ptr<lrc::conversation::Info>(newConversationItem));
        }
        search();
    } else {
        // No filter, so we can remove the newConversationItem
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front()->isUsed;
            if (!isUsed) {
                conversations_.pop_front();
            }
        }
    }
    emit modelUpdated();
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

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
#include <stdexcept>

// Dbus
#include "dbus/configurationmanager.h"

// Models and database
#include "database.h"
#include "newcallmodel.h"
#include "contactmodel.h"

// Lrc
#include "availableaccountmodel.h"

namespace lrc
{

ConversationModel::ConversationModel(const NewAccountModel& parent,
                                     const Database& database,
                                     const account::Info& info)
: parent_(parent)
, database_(database)
, owner(info)
, QObject()
{
    initConversations();
    connect(&database_, &Database::messageAdded, this, &ConversationModel::slotMessageAdded);
    connect(&*owner.contactModel, &ContactModel::contactsChanged, this, &ConversationModel::slotContactsChanged);
}

ConversationModel::~ConversationModel()
{

}

const ConversationsList&
ConversationModel::getFilteredConversations() const
{
    filteredConversations_ = conversations_;

    if (filter_.length() == 0) return filteredConversations_;

    auto filter = filter_;
    auto it = std::copy_if(conversations_.begin(), conversations_.end(), filteredConversations_.begin(),
    [&filter, this] (const conversation::Info& entry) {
        auto isUsed = entry.isUsed;
        if (!isUsed) return true;
        auto contact = owner.contactModel->getContact(entry.participants.front());
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(contact.uri, regexFilter)
            | std::regex_search(contact.alias, regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring of the title or the alias.
            return contact.alias.find(filter) != std::string::npos
            && contact.uri.find(filter) != std::string::npos;
        }
    });
    filteredConversations_.resize(std::distance(filteredConversations_.begin(), it));

    return filteredConversations_;
}

conversation::Info
ConversationModel::getConversation(const unsigned int row) const
{
    if (row >= filteredConversations_.size())
        return conversation::Info();
    return filteredConversations_.at(row);
}

void
ConversationModel::addConversation(const std::string& uri) const
{
    auto conversationIdx = find(uri);
    if (conversationIdx == -1) return;
    auto conversation = conversations_.at(conversationIdx);
    if (conversation.participants.empty()) return;
    auto contact = conversation.participants.front();

    // Send contact request if non used
    if(!conversation.isUsed) {
        if (contact.length() == 0) return;
        owner.contactModel->addContact(contact);
    }

}

void
ConversationModel::selectConversation(const std::string& uid)
{
    // Get conversation
    auto conversationIdx = find(uid);
    if (conversationIdx == -1) return;
    auto conversation = conversations_.at(conversationIdx);
    auto participants = conversation.participants;
    // Check if conversation has a valid contact.
    if (participants.empty() || participants.front().empty())
        return;
    emit showChatView(conversation);
    /* TODO
    if (conversation.call.status == call::Status::INVALID) {
        emit showChatView(conversation);
        return;
    }
    switch (conversation.call.status) {
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
        default:
            // We are not in a call, show the chatview
            emit showChatView(conversation);
    }*/
}

void
ConversationModel::removeConversation(const std::string& uid)
{
    // Get conversation
    auto i = std::find_if(conversations_.begin(), conversations_.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    if (i == conversations_.end()) return;
    auto conversation = *i;
    if (conversation.participants.empty()) return;

    // Remove contact from daemon
    auto contact = conversation.participants.front();
    owner.contactModel->removeContact(contact);
}

void
ConversationModel::placeCall(const std::string& uid) const
{
    owner.callModel->createCall("fake uri just for class test");
}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{
    auto conversationIdx = find(uid);
    if (conversationIdx == -1) return;
    auto conversation = conversations_.at(conversationIdx);
    if (conversation.participants.empty()) return;
    auto contact = conversation.participants.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account
    if (!account) return;

    // Send contact request if non used
    if(!conversation.isUsed) {
        if (contact.length() == 0) return;
        addConversation(contact);
    }
    // Send message to contact.
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    // TODO change this for group messages
    auto id = ConfigurationManager::instance().sendTextMessage(account->id(),
    contact.c_str(), payloads);

    message::Info msg;
    msg.contact = contact.c_str();
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;

    database_.addMessage(account->id().toStdString(), msg);

}

void
ConversationModel::setFilter(const std::string& filter)
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    filter_ = filter;
    conversation::Info newConversationItem;
    if (!filter_.empty()) {
        // add the first item, wich is the NewConversationItem
        conversation::Info conversation;
        contact::Info participant;
        participant.alias = "Searching...";
        conversation.uid = participant.uri;
        participant.alias += filter;
        owner.contactModel->temporaryContact = participant;
        conversation.participants.emplace_back("");
        conversation.accountId = account->id().toStdString();
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front().isUsed;
            if (isUsed) {
                // No newConversationItem, create one
                newConversationItem = conversation;
                conversations_.emplace_front(newConversationItem);
            } else {
                // The item already exists
                conversations_.pop_front();
                newConversationItem = conversation;
                conversations_.emplace_front(newConversationItem);
            }
        } else {
            newConversationItem = conversation;
            conversations_.emplace_front(newConversationItem);
        }
        search();
    } else {
        // No filter, so we can remove the newConversationItem
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front().isUsed;
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
ConversationModel::clearHistory(const std::string& uid)
{

}

void
ConversationModel::initConversations()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    conversations_.clear();
    // Fill conversations_
    for(auto const& contact : owner.contactModel->getAllContacts())
    {
        auto contactinfo = contact;
        conversation::Info conversation;
        conversation.uid = contactinfo.second->uri;
        conversation.participants.emplace_back((*contactinfo.second.get()).uri);
        auto messages = database_.getHistory(account->id().toStdString(),
                                             contactinfo.second->uri);
        conversation.messages = messages;
        if(!messages.empty()) {
            conversation.lastMessageUid = (--messages.end())->first;
            conversation.isUsed = true;
        }
        conversation.accountId = account->id().toStdString();
        conversations_.emplace_front(conversation);
    }
    sortConversations();
    filteredConversations_ = conversations_;
}

void
ConversationModel::sortConversations()
{
    std::sort(conversations_.begin(), conversations_.end(),
    [](const conversation::Info& conversationA, const conversation::Info& conversationB)
    {
        auto historyA = conversationA.messages;
        auto historyB = conversationB.messages;
        // A or B is a new conversation (without INVITE message)
        if (historyA.empty()) return true;
        if (historyB.empty()) return false;
        // Sort by last Interaction
        try
        {
            auto lastMessageA = historyA.at(conversationA.lastMessageUid);
            auto lastMessageB = historyB.at(conversationB.lastMessageUid);
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
    auto conversationIdx = find(msg.contact);
    if (conversationIdx == -1) return;
    auto conversation = conversations_.at(conversationIdx);
    if (conversation.participants.empty()) return;
    if (!conversation.isUsed) conversation.isUsed = true;
    // Add message to conversation
    conversation.messages.insert(std::pair<int, message::Info>(uid, msg));
    conversation.lastMessageUid = uid;
    emit newMessageAdded(msg.contact, msg);
    sortConversations();
    emit modelUpdated();
}

int
ConversationModel::find(const std::string& uid) const
{
    for (unsigned int i = 0; i < conversations_.size() ; ++i) {
        if(conversations_.at(i).uid == uid) return i;
    }
    return -1;
}

void
ConversationModel::search()
{
    // Update alias
    auto uri = URI(QString(filter_.c_str()));
    // Query NS
    Account* account = nullptr;
    if (uri.schemeType() != URI::SchemeType::NONE) {
        account = AvailableAccountModel::instance().currentDefaultAccount(uri.schemeType());
    } else {
        account = AvailableAccountModel::instance().currentDefaultAccount();
    }
    if (!account) return;
    connect(&NameDirectory::instance(), &NameDirectory::registeredNameFound,
    this, &ConversationModel::registeredNameFound);

    if (account->protocol() == Account::Protocol::RING &&
        uri.protocolHint() != URI::ProtocolHint::RING)
    {
        account->lookupName(QString(filter_.c_str()));
    } else {
        /* no lookup, simply use the URI as is */
        auto cm = PhoneDirectoryModel::instance().getNumber(uri, account);
        if (!conversations_.empty()) {
            auto firstConversation = conversations_.front();
            if (!firstConversation.isUsed) {
                auto account = AvailableAccountModel::instance().currentDefaultAccount();
                if (!account) return;
                auto uid = cm->uri().toStdString();
                conversations_.pop_front();
                auto conversationIdx = find(uid);
                if (conversationIdx == -1) {
                    // create the new conversation
                    conversation::Info conversation;
                    contact::Info participant;
                    participant.uri = uid;
                    participant.alias = cm->bestName().toStdString();
                    conversation.uid = participant.uri;
                    owner.contactModel->temporaryContact = participant;
                    conversation.participants.emplace_back("");
                    conversation.accountId = account->id().toStdString();
                    conversations_.emplace_front(conversation);
                }
                emit modelUpdated();
            }
        }
    }
}

void
ConversationModel::registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name)
{
    Q_UNUSED(account)

    if (status == NameDirectory::LookupStatus::SUCCESS) {
        if (!conversations_.empty()) {
            auto firstConversation = conversations_.front();
            if (!firstConversation.isUsed) {
                auto account = AvailableAccountModel::instance().currentDefaultAccount();
                if (!account) return;
                auto uid = address.toStdString();
                conversations_.pop_front();
                auto conversationIdx = find(uid);
                if (conversationIdx == -1) {
                    // create the new conversation
                    conversation::Info conversation;
                    contact::Info participant;
                    participant.uri = uid;
                    participant.alias = name.toStdString();
                    conversation.uid = participant.uri;
                    owner.contactModel->temporaryContact = participant;
                    conversation.participants.emplace_back("");
                    conversation.accountId = account->id().toStdString();
                    conversations_.emplace_front(conversation);
                }
                emit modelUpdated();
            }
        }
    }
    disconnect(&NameDirectory::instance(), &NameDirectory::registeredNameFound,
    this, &ConversationModel::registeredNameFound);
}

void
ConversationModel::slotContactsChanged()
{
    initConversations();
    emit modelUpdated();
}

} // namespace lrc

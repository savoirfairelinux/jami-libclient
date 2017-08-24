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
#include "dbus/configurationmanager.h"
#include "namedirectory.h"
#include "uri.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"

// TODO move
#include "private/imconversationmanagerprivate.h"

// std
#include <algorithm>
#include <stdexcept>
#include <regex>

ConversationModel::ConversationModel(std::shared_ptr<NewCallModel> callModel,
  std::shared_ptr<ContactModel> contactModel,
  std::shared_ptr<DatabaseManager> dbManager, QObject* parent):
  callModel_(callModel), contactModel_(contactModel), dbManager_(dbManager), QObject(parent)
{
    initConversations();
    connect(&*dbManager_, &DatabaseManager::messageAdded, this, &ConversationModel::slotMessageAdded);

    // TODO move this
    IMConversationManagerPrivate::instance().setDatabaseManager(dbManager_);
}

ConversationModel::~ConversationModel()
{

}

std::shared_ptr<ContactModel>
ConversationModel::getContactModel()
{
    return contactModel_;
}


const Conversations&
ConversationModel::getConversations() const
{
    filteredConversations_ = conversations_;

    if (filter_.length() == 0) return filteredConversations_;

    auto filter = filter_;
    auto it = std::copy_if(conversations_.begin(), conversations_.end(), filteredConversations_.begin(),
    [&filter, this] (const ConversationEntry& entry) {
        auto isUsed = entry.second->isUsed_;
        if (!isUsed) return true;
        auto contact = entry.second->participants_.front();
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(contact->uri_, regexFilter)
            | std::regex_search(contact->alias_, regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring of the title or the alias.
            return contact->alias_.find(filter) != std::string::npos
            && contact->uri_.find(filter) != std::string::npos;
        }
    });
    filteredConversations_.resize(std::distance(filteredConversations_.begin(), it));

    return filteredConversations_;
}

std::shared_ptr<Conversation::Info>
ConversationModel::getConversation(const unsigned int row) const
{
    if (row >= filteredConversations_.size())
        throw std::out_of_range("Can't get conversation at row " + std::to_string(row));
    return filteredConversations_.at(row).second;
}

void
ConversationModel::addConversation(const std::string& uri)
{
    auto conversation = find(uri);
    if (!conversation || conversation->participants_.empty()) return;
    auto contact = conversation->participants_.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account
    if (!account) return;

    // Send contact request if non used
    if(!conversation->isUsed_) {
        if (contact->uri_.length() == 0) return;
        ConfigurationManager::instance().addContact(account->id(),
        QString(contact->uri_.c_str()));
    }
}

void
ConversationModel::selectConversation(const std::string& uid)
{
    // Get conversation
    auto conversation = find(uid);
    if (!conversation) return;
    auto participants = conversation->participants_;
    // Check if conversation has a valid contact.
    if (participants.empty() || participants.front()->uri_.empty())
        return;
    if (!conversation->call_) {
        emit showChatView(conversation);
        return;
    }
    switch (conversation->call_->status_) {
        case NewCall::Status::INCOMING_RINGING:
        case NewCall::Status::OUTGOING_RINGING:
        case NewCall::Status::CONNECTING:
        case NewCall::Status::SEARCHING:
            // We are currently in a call
            emit showIncomingCallView(conversation);
            break;
        case NewCall::Status::IN_PROGRESS:
            // We are currently receiving a call
            emit showCallView(conversation);
            break;
        case NewCall::Status::NONE:
        default:
            // We are not in a call, show the chatview
            emit showChatView(conversation);
    }
}

std::shared_ptr<Conversation::Info>
ConversationModel::find(const std::string& uid) const
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
    contactModel_->removeContact(contact->uri_);

    // Remove conversation
    auto it = conversations_.begin();
    std::advance(it, conversation->index_);
    it = conversations_.erase(it);

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
    auto conversation = find(uid);
    if (!conversation || conversation->participants_.empty()) return;
    auto contact = conversation->participants_.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account
    if (!account) return;

    // Send contact request if non used
    if(!conversation->isUsed_) {
        if (contact->uri_.length() == 0) return;
        ConfigurationManager::instance().addContact(account->id(),
        QString(contact->uri_.c_str()));
    }
    // Send message to contact.
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    // TODO change this for group messages
    auto id = ConfigurationManager::instance().sendTextMessage(account->id(),
    contact->uri_.c_str(), payloads);

    Message::Info msg(contact->uri_.c_str(), body, true, Message::Type::TEXT,
    std::time(nullptr), Message::Status::SENDING);

    dbManager_->addMessage(account->id().toStdString(), msg);

    // TODO change last interaction + sort + isUsed + add contact to db + invite msg
}


void
ConversationModel::setFilter(const std::string& filter)
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    auto participant = std::make_shared<Contact::Info>("", "", "", "Searching... " + filter);
    filter_ = filter;
    std::shared_ptr<Conversation::Info> newConversationItem;
    if (!filter_.empty()) {
        // add the first item, wich is the NewConversationItem
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front().second->isUsed_;
            if (isUsed) {
                // No newConversationItem, create one
                newConversationItem = std::make_shared<Conversation::Info>(account, participant);
                conversations_.emplace_front(ConversationEntry("", newConversationItem));
            } else {
                // The item already exists
                conversations_.pop_front();
                newConversationItem = std::make_shared<Conversation::Info>(account, participant);
                conversations_.emplace_front(ConversationEntry("", newConversationItem));
            }
        } else {
            newConversationItem = std::make_shared<Conversation::Info>(account, participant);
            conversations_.emplace_front(ConversationEntry("", newConversationItem));
        }
        search();
    } else {
        // No filter, so we can remove the newConversationItem
        if (!conversations_.empty()) {
            auto isUsed = conversations_.front().second->isUsed_;
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

}

void
ConversationModel::cleanHistory(const std::string& uid)
{
    auto conversation = find(uid);
    if (!conversation || conversation->participants_.empty()) return;
    dbManager_->removeHistory(conversation->account_->id().toStdString(), uid);
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
        auto conversation = std::make_shared<Conversation::Info>(account,
        contactinfo->uri_,
        contactinfo,
        dbManager_->getMessages(account->id().toStdString(), contactinfo->uri_));
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
    // TODO change conversations indexes
}

void
ConversationModel::slotMessageAdded(int uid, const std::string& account, Message::Info msg)
{
    auto conversation = find(msg.uid_);
    if (!conversation || conversation->participants_.empty()) return;
    if (!conversation->isUsed_) conversation->isUsed_ = true;
    // Add message to conversation
    conversation->messages_.insert(std::pair<int, Message::Info>(uid, msg));
    conversation->lastMessageUid_ = uid;
    emit newMessageAdded(msg.uid_, msg);
    // TODO sort
    // TODO update model
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
            auto firstConversation = conversations_.front().second;
            if (!firstConversation->isUsed_) {
                auto account = AvailableAccountModel::instance().currentDefaultAccount();
                if (!account) return;
                auto uid = cm->uri().toStdString();
                auto participant = std::make_shared<Contact::Info>(
                uid, "", "", cm->bestName().toStdString());
                conversations_.pop_front();
                if (!find(uid)) {
                    conversations_.emplace_front(ConversationEntry(
                        uid,
                        std::make_shared<Conversation::Info>(account, participant)
                    ));
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
            auto firstConversation = conversations_.front().second;
            if (!firstConversation->isUsed_) {
                auto account = AvailableAccountModel::instance().currentDefaultAccount();
                if (!account) return;
                auto uid = address.toStdString();
                auto participant = std::make_shared<Contact::Info>(
                uid, "", "", name.toStdString());
                conversations_.pop_front();
                if (!find(uid)) {
                    conversations_.emplace_front(ConversationEntry(
                        uid,
                        std::make_shared<Conversation::Info>(account, participant)
                    ));
                }
                emit modelUpdated();
            }
        }
    }
    disconnect(&NameDirectory::instance(), &NameDirectory::registeredNameFound,
    this, &ConversationModel::registeredNameFound);
}

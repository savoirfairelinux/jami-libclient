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
#include "api/conversationmodel.h"

// std
#include <regex>
#include <stdexcept>

// Models and database
#include "database.h"
#include "api/contactmodel.h"
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"

// Dbus
#include "dbus/configurationmanager.h"

// Lrc
#include "availableaccountmodel.h"

namespace lrc
{

namespace api
{

class ConversationModelPimpl : public QObject
{
public:
    ConversationModelPimpl(const ConversationModel& linked,
                           const NewAccountModel& p,
                           const Database& d,
                           const account::Info& o);

    ~ConversationModelPimpl();

    // shortcuts in owner
    NewCallModel& callModel; // [jn] : is it really useful ?

    /**
     * return a conversation index from conversations or -1 if no index is found.
     * @param uid of the contact to search.
     * @return an int.
     */
    int indexOf(const std::string& uid) const;
    /**
     * Initialize conversations_ and filteredConversations_
     */
    void initConversations();
    /**
     * Sort conversation by last action
     */
    void sortConversations();
    void search();

    const ConversationModel& linked;
    const NewAccountModel& parent;
    const Database& database;

    ConversationQueue conversations;
    mutable ConversationQueue filteredConversations;
    std::string filter;

public Q_SLOTS:
    void slotContactModelUpdated();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);

};

ConversationModel::ConversationModel(const NewAccountModel& parent,
                                     const Database& database,
                                     const account::Info& info)
: pimpl_(std::make_unique<ConversationModelPimpl>(*this, parent, database, owner))
, owner(info)
{

}

ConversationModel::~ConversationModel()
{

}

const ConversationQueue&
ConversationModel::getFilteredConversations() const
{
    return pimpl_->conversations;
}

conversation::Info
ConversationModel::getConversation(const unsigned int row) const
{
    if (row >= pimpl_->filteredConversations.size())
        return conversation::Info();
    return pimpl_->filteredConversations.at(row);
}

void
ConversationModel::addConversation(const std::string& uid) const
{
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto conversation = pimpl_->conversations.at(conversationIdx);

    if (conversation.participants.empty())
        return;

    auto contact = conversation.participants.front();

    // Send contact request if non used
    if (not conversation.isUsed) {
        if (contact.length() == 0)
            return;

        owner.contactModel->addContact(contact);
    }

}

void
ConversationModel::selectConversation(const std::string& uid)
{
    // Get conversation
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto conversation = pimpl_->conversations.at(conversationIdx);
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
    auto i = std::find_if(pimpl_->conversations.begin(), pimpl_->conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });

    if (i == pimpl_->conversations.end())
        return;

    auto conversation = *i;

    if (conversation.participants.empty())
        return;

    // Remove contact from daemon
    auto contact = conversation.participants.front();
    owner.contactModel->removeContact(contact);
}

void
ConversationModel::placeCall(const std::string& uid) const
{
}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto conversation = pimpl_->conversations.at(conversationIdx);
    if (conversation.participants.empty())
        return;

    auto contact = owner.contactModel->getContact(conversation.participants.front());

    // Send contact request if non used
    if (!conversation.isUsed)
        addConversation(uid);

    // Send message to contact.
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    // TODO change this for group messages
    auto id = ConfigurationManager::instance().sendTextMessage(QString(owner.id.c_str()),
    contact.uri.c_str(), payloads);

    message::Info msg;
    msg.contact = contact.uri.c_str();
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;

    pimpl_->database.addMessage(owner.id, msg);

}

void
ConversationModel::setFilter(const std::string& filter)
{

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
ConversationModelPimpl::slotMessageAdded(int uid, const std::string& account, const message::Info& msg)
{
    auto conversationIdx = indexOf(msg.contact);

    if (conversationIdx == -1)
        return;

    auto conversation = conversations.at(conversationIdx);

    if (conversation.participants.empty())
        return;

    if (!conversation.isUsed)
        conversation.isUsed = true;

    // Add message to conversation
    conversation.messages.insert(std::pair<int, message::Info>(uid, msg));
    conversation.lastMessageUid = uid;

    emit linked.newMessageAdded(msg.contact, msg);

    sortConversations();
    emit linked.modelUpdated();
}

void
ConversationModelPimpl::registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                       const QString& address, const QString& name)
{

    std::sort(conversations.begin(), conversations.end(),
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

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked,
                                               const NewAccountModel& p,
                                               const Database& d,
                                               const account::Info& o)
: linked(linked)
, parent(p)
, database(d)
, callModel(*o.callModel)
{
    initConversations();
    connect(&database, &Database::messageAdded, this, &ConversationModelPimpl::slotMessageAdded);
    connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            this, &ConversationModelPimpl::slotContactModelUpdated);
}

ConversationModelPimpl::~ConversationModelPimpl()
{

}

int
ConversationModelPimpl::indexOf(const std::string& uid) const
{
    for (unsigned int i = 0; i < conversations.size() ; ++i) {
        if(conversations.at(i).uid == uid) return i;
    }
    return -1;
}

void
ConversationModelPimpl::initConversations()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (!account)
        return;

    conversations.clear();
    // Fill conversations
    for (auto const& contact : linked.owner.contactModel->getAllContacts())
    {
        auto contactinfo = contact;
        conversation::Info conversation;
        conversation.uid = contactinfo.second->uri;
        conversation.participants.emplace_back((*contactinfo.second.get()).uri);
        auto messages = database.getHistory(account->id().toStdString(),
                                             contactinfo.second->uri);
        conversation.messages = messages;
        if (!messages.empty()) {
            conversation.lastMessageUid = (--messages.end())->first;
            conversation.isUsed = true;
        }
        conversation.accountId = account->id().toStdString();
        conversations.emplace_front(conversation);
    }
    sortConversations();
    filteredConversations = conversations;
}

void
ConversationModelPimpl::sortConversations()
{

}

void
ConversationModelPimpl::slotContactModelUpdated()
{
    initConversations();
    emit linked.modelUpdated();
}

} // namespace api
} // namespace lrc

#include "api/moc_conversationmodel.cpp"

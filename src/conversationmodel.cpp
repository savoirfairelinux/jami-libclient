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

// LRC
#include "api/contactmodel.h"
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"
#include "api/account.h"
#include "callbackshandler.h"
#include "database.h"

#include "availableaccountmodel.h"
#include "namedirectory.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class ConversationModelPimpl : public QObject
{
    Q_OBJECT
public:
    ConversationModelPimpl(const ConversationModel& linked,
                           Database& db,
                           const CallbacksHandler& callbacksHandler);

    ~ConversationModelPimpl();

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
    int find(const std::string& uid) const;

    const ConversationModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    ConversationModel::ConversationQueue conversations;
    mutable ConversationModel::ConversationQueue filteredConversations;
    std::string filter;

public Q_SLOTS:
    void slotContactsChanged();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);
    void slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);

};

ConversationModel::ConversationModel(const account::Info& owner, Database& db, const CallbacksHandler& callbacksHandler)
: QObject()
, pimpl_(std::make_unique<ConversationModelPimpl>(*this, db, callbacksHandler))
, owner(owner)
{

}

ConversationModel::~ConversationModel()
{

}

const ConversationModel::ConversationQueue&
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

    auto contact = conversation.participants.front();
    auto account = AvailableAccountModel::instance().currentDefaultAccount(); // TODO replace by linked account

    if (!account || contact.length() == 0)
        return;

    // Send contact request if non used
    if (!conversation.isUsed)
        addConversation(contact);

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

    // Add to database
    pimpl_->db.insertInto("conversations",
                  {{":contactId", "contact"}, {":accountId", "account"}, {":body", "body"},
                   {":timestamp", "timestamp"}, {":isUnread", "is_unread"}, {":isOutgoing", "is_outgoing"},
                   {":type", "type"}, {":status", "status"}},
                  {{":contactId", msg.contact}, {":accountId", owner.id}, {":body", msg.body},
                   {":timestamp", std::to_string(msg.timestamp)}, {":isUnread", "1"}, {":isOutgoing", "true"},
                   {":type", TypeToString(msg.type)}, {":status", StatusToString(msg.status)}});
}

void
ConversationModel::setFilter(const std::string& filter)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{

}

void
ConversationModel::clearHistory(const std::string& uid)
{

}

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked,
                                               Database& db,
                                               const CallbacksHandler& callbacksHandler)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
{
    initConversations();

    // [jn] those signals don't make sense anymore. We may have to recycle them, or just to delete them.
    // since adding a message from Conversation use insertInto wich return something, I guess we can deal with
    // messageAdded without signal.
    //~ connect(&db, &Database::messageAdded, this, &ConversationModelPimpl::slotMessageAdded);
    //~ connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            //~ this, &ConversationModelPimpl::slotContactModelUpdated);
}


ConversationModelPimpl::~ConversationModelPimpl()
{

}

int
ConversationModelPimpl::find(const std::string& uid) const
{
    return -1;
}

void
ConversationModelPimpl::search()
{

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
        /*auto contactinfo = contact;
        conversation::Info conversation;
        conversation.uid = contactinfo.second->uri;
        conversation.participants.emplace_back((*contactinfo.second.get()).uri);
        auto returnFromDb = db.select("id, contact, body, timestamp, is_outgoing, type, status",
                                      "conversations",
                                      "contact=:c AND account=:a",
                                      {{":c", "contactinfo.second->uri"}, {":a", "account->id().toStdString()"}});

        // we excpect 7 columns
        if (returnFromDb.nbrOfCols == 7 and returnFromDb.payloads.size() > 0 and returnFromDb.payloads.size()%7 == 0) {
            std::map<int, api::message::Info> messages;
            for (int i = 0 ; i < returnFromDb.payloads.size() ; i+=7) {
                const auto messageId = std::stoi(returnFromDb.payloads[i]);
                message::Info msg;
                msg.contact = returnFromDb.payloads[i+1];
                msg.body = returnFromDb.payloads[i+2];
                msg.timestamp = std::stoi(returnFromDb.payloads[i+3]);

                // is outgoing = returnFromDb.payloads[i+4];
                msg.type = message::StringToType(returnFromDb.payloads[i+5]);
                msg.status = message::StringToStatus(returnFromDb.payloads[i+6]);
                messages.insert(std::pair<int, message::Info>(messageId, msg));
            }

            conversation.messages = std::move(messages);
            if (!messages.empty()) {
                conversation.lastMessageUid = (--messages.end())->first;
                conversation.isUsed = true;
            }

            conversation.accountId = account->id().toStdString();
            conversations.emplace_front(conversation);
        }*/
    }
    sortConversations();
    filteredConversations = conversations;
}

void
ConversationModelPimpl::sortConversations()
{

}

void
ConversationModelPimpl::slotContactsChanged()
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
ConversationModelPimpl::slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
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

int
ConversationModelPimpl::indexOf(const std::string& uid) const
{
    for (unsigned int i = 0; i < conversations.size() ; ++i) {
        if(conversations.at(i).uid == uid) return i;
    }
    return -1;
}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

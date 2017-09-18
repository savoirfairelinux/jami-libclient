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
#include <algorithm>

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

    // Add to db
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
    auto accountProfileId = db.select("id",
                                  "profiles",
                                  "uri=:uri",
                                  {{":uri", linked.owner.profile.uri}}).payloads;
    if (accountProfileId.empty()) {
        qDebug() << "ConversationModelPimpl::initConversations(), account not in bdd... abort";
        return;
    }
    auto conversationsForAccount = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", accountProfileId[0]}}).payloads;
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    for (auto const& contact : linked.owner.contactModel->getAllContacts())
    {
        // for each contact
        // TODO: split this
        auto contactinfo = contact;
        auto contactProfileId = db.select("id",
                                      "profiles",
                                      "uri=:uri",
                                      {{":uri", contactinfo.second->uri}}).payloads;
        if (contactProfileId.empty()) {
          qDebug() << "ConversationModelPimpl::initConversations(), contact not in bdd... abort";
          return;
        }
        // Get linked conversation with they
        auto conversationsForContact = db.select("id",
                                      "conversations",
                                      "participant_id=:participant_id",
                                      {{":participant_id", contactProfileId[0]}}).payloads;

        std::sort(conversationsForContact.begin(), conversationsForContact.end());
        std::vector<std::string> common;

        std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                              conversationsForContact.begin(), conversationsForContact.end(),
                              std::back_inserter(common));
        // Can't find a conversation with this contact. It's anormal, add one in the db
        if (common.empty()) {
            auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1",
                                          "conversations",
                                          "1=1",
                                          {}).payloads[0];
            db.insertInto("conversations",
                          {{":id", "id"}, {":participant_id", "participant_id"}},
                          {{":id", newConversationsId}, {":participant_id", accountProfileId[0]}});
            db.insertInto("conversations",
                          {{":id", "id"}, {":participant_id", "participant_id"}},
                          {{":id", newConversationsId}, {":participant_id", contactProfileId[0]}});
            // Add "Conversation started" message
            db.insertInto("interactions",
                          {{":account_id", "account_id"}, {":author_id", "author_id"},
                          {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                          {":group_id", "group_id"}, {":timestamp", "timestamp"},
                          {":body", "body"}, {":type", "type"},
                          {":status", "status"}},
                          {{":account_id", accountProfileId[0]}, {":author_id", accountProfileId[0]},
                          {":conversation_id", newConversationsId}, {":device_id", "0"},
                          {":group_id", "0"}, {":timestamp", "0"},
                          {":body", "Conversation started"}, {":type", "CONTACT"},
                          {":status", "SUCCEED"}});
            common.emplace_back(newConversationsId);
        }

        // Add the conversation
        conversation::Info conversation;
        conversation.uid = common[0];
        conversation.accountId = linked.owner.id;
        conversation.participants = {contactinfo.second->uri};
        conversation.callId = ""; // TODO update if current call.
        // Get messages
        auto messagesResult = db.select("id, body, timestamp, type, status",
                                        "interactions",
                                        "conversation_id=:conversation_id",
                                        {{":conversation_id", conversation.uid}});
        if (messagesResult.nbrOfCols == 5) {
            auto payloads = messagesResult.payloads;
            for (auto i = 0; i < payloads.size(); i += 5) {
                message::Info msg;
                msg.contact = contactinfo.second->uri;
                msg.body = payloads[i + 1];
                msg.timestamp = std::stoi(payloads[i + 2]);
                msg.type = message::StringToType(payloads[i + 3]);
                msg.status = message::StringToStatus(payloads[i + 4]);
                conversation.messages.emplace(std::make_pair<int, message::Info>(std::stoi(payloads[i]), std::move(msg)));
                conversation.lastMessageUid = std::stoi(payloads[i]);
            }
        }
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

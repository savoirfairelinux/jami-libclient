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
#include "api/call.h"
#include "callbackshandler.h"
#include "database.h"

#include "availableaccountmodel.h"
#include "namedirectory.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"

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
     * return a conversation index from conversations or -1 if no index is found.
     * @param uri of the contact to search.
     * @return an int.
     */
    int indexOfContact(const std::string& uri) const;
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
    Database& db;
    const CallbacksHandler& callbacksHandler;

    ConversationModel::ConversationQueue conversations;
    mutable ConversationModel::ConversationQueue filteredConversations;
    std::string filter;
    contact::Type typeFilter;

    // Database Helpers
    // TODO: move in another class.
    /**
     * @note the account must be in the database or it will fails
     * @return the id in the database for the current profile
     */
    std::string getAccountProfileId() const;
    /**
     * @param contactUri
     * @param alias
     * @param avatar
     * @return the id in the database for the current profile
     */
    std::string getOrInsertContact(const std::string& contactUri,
                                   const std::string& alias = "",
                                   const std::string& avatar = "") const;
    /**
     * @param accountProfile the id of the account in the database
     * @param contactProfile the id of the contact in the database
     * @return conversations id for conversations between account and contact
     */
    std::vector<std::string> getConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const;
    /**
     * Start a conversation between account and contact. Creates an entry in the conversations table
     * and an entry in the interactions table.
     * @param accountProfile the id of the account in the database
     * @param contactProfile the id of the contact in the database
     * @return conversation_id of the new conversation.
     */
    std::string beginConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const;
    /**
     * Remove a conversation between current account and a contact. Remove corresponding entries in
     * the conversations table and profiles if the profile is linked to no conversations.
     * @param contactUri
     */
    void removeContactFromDb(const std::string& contactUri) const;

public Q_SLOTS:
    void slotContactModelUpdated();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);
    void slotIncomingCall(const std::string& fromId, const std::string& callId);
    void slotCallStatusChanged(const std::string& callId);
    void slotNewAccountMessage(std::string& accountId,
                               std::string& from,
                               std::map<std::string,std::string> payloads);

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
    pimpl_->filteredConversations = pimpl_->conversations;

    auto filter = pimpl_->filter;
    auto typeFilter = pimpl_->typeFilter;
    auto it = std::copy_if(pimpl_->conversations.begin(), pimpl_->conversations.end(), pimpl_->filteredConversations.begin(),
    [&filter, &typeFilter, this] (const conversation::Info& entry) {
        auto contact = owner.contactModel->getContact(entry.participants.front());
        // Check type
        if (typeFilter != contact::Type::PENDING) {
            if (contact.type == contact::Type::PENDING) return false;
            if (contact.type == contact::Type::TEMPORARY) return true;
        } else {
            if (contact.type != contact::Type::PENDING) return false;
        }

        // Check contact
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
    pimpl_->filteredConversations.resize(std::distance(pimpl_->filteredConversations.begin(), it));
    return pimpl_->filteredConversations;
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

    auto& conversation = pimpl_->conversations.at(conversationIdx);

    if (conversation.participants.empty())
        return;

    auto contactUri = conversation.participants.front();
    auto contact = owner.contactModel->getContact(contactUri);

    // Send contact request if non used
    auto isNotUsed = contact.type == contact::Type::TEMPORARY
    || contact.type == contact::Type::PENDING;
    if (isNotUsed) {
        if (contactUri.length() == 0) {
            contactUri = owner.contactModel->getContact(contactUri).uri;
        }
        owner.contactModel->addContact({contactUri, "", "", "" , false, false, owner.profile.type});
    }

}

void
ConversationModel::selectConversation(const std::string& uid) const
{
    // Get conversation
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    auto participants = conversation.participants;

    // Check if conversation has a valid contact.
    if (participants.empty())
        return;

    try  {
        auto call = owner.callModel->getCall(conversation.callId);
        switch (call.status) {
            case call::Status::INCOMING_RINGING:
            case call::Status::OUTGOING_RINGING:
            case call::Status::CONNECTING:
            case call::Status::SEARCHING:
                // We are currently in a call
                emit showIncomingCallView(conversation);
                break;
            case call::Status::PAUSED:
            case call::Status::PEER_PAUSED:
            case call::Status::CONNECTED:
            case call::Status::IN_PROGRESS:
                // We are currently receiving a call
                emit showCallView(conversation);
                break;
            case call::Status::INVALID:
            case call::Status::OUTGOING_REQUESTED:
            case call::Status::INACTIVE:
            case call::Status::ENDED:
            case call::Status::TERMINATING:
            case call::Status::AUTO_ANSWERING:
            default:
                // We are not in a call, show the chatview
                emit showChatView(conversation);
        }
    } catch (const std::out_of_range&) {
        emit showChatView(conversation);
    }
}

void
ConversationModel::removeConversation(const std::string& uid, bool banned)
{
    // Get conversation
    auto i = std::find_if(pimpl_->conversations.begin(), pimpl_->conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });

    if (i == pimpl_->conversations.end())
        return;

    auto& conversation = *i;

    if (conversation.participants.empty())
        return;

    // Remove contact from daemon
    auto contact = conversation.participants.front();
    owner.contactModel->removeContact(contact, banned);
}

void
ConversationModel::placeCall(const std::string& uid) const
{
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);

    auto url = conversation.participants.front();
    if (owner.profile.type == contact::Type::RING) {
        url = "ring:" + url;
    }
    conversation.callId = owner.callModel->createCall(url);
    emit showIncomingCallView(conversation);
}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);

    if (conversation.participants.empty())
        return;

    auto account = AccountModel::instance().getById(owner.id.c_str());

    auto contact = owner.contactModel->getContact(conversation.participants.front());
    auto isNotUsed = contact.type == contact::Type::TEMPORARY
    || contact.type == contact::Type::PENDING;


    // Send contact request if non used
    if (isNotUsed)
        addConversation(contact.uri);

    // Send message to contact.
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    // TODO change this for group messages
    auto id = ConfigurationManager::instance().sendTextMessage(account->id(),
    contact.uri.c_str(), payloads);

    message::Info msg;
    msg.contact = contact.uri.c_str();
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;

    auto accountId = pimpl_->db.select("id", "profiles","uri=:uri", {{":uri", owner.profile.uri}}).payloads[0];
    auto peerId = pimpl_->db.select("id", "profiles","uri=:uri", {{":uri", contact.uri}}).payloads[0];
    auto conversations = pimpl_->getConversationsBetween(accountId, peerId);
    pimpl_->db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountId}, {":author_id", accountId},
                  {":conversation_id", conversations[0]}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", std::to_string(msg.timestamp)},
                  {":body", msg.body}, {":type", TypeToString(msg.type)},
                  {":status", StatusToString(msg.status)}});
}

void
ConversationModel::setFilter(const std::string& filter)
{
    pimpl_->filter = filter;
    owner.contactModel->searchContact(filter);
}

void
ConversationModel::setFilter(const contact::Type& filter)
{
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (!account) return;
    pimpl_->typeFilter = filter;
    emit modelUpdated();
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
, typeFilter(contact::Type::INVALID)
{
    initConversations();
    // [jn] those signals don't make sense anymore. We may have to recycle them, or just to delete them.
    // since adding a message from Conversation use insertInto wich return something, I guess we can deal with
    // messageAdded without signal.
    connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            this, &ConversationModelPimpl::slotContactModelUpdated);
    connect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
            this, &ConversationModelPimpl::slotIncomingCall);
    connect(&*linked.owner.contactModel, &ContactModel::incomingCallFromPending,
            this, &ConversationModelPimpl::slotIncomingCall);
    connect(&*linked.owner.callModel,
            &lrc::api::NewCallModel::callStatusChanged,
            this,
            &ConversationModelPimpl::slotCallStatusChanged);
    connect(&callbacksHandler, &lrc::CallbacksHandler::NewAccountMessage,
            this, &ConversationModelPimpl::slotNewAccountMessage);
}


ConversationModelPimpl::~ConversationModelPimpl()
{

}

void
ConversationModelPimpl::initConversations()
{
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());

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
        if(linked.owner.profile.uri == contact.second.uri) continue;
        // for each contact
        // TODO: split this
        auto contactinfo = contact;
        auto contactProfileId = db.select("id",
                                      "profiles",
                                      "uri=:uri",
                                      {{":uri", contactinfo.second.uri}}).payloads;
        if (contactProfileId.empty()) {
          qDebug() << "ConversationModelPimpl::initConversations(), contact not in bdd... abort";
          continue;
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
        qDebug() << contactinfo.second.uri.c_str();
        conversation.participants = {contactinfo.second.uri};
        try {
            conversation.callId = linked.owner.callModel->getCallFromURI(contactinfo.second.uri).id;
        } catch (...) {
            conversation.callId = "";
        }
        // Get messages
        auto messagesResult = db.select("id, body, timestamp, type, status",
                                        "interactions",
                                        "conversation_id=:conversation_id",
                                        {{":conversation_id", conversation.uid}});
        if (messagesResult.nbrOfCols == 5) {
            auto payloads = messagesResult.payloads;
            for (auto i = 0; i < payloads.size(); i += 5) {
                message::Info msg;
                msg.contact = contactinfo.second.uri;
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
ConversationModelPimpl::slotContactModelUpdated()
{
    initConversations();
    auto accountInfo = AvailableAccountModel::instance().currentDefaultAccount();
    if (!accountInfo) return;
    // We don't create newConversationItem if we already filter on pending
    conversation::Info newConversationItem;
    if (!filter.empty()) {
        // Create a conversation with the temporary item
        conversation::Info conversationInfo;
        auto temporaryContact = linked.owner.contactModel->getContact("");
        conversationInfo.uid = temporaryContact.uri;
        conversationInfo.participants.emplace_back("");
        conversationInfo.accountId = accountInfo->id().toStdString();
        // if temporary contact is already present, its alias is empty.
        if (!temporaryContact.alias.empty()) {
            if (!conversations.empty()) {
                auto firstContactUri = conversations.front().participants.front();
                if (!firstContactUri.empty()) {
                    conversations.emplace_front(conversationInfo);
                }
            } else {
                conversations.emplace_front(conversationInfo);
            }
        }
    } else {
        // No filter, so we can remove the newConversationItem
        if (!conversations.empty()) {
            auto firstContactUri = conversations.front().participants.front();
            if (firstContactUri.empty()) {
                conversations.pop_front();
            }
        }
    }
    emit linked.modelUpdated();
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

    // Add message to conversation
    conversation.messages.insert(std::pair<int, message::Info>(uid, msg));
    conversation.lastMessageUid = uid;

    emit linked.newUnreadMessage(msg.contact, msg);

    sortConversations();
    emit linked.modelUpdated();
}

int
ConversationModelPimpl::indexOf(const std::string& uid) const
{
    for (unsigned int i = 0; i < conversations.size() ; ++i) {
        if(conversations.at(i).uid == uid) return i;
    }
    return -1;
}

int
ConversationModelPimpl::indexOfContact(const std::string& uri) const
{
    for (unsigned int i = 0; i < conversations.size() ; ++i) {
        if(conversations.at(i).participants.front() == uri) return i;
    }
    return -1;
}

void
ConversationModelPimpl::slotIncomingCall(const std::string& fromId, const std::string& callId)
{
    auto conversationIdx = indexOfContact(fromId);

    if (conversationIdx == -1) {
        qDebug() << "ConversationModelPimpl::slotIncomingCall, but conversation not found";
        return; // Not a contact
    }

    auto& conversation = conversations.at(conversationIdx);

    qDebug() << "Add call to conversation with " << fromId.c_str();
    conversation.callId = callId;
    emit linked.showIncomingCallView(conversation);
}

void
ConversationModelPimpl::slotCallStatusChanged(const std::string& callId)
{
    // Get conversation
    auto i = std::find_if(
    conversations.begin(), conversations.end(),
    [callId](const conversation::Info& conversation) {
      return conversation.callId == callId;
    });

    if (i == conversations.end()) return;

    auto& conversation = *i;
    auto uid = conversation.uid;
    linked.selectConversation(uid);
}

void
ConversationModelPimpl::slotNewAccountMessage(std::string& accountId,
                                              std::string& from,
                                              std::map<std::string,std::string> payloads)
{
    if (accountId == linked.owner.id) {
        qDebug() << payloads["text/plain"].c_str();

        // [jn] il nous faut une table pour convertir accounit/uri et authorid/uri
        // [jn] il nous faut traiter le cas d'un message arrivant d'un peer inconnu.

        // Add "Conversation started" message
        auto row = getOrInsertContact(from);
        auto accountProfileId = getAccountProfileId();
        auto conversations = getConversationsBetween(accountProfileId, row);
        if (conversations.empty()) {
            conversations.emplace_back(beginConversationsBetween(accountProfileId, row));
        }
        auto lastMessageAdded = db.insertInto("interactions",
                                              {{":account_id", "account_id"}, {":author_id", "account_id"},
                                              {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                                              {":group_id", "group_id"}, {":timestamp", "timestamp"},
                                              {":body", "body"}, {":type", "type"},
                                              {":status", "status"}},
                                              {{":account_id", accountProfileId}, {":author_id", row},
                                              {":conversation_id", conversations[0]}, {":device_id", "0"},
                                              {":group_id", "0"}, {":timestamp", "0"},
                                              {":body", payloads["text/plain"].c_str()}, {":type", "TEXT"},
                                              {":status", "UNREAD"}});
        if (lastMessageAdded != -1) {
            message::Info msg;
            msg.contact = from;
            msg.body = payloads["text/plain"];
            msg.timestamp = std::time(nullptr);
            msg.type = message::Type::TEXT;
            msg.status = message::Status::UNREAD;
            emit linked.newUnreadMessage(from, msg);
        }
    }
}

// Database Helpers
std::string
ConversationModelPimpl::getAccountProfileId() const
{
    return db.select("id", "profiles","uri=:uri", {{":uri", linked.owner.profile.uri}}).payloads[0];
}

std::string
ConversationModelPimpl::getOrInsertContact(const std::string& contactUri,
                                      const std::string& alias,
                                      const std::string& avatar) const
{
    // Check if profile is already present.
    auto profileAlreadyExists = db.select("id",
                                                "profiles",
                                                "uri=:uri",
                                                {{":uri", contactUri}});
    if (profileAlreadyExists.payloads.empty()) {
        // Doesn't exists, add contact to the database
        auto type = TypeToString(linked.owner.profile.type);
        auto row = db.insertInto("profiles",
        {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
        {":status", "status"}},
        {{":uri", contactUri}, {":alias", alias}, {":photo", avatar}, {":type", type},
        {":status", "TRUSTED"}});

        if (row == -1) {
            qDebug() << "contact not added to the database";
            return "";
        }

        return std::to_string(row);
    } else {
        // Exists, update and retrieve it.
        if (!avatar.empty() || !alias.empty()) {
            db.update("profiles",
                      "alias=:alias, photo=:photo",
                      {{":alias", alias}, {":photo", avatar}},
                      "uri=:uri", {{":uri", contactUri}});
        }
        return profileAlreadyExists.payloads[0];
    }
}


std::vector<std::string>
ConversationModelPimpl::getConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const
{
    auto conversationsForAccount = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", accountProfile}}).payloads;
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    auto conversationsForContact = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", contactProfile}}).payloads;

    std::sort(conversationsForContact.begin(), conversationsForContact.end());
    std::vector<std::string> common;

    std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                          conversationsForContact.begin(), conversationsForContact.end(),
                          std::back_inserter(common));
    return common;
}

std::string
ConversationModelPimpl::beginConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const
{
    // Add conversation between account and profile
    auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1",
                                        "conversations",
                                        "1=1",
                                        {}).payloads[0];
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", accountProfile}});
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", contactProfile}});
    // Add "Conversation started" message
    db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountProfile}, {":author_id", accountProfile},
                  {":conversation_id", newConversationsId}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", "0"},
                  {":body", "Conversation started"}, {":type", "CONTACT"},
                  {":status", "SUCCEED"}});
    return newConversationsId;
}

void
ConversationModelPimpl::removeContactFromDb(const std::string& contactUri) const
{
    // Get profile for contact
    auto contactId = db.select("id", "profiles","uri=:uri", {{":uri", contactUri}}).payloads;
    if (contactId.empty()) return; // No profile
    // Get common conversations
    auto accountProfileId = getAccountProfileId();
    auto conversations = getConversationsBetween(accountProfileId, contactId[0]);
    // Remove conversations + interactions
    for (const auto& conversationId: conversations) {
        // Remove conversation
        db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
        // clear History
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
    }
    // Get conversations for this contact.
    conversations = db.select("id", "conversations","participant_id=:id", {{":id", contactId[0]}}).payloads;
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles", "id=:id", {{":id", contactId[0]}});
    }
}


} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

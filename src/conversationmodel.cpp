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

public Q_SLOTS:
    void slotContactModelUpdated();
    void slotMessageAdded(int uid, const std::string& accountId, const message::Info& msg);

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
        owner.contactModel->addContact(contactUri);
    }

}

void
ConversationModel::selectConversation(const std::string& uid)
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

    /*try  {
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
    } catch (const std::out_of_range&) {*/
        emit showChatView(conversation);
        qDebug() << "SHOW CHAT VIEW WITH " << conversation.uid.c_str();
    //}
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

    auto conversation = *i;

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
    //conversation.callId = owner.callModel->createCall(url);
    //emit showIncomingCallView(conversation);
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

    // TODO Add to database
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
    //~ connect(&db, &Database::messageAdded, this, &ConversationModelPimpl::slotMessageAdded);
    connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            this, &ConversationModelPimpl::slotContactModelUpdated);
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
        // for each contact
        // TODO: split this
        auto contactinfo = contact;
        auto contactProfileId = db.select("id",
                                      "profiles",
                                      "uri=:uri",
                                      {{":uri", contactinfo.second.uri}}).payloads;
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
        conversation.participants = {contactinfo.second.uri};
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

    emit linked.newMessageAdded(msg.contact, msg);

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

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

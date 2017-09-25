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
#include "authority/databasehelper.h"

#include "availableaccountmodel.h"
#include "namedirectory.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace authority;
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
    /**
     * Call contactModel.addContact if necessary
     * @param contactUri
     */
    void sendContactRequest(const std::string& contactUri);
    /**
     * Add a conversation with contactUri
     * @param convId
     * @param contactUri
     */
    void addConversationWith(const std::string& convId, const std::string& contactUri);
    /**
     * Add call message for conversation with callId
     * @param callId
     * @param body
     */
    void addCallMessage(const std::string& callId, const std::string& body);

    const ConversationModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;
    const std::string accountProfileId;

    // contains all conversations
    ConversationModel::ConversationQueue conversations;
    // contains conversations for client
    mutable ConversationModel::ConversationQueue filteredConversations;
    // filters
    std::string filter;
    contact::Type typeFilter;

public Q_SLOTS:
    /**
     * Listen from contactModel when updated (like new alias, avatar, etc.)
     */
    void slotContactModelUpdated();
    /**
     * Listen from contactModel when aa new contact is added
     * @param uri
     */
    void slotContactAdded(const std::string& uri);
    /**
     * Listen from contactModel when aa new contact is removed
     * @param uri
     */
    void slotContactRemoved(const std::string& uri);
    /**
     * Listen from acllmodel for new calls.
     * @param fromId caller uri
     * @param callId
     */
    void slotIncomingCall(const std::string& fromId, const std::string& callId);
    /**
     * Listen from callmodel for calls status changed.
     * @param callId
     */
    void slotCallStatusChanged(const std::string& callId);
    /**
     * Listen from callmodel for writing "Call started"
     * @param callId
     */
    void slotCallStarted(const std::string& callId);
    /**
     * Listen from callmodel for writing "Call ended"
     * @param callId
     */
    void slotCallEnded(const std::string& callId);
    /**
     * Listen from CallbacksHandler for new incoming messages;
     * @param accountId
     * @param from uri
     * @param payloads body
     */
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
        // TODO conferences?
        auto contact = owner.contactModel->getContact(entry.participants.front());
        // Check type
        if (typeFilter != contact::Type::PENDING) {
            // Remove pending contacts and get the temporary item if filter is not empty
            if (contact.type == contact::Type::PENDING) return false;
            if (contact.type == contact::Type::TEMPORARY) return !contact.alias.empty();
        } else {
            // We only want pending requests matching zith the filter
            if (contact.type != contact::Type::PENDING) return false;
        }

        // Check contact
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(contact.uri, regexFilter)
            | std::regex_search(contact.alias, regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring
            // of the uri or the alias.
            return contact.alias.find(filter) != std::string::npos
            && contact.uri.find(filter) != std::string::npos;
        }
    });
    pimpl_->filteredConversations.resize(
        std::distance(pimpl_->filteredConversations.begin(), it)
    );
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
    if (conversation.participants.empty()) {
        // Should not.
        qDebug() << "ConversationModel::addConversation can't add a conversation with no participant";
        return;
    }

    // Send contact request if non used
    pimpl_->sendContactRequest(conversation.participants.front());
}

void
ConversationModel::selectConversation(const std::string& uid) const
{
    // Get conversation
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    if (uid.empty() && owner.contactModel->getContact("").uri.empty()) {
        // if we select the temporary contact, check if its a valid contact.
        return;
    }

    auto& conversation = pimpl_->conversations.at(conversationIdx);
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
    auto conversationIdx = pimpl_->indexOf(uid);
    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    if (conversation.participants.empty()) {
        // Should not
        qDebug() << "ConversationModel::removeConversation can't remove a conversation with no participant";
        return;
    }

    // Remove contact from daemon
    // NOTE: this will also remove the conversation into the database.
    for (const auto& participant: conversation.participants)
    owner.contactModel->removeContact(participant, banned);
}

void
ConversationModel::placeCall(const std::string& uid) const
{
    auto conversationIdx = pimpl_->indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    if (conversation.participants.empty()) {
        // Should not
        qDebug() << "ConversationModel::placeCall can't call a conversation with no participant";
        return;
    }

    auto contactUri = conversation.participants.front();
    pimpl_->sendContactRequest(contactUri);
    auto url = owner.contactModel->getContact(contactUri).uri;
    if (owner.profile.type != contact::Type::SIP) {
        url = "ring:" + url; // Add the ring: before or it will fail.
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
    if (conversation.participants.empty()) {
        // Should not
        qDebug() << "ConversationModel::sendMessage can't send a message to a conversation with no participant";
        return;
    }

    // Send message to all participants
    // NOTE: conferences are not implemented yet, so we have only one participant
    for (const auto& participant: conversation.participants) {
        pimpl_->sendContactRequest(participant);
        owner.contactModel->sendDhtMessage(participant, body);
    }

    // Add message to database
    auto accountId = pimpl_->accountProfileId;
    auto msg = message::Info({accountId, body, std::time(nullptr),
                            message::Type::TEXT, message::Status::SENDING});
    int msgId = database::addMessageToConversation(pimpl_->db, accountId, uid, msg);
    // Update conversation
    conversation.messages.insert(std::pair<int, message::Info>(msgId, msg));
    conversation.lastMessageUid = msgId;
    // Emit this signal for chatview in the client
    emit newUnreadMessage(uid, msg);
    // This conversation is now at the top of the list
    pimpl_->sortConversations();
    // The order has changed, informs the client to redraw the list
    emit modelUpdated();

}

void
ConversationModel::setFilter(const std::string& filter)
{
    pimpl_->filter = filter;
    // Will update the temporary contact in the contactModel
    owner.contactModel->searchContact(filter);
}

void
ConversationModel::setFilter(const contact::Type& filter)
{
    // Switch between PENDING, RING and SIP contacts.
    pimpl_->typeFilter = filter;
    emit modelUpdated();
}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{
    Q_UNUSED(uid)
    Q_UNUSED(uri)
    // TODO when conferences.will be implemented
}

void
ConversationModel::clearHistory(const std::string& uid)
{
    auto conversationIdx = pimpl_->indexOf(uid);
    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    // Remove all TEXT messages from database
    database::clearHistory(pimpl_->db, uid);
    // Update conversation
    conversation.messages.clear();
    database::getHistory(pimpl_->db, conversation); // will contains "Conversation started"
    emit modelUpdated();
    // TODO signal for chatview in client?
}

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked,
                                               Database& db,
                                               const CallbacksHandler& callbacksHandler)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
, typeFilter(contact::Type::INVALID)
, accountProfileId(database::getProfileId(db, linked.owner.profile.uri))
{
    initConversations();

    // Contact related
    connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            this, &ConversationModelPimpl::slotContactModelUpdated);
    connect(&*linked.owner.contactModel, &ContactModel::contactAdded,
            this, &ConversationModelPimpl::slotContactAdded);
    connect(&*linked.owner.contactModel, &ContactModel::contactRemoved,
            this, &ConversationModelPimpl::slotContactRemoved);

    // Messages related
    connect(&*linked.owner.contactModel, &lrc::ContactModel::newAccountMessage,
            this, &ConversationModelPimpl::slotNewAccountMessage);

    // Call related
    connect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
            this, &ConversationModelPimpl::slotIncomingCall);
    connect(&*linked.owner.contactModel, &ContactModel::incomingCallFromPending,
            this, &ConversationModelPimpl::slotIncomingCall);
    connect(&*linked.owner.callModel,
            &lrc::api::NewCallModel::callStatusChanged,
            this,
            &ConversationModelPimpl::slotCallStatusChanged);
    connect(&*linked.owner.callModel,
            &lrc::api::NewCallModel::callStarted,
            this,
            &ConversationModelPimpl::slotCallStarted);
    connect(&*linked.owner.callModel,
            &lrc::api::NewCallModel::callEnded,
            this,
            &ConversationModelPimpl::slotCallEnded);
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

    // Fill conversations
    if (accountProfileId.empty()) {
        // Should not, NewAccountModel must create this profile before.
        qDebug() << "ConversationModelPimpl::initConversations(), account not in bdd... abort";
        return;
    }
    auto conversationsForAccount = database::getConversationsForProfile(db, accountProfileId);
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    for (auto const& contact : linked.owner.contactModel->getAllContacts())
    {
        if(linked.owner.profile.uri == contact.second.uri) continue;
        auto contactProfileId = database::getProfileId(db, contact.second.uri);
        if (contactProfileId.empty()) {
            // Should not, ContactModel must create profiles before.
            qDebug() << "ConversationModelPimpl::initConversations(), contact not in bdd... abort";
            continue;
        }
        // Get linked conversation with contact
        auto conversationsForContact = database::getConversationsForProfile(db, contactProfileId);
        std::sort(conversationsForContact.begin(), conversationsForContact.end());
        std::vector<std::string> common;
        std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                              conversationsForContact.begin(), conversationsForContact.end(),
                              std::back_inserter(common));
        if (common.empty()) {
            // Can't find a conversation with this contact. Start it.
            auto newConversationsId = database::beginConversationsBetween(db, accountProfileId, contactProfileId);
            common.emplace_back(newConversationsId);
        }

        addConversationWith(common[0], contact.first);
    }

    sortConversations();
    filteredConversations = conversations;
}

void
ConversationModelPimpl::sortConversations()
{
    std::sort(conversations.begin(), conversations.end(),
    [](const conversation::Info& conversationA, const conversation::Info& conversationB)
    {
        auto historyA = conversationA.messages;
        auto historyB = conversationB.messages;
        // A or B is a new conversation (without CONTACT message)
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
ConversationModelPimpl::sendContactRequest(const std::string& contactUri)
{
    auto contact = linked.owner.contactModel->getContact(contactUri);
    auto isNotUsed = contact.type == contact::Type::TEMPORARY
    || contact.type == contact::Type::PENDING;;
    if (isNotUsed) linked.owner.contactModel->addContact(contact);
}

void
ConversationModelPimpl::slotContactAdded(const std::string& uri)
{
    auto contactProfileId = database::getOrInsertProfile(db, uri);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conv.empty()) {
        conv.emplace_back(database::beginConversationsBetween(db, accountProfileId, contactProfileId));
    }
    // Add the conversation if not already here
    if (indexOf(conv[0]) == -1)
        addConversationWith(conv[0], uri);
    sortConversations();
    emit linked.modelUpdated();
}

void
ConversationModelPimpl::slotContactRemoved(const std::string& uri)
{
    auto conversationIdx = indexOfContact(uri);
    if (conversationIdx == -1) {
        qDebug() << "ConversationModelPimpl::slotContactRemoved, but conversation not found";
        return; // Not a contact
    }
    conversations.erase(conversations.begin() + conversationIdx);
    // Already sorted
    emit linked.modelUpdated();
}


void
ConversationModelPimpl::slotContactModelUpdated()
{
    // We don't create newConversationItem if we already filter on pending
    conversation::Info newConversationItem;
    if (!filter.empty()) {
        // Create a conversation with the temporary item
        conversation::Info conversationInfo;
        auto temporaryContact = linked.owner.contactModel->getContact("");
        conversationInfo.uid = temporaryContact.uri;
        conversationInfo.participants.emplace_back("");
        conversationInfo.accountId = linked.owner.id;
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
ConversationModelPimpl::addConversationWith(const std::string& convId,
                                            const std::string& contactUri)
{
    conversation::Info conversation;
    conversation.uid = convId;
    conversation.accountId = linked.owner.id;
    conversation.participants = {contactUri};
    try {
        conversation.callId = linked.owner.callModel->getCallFromURI(contactUri).id;
    } catch (...) {
        conversation.callId = "";
    }
    database::getHistory(db, conversation);
    conversations.emplace_front(conversation);
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
ConversationModelPimpl::slotCallStarted(const std::string& callId)
{
    addCallMessage(callId, "📞 Conversation started");
}

void
ConversationModelPimpl::slotCallEnded(const std::string& callId)
{
    addCallMessage(callId, "🕽 Conversation ended");
}

void
ConversationModelPimpl::addCallMessage(const std::string& callId, const std::string& body)
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
    auto msg = message::Info({accountProfileId, body, std::time(nullptr),
                            message::Type::CALL, message::Status::SUCCEED});
    int msgId = database::addMessageToConversation(db, accountProfileId, conversation.uid, msg);
    conversation.messages.emplace(msgId, msg);
    emit linked.newUnreadMessage(conversation.uid, msg);
    sortConversations();
    emit linked.modelUpdated();
}


void
ConversationModelPimpl::slotNewAccountMessage(std::string& accountId,
                                              std::string& from,
                                              std::map<std::string,std::string> payloads)
{
    if (accountId != linked.owner.id) return;
    qDebug() << payloads["text/plain"].c_str();

    // [jn] il nous faut une table pour convertir accounit/uri et authorid/uri
    // [jn] il nous faut traiter le cas d'un message arrivant d'un peer inconnu.

    // Add "Conversation started" message
    auto contactProfileId = database::getOrInsertProfile(db, from);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conv.empty()) {
        conv.emplace_back(database::beginConversationsBetween(db, accountProfileId, from));
    }
    auto msg = message::Info({contactProfileId, payloads["text/plain"], std::time(nullptr),
                            message::Type::TEXT, message::Status::UNREAD});
    int msgId = database::addMessageToConversation(db, accountProfileId, conv[0], msg);
    auto conversationIdx = indexOf(conv[0]);
    // Add the conversation if not already here
    if (conversationIdx == -1) {
        addConversationWith(conv[0], from);
    } else {
        conversations[conversationIdx].messages.emplace(msgId, msg);
    }
    emit linked.newUnreadMessage(conv[0], msg);
    sortConversations();
    emit linked.modelUpdated();
}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

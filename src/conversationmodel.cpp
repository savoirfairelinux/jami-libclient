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
#include "api/behaviorcontroller.h"
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
                           const CallbacksHandler& callbacksHandler,
                           const BehaviorController& behaviorController);

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
     * Add call interaction for conversation with callId
     * @param callId
     * @param body
     */
    void addCallMessage(const std::string& callId, const std::string& body);
    /**
     * Add a new message from a peer in the database
     * @param from the peer uri
     * @param body the content of the message
     */
    void addIncomingMessage(const std::string& from, const std::string& body);

    const ConversationModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;
    const std::string accountProfileId;
    const BehaviorController& behaviorController;

    // contains all conversations
    ConversationModel::ConversationQueue conversations;
    // contains conversations for client
    mutable ConversationModel::ConversationQueue filteredConversations;
    // filters
    std::string filter;
    profile::Type typeFilter;

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
     * Listen from callmodel for new calls.
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
     * Listen from CallbacksHandler for new incoming interactions;
     * @param accountId
     * @param from uri
     * @param payloads body
     */
    void slotNewAccountMessage(std::string& accountId,
                               std::string& from,
                               std::map<std::string,std::string> payloads);
    /**
     * Listen from CallbacksHandler for new messages in a SIP call
     * @param callId call linked to the interaction
     * @param from author uri
     * @param body of the message
     */
    void slotIncomingCallMessage(const std::string& callId, const std::string& from, const std::string& body);

};

ConversationModel::ConversationModel(const account::Info& owner,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler,
                                     const BehaviorController& behaviorController)
: QObject()
, pimpl_(std::make_unique<ConversationModelPimpl>(*this, db, callbacksHandler, behaviorController))
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
        auto contactInfo = owner.contactModel->getContact(entry.participants.front());
        // Check type
        if (typeFilter != profile::Type::PENDING) {
            // Remove pending contacts and get the temporary item if filter is not empty
            if (contactInfo.profileInfo.type == profile::Type::PENDING) return false;
            if (contactInfo.profileInfo.type == profile::Type::TEMPORARY) return !contactInfo.profileInfo.alias.empty();
        } else {
            // We only want pending requests matching with the filter
            if (contactInfo.profileInfo.type != profile::Type::PENDING) return false;
        }

        // Check contact
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(contactInfo.profileInfo.uri, regexFilter)
            | std::regex_search(contactInfo.profileInfo.alias, regexFilter)
            | std::regex_search(contactInfo.registeredName, regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring
            // of the uri or the alias.
            return contactInfo.profileInfo.alias.find(filter) != std::string::npos
            && contactInfo.profileInfo.uri.find(filter) != std::string::npos
            && contactInfo.registeredName.find(filter) != std::string::npos;
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

    if (uid.empty() && owner.contactModel->getContact("").profileInfo.uri.empty()) {
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
                emit pimpl_->behaviorController.showIncomingCallView(owner.id, conversation);
                break;
            case call::Status::PAUSED:
            case call::Status::PEER_PAUSED:
            case call::Status::CONNECTED:
            case call::Status::IN_PROGRESS:
                // We are currently receiving a call
                emit pimpl_->behaviorController.showCallView(owner.id, conversation);
                break;
            case call::Status::INVALID:
            case call::Status::OUTGOING_REQUESTED:
            case call::Status::INACTIVE:
            case call::Status::ENDED:
            case call::Status::TERMINATING:
            case call::Status::AUTO_ANSWERING:
            default:
                // We are not in a call, show the chatview
                emit pimpl_->behaviorController.showChatView(owner.id, conversation);
        }
    } catch (const std::out_of_range&) {
        emit pimpl_->behaviorController.showChatView(owner.id, conversation);
    }
}

void
ConversationModel::removeConversation(const std::string& uid, bool banned) const
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

    auto convId = uid;
    auto accountId = pimpl_->accountProfileId;

    auto participant = conversation.participants.front();
    auto contactInfo = owner.contactModel->getContact(participant);
    auto url = contactInfo.profileInfo.uri;
    if (url.empty()) return; // Incorrect item
    pimpl_->sendContactRequest(participant);
    if (owner.profileInfo.type != profile::Type::SIP) {
        url = "ring:" + url; // Add the ring: before or it will fail.
    }
    conversation.callId = owner.callModel->createCall(url);
    if (convId.empty()) {
        // The conversation has changed because it was with the temporary item
        auto contactProfileId = database::getProfileId(pimpl_->db, contactInfo.profileInfo.uri);
        auto common = database::getConversationsBetween(pimpl_->db, accountId, contactProfileId);
        if (common.empty()) return;
        convId = common.front();
        // Get new conversation
        conversationIdx = pimpl_->indexOf(convId);
        if (conversationIdx == -1)
            return;
        conversation = pimpl_->conversations.at(conversationIdx);
    }
    emit pimpl_->behaviorController.showIncomingCallView(owner.id, conversation);
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
        qDebug() << "ConversationModel::sendMessage can't send a interaction to a conversation with no participant";
        return;
    }

    auto convId = uid;
    auto accountId = pimpl_->accountProfileId;

    // Send interaction to all participants
    // NOTE: conferences are not implemented yet, so we have only one participant
    for (const auto& participant: conversation.participants) {
        auto contactInfo = owner.contactModel->getContact(participant);
        pimpl_->sendContactRequest(participant);
        if (not conversation.callId.empty())
            owner.callModel->sendSipMessage(conversation.callId, body);
        else
            owner.contactModel->sendDhtMessage(contactInfo.profileInfo.uri, body);
        if (convId.empty()) {
            // The conversation has changed because it was with the temporary item
            auto contactProfileId = database::getProfileId(pimpl_->db, contactInfo.profileInfo.uri);
            auto common = database::getConversationsBetween(pimpl_->db, accountId, contactProfileId);
            if (common.empty()) return;
            convId = common.front();
            // Get new conversation
            conversationIdx = pimpl_->indexOf(convId);
            if (conversationIdx == -1)
                return;
            conversation = pimpl_->conversations.at(conversationIdx);
        }
    }

    // Add interaction to database
    auto msg = interaction::Info({accountId, body, std::time(nullptr),
                            interaction::Type::TEXT, interaction::Status::SENDING});
    int msgId = database::addMessageToConversation(pimpl_->db, accountId, convId, msg);
    // Update conversation
    conversation.interactions.insert(std::pair<int, interaction::Info>(msgId, msg));
    conversation.lastMessageUid = msgId;
    // Emit this signal for chatview in the client
    emit newUnreadMessage(convId, msg);
    // This conversation is now at the top of the list
    pimpl_->sortConversations();
    // The order has changed, informs the client to redraw the list
    emit modelSorted();

}

void
ConversationModel::setFilter(const std::string& filter)
{
    pimpl_->filter = filter;
    // Will update the temporary contact in the contactModel
    owner.contactModel->searchContact(filter);
    emit filterChanged();
}

void
ConversationModel::setFilter(const profile::Type& filter)
{
    // Switch between PENDING, RING and SIP contacts.
    pimpl_->typeFilter = filter;
    emit filterChanged();
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
    // Remove all TEXT interactions from database
    database::clearHistory(pimpl_->db, uid);
    // Update conversation
    conversation.interactions.clear();
    database::getHistory(pimpl_->db, conversation); // will contains "Conversation started"
    pimpl_->sortConversations();
    emit modelSorted();
    emit conversationCleared(uid);
}

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked,
                                               Database& db,
                                               const CallbacksHandler& callbacksHandler,
                                               const BehaviorController& behaviorController)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
, typeFilter(profile::Type::INVALID)
, accountProfileId(database::getProfileId(db, linked.owner.profileInfo.uri))
, behaviorController(behaviorController)
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
    connect(&callbacksHandler, &CallbacksHandler::incomingCallMessage,
            this, &ConversationModelPimpl::slotIncomingCallMessage);

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
    auto* account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (!account)
        return;

    // Fill conversations
    if (accountProfileId.empty()) {
        // Should not, NewAccountModel must create this profile before.
        qDebug() << "ConversationModelPimpl::initConversations(), account not in db";
        return;
    }
    auto conversationsForAccount = database::getConversationsForProfile(db, accountProfileId);
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    for (auto const& c : linked.owner.contactModel->getAllContacts())
    {
        if(linked.owner.profileInfo.uri == c.second.profileInfo.uri)
            continue;

        auto contactProfileId = database::getProfileId(db, c.second.profileInfo.uri);
        if (contactProfileId.empty()) {
            // Should not, ContactModel must create profiles before.
            qDebug() << "ConversationModelPimpl::initConversations(), contact not in db";
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

        addConversationWith(common[0], c.first);
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
        auto historyA = conversationA.interactions;
        auto historyB = conversationB.interactions;
        // A or B is a new conversation (without CONTACT interaction)
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
    auto isNotUsed = contact.profileInfo.type == profile::Type::TEMPORARY
        || contact.profileInfo.type == profile::Type::PENDING;
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
    emit linked.newConversation(conv[0]);
    sortConversations();
    auto firstContactUri = conversations.front().participants.front();
    if (firstContactUri.empty()) {
        conversations.pop_front();
    }
    emit linked.modelSorted();
}

void
ConversationModelPimpl::slotContactRemoved(const std::string& uri)
{
    auto conversationIdx = indexOfContact(uri);
    if (conversationIdx == -1) {
        qDebug() << "ConversationModelPimpl::slotContactRemoved, but conversation not found";
        return; // Not a contact
    }
    auto& conversationUid = conversations[conversationIdx].uid;
    conversations.erase(conversations.begin() + conversationIdx);
    emit linked.conversationRemoved(conversationUid);
    emit linked.modelSorted();
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
        conversationInfo.uid = temporaryContact.profileInfo.uri;
        conversationInfo.participants.emplace_back("");
        conversationInfo.accountId = linked.owner.id;
        // if temporary contact is already present, its alias is empty.
        if (!temporaryContact.profileInfo.alias.empty()) {
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
    emit linked.modelSorted();
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
    emit behaviorController.showIncomingCallView(linked.owner.id, conversation);
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
    addCallMessage(callId, "📞 Call started");
}

void
ConversationModelPimpl::slotCallEnded(const std::string& callId)
{
    addCallMessage(callId, "🕽 Call ended");

    // reset the callId stored in the conversation
    auto it = std::find_if(conversations.begin(), conversations.end(),
    [callId](const conversation::Info& conversation) {
      return conversation.callId == callId;
    });

    if (it != conversations.end())
        it->callId = "";
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
    auto msg = interaction::Info({accountProfileId, body, std::time(nullptr),
                            interaction::Type::CALL, interaction::Status::SUCCEED});
    int msgId = database::addMessageToConversation(db, accountProfileId, conversation.uid, msg);
    conversation.interactions.emplace(msgId, msg);
    conversation.lastMessageUid = msgId;
    emit linked.newUnreadMessage(conversation.uid, msg);
    sortConversations();
    emit linked.modelSorted();
}


void
ConversationModelPimpl::slotNewAccountMessage(std::string& accountId,
                                              std::string& from,
                                              std::map<std::string,std::string> payloads)
{
    if (accountId != linked.owner.id)
        return;

    addIncomingMessage(from, payloads["text/plain"]);
}

void
ConversationModelPimpl::slotIncomingCallMessage(const std::string& callId, const std::string& from, const std::string& body)
{
    if (not linked.owner.callModel->hasCall(callId))
        return;

    addIncomingMessage(from, body);
}

void
ConversationModelPimpl::addIncomingMessage(const std::string& from, const std::string& body)
{
    auto contactProfileId = database::getOrInsertProfile(db, from);
    auto accountProfileId = database::getProfileId(db, linked.owner.profileInfo.uri);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conv.empty()) {
        conv.emplace_back(database::beginConversationsBetween(db, accountProfileId, contactProfileId));
    }
    auto msg = interaction::Info({contactProfileId, body, std::time(nullptr),
                            interaction::Type::TEXT, interaction::Status::UNREAD});
    int msgId = database::addMessageToConversation(db, accountProfileId, conv[0], msg);
    auto conversationIdx = indexOf(conv[0]);
    // Add the conversation if not already here
    if (conversationIdx == -1) {
        addConversationWith(conv[0], from);
        emit linked.newConversation(conv[0]);
    } else {
        conversations[conversationIdx].interactions.emplace(msgId, msg);
        conversations[conversationIdx].lastMessageUid = msgId;
    }
    emit linked.newUnreadMessage(conv[0], msg);
    sortConversations();
    emit linked.modelSorted();
}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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

//Qt
#include <QtCore/QTimer>

// daemon
#include <account_const.h>
#include <datatransfer_interface.h>

// std
#include <algorithm>
#include <mutex>
#include <regex>

// LRC
#include "api/lrc.h"
#include "api/behaviorcontroller.h"
#include "api/contactmodel.h"
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"
#include "api/account.h"
#include "api/call.h"
#include "api/datatransfer.h"
#include "api/datatransfermodel.h"
#include "callbackshandler.h"
#include "authority/databasehelper.h"

#include "availableaccountmodel.h"
#include "namedirectory.h"
#include "phonedirectorymodel.h"
#include "contactmethod.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"

namespace lrc
{

using namespace authority;
using namespace api;

class ConversationModelPimpl : public QObject
{
    Q_OBJECT
public:
    ConversationModelPimpl(const ConversationModel& linked,
                           Lrc& lrc,
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
    void addOrUpdateCallMessage(const std::string& callId, const std::string& body);
    /**
     * Add a new message from a peer in the database
     * @param from the peer uri
     * @param body the content of the message
     * @param authorProfileId override the author of the message (if empty it's from)*/
    void addIncomingMessage(const std::string& from,
                            const std::string& body,
                            const std::string& authorProfileId="");
    /**
     * Change the status of an interaction. Listen from callbacksHandler
     * @param accountId, account linked
     * @param id, interaction to update
     * @param to, peer uri
     * @param status, new status for this interaction
     */
    void slotUpdateInteractionStatus(const std::string& accountId,
                                     const uint64_t id,
                                     const std::string& to, int status);

    /**
     * place a call
     * @param uid, conversation id
     * @param isAudioOnly, allow to specify if the call is only audio. Set to false by default.
     */
    void placeCall(const std::string& uid, bool isAudioOnly = false);

    /**
     * get number of unread messages
     */
    int getNumberOfUnreadMessagesFor(const std::string& uid);

    /**
     * Handle data transfer progression
     */
    void updateTransfer(QTimer* timer, const std::string& conversation, int conversationIdx,
                        int interactionId);

    bool usefulDataFromDataTransfer(long long dringId, const datatransfer::Info& info,
                                    int& interactionId, std::string& convId);

    const ConversationModel& linked;
    Lrc& lrc;
    Database& db;
    const CallbacksHandler& callbacksHandler;
    const std::string accountProfileId;
    const BehaviorController& behaviorController;

    ConversationModel::ConversationQueue conversations; ///< non-filtered conversations
    ConversationModel::ConversationQueue filteredConversations;
    ConversationModel::ConversationQueue customFilteredConversations;
    std::string filter;
    profile::Type typeFilter;
    profile::Type customTypeFilter;
    std::pair<bool, bool> dirtyConversations {true, true}; ///< true if filteredConversations/customFilteredConversations must be regenerated
    std::map<std::string, std::mutex> interactionsLocks; ///< {convId, mutex}

public Q_SLOTS:
    /**
     * Listen from contactModel when updated (like new alias, avatar, etc.)
     */
    void slotContactModelUpdated(const std::string& uri);
    /**
     * Listen from contactModel when a new contact is added
     * @param uri
     */
    void slotContactAdded(const std::string& uri);
    /**
     * Listen from contactModel when a pending contact is accepted
     * @param uri
     */
    void slotPendingContactAccepted(const std::string& uri);
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
    /**
     * Listen from CallModel when a call is added to a conference
     * @param callId
     * @param confId
     */
    void slotCallAddedToConference(const std::string& callId, const std::string& confId);
    /**
     * Listen from CallbacksHandler when a conference is deleted.
     * @param confId
     */
    void slotConferenceRemoved(const std::string& confId);

    void slotTransferStatusCreated(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusCanceled(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusAwaitingPeer(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusAwaitingHost(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusOngoing(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusFinished(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusError(long long dringId, api::datatransfer::Info info);
    void slotTransferStatusUnjoinable(long long dringId, api::datatransfer::Info info);
};

ConversationModel::ConversationModel(const account::Info& owner,
                                     Lrc& lrc,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler,
                                     const BehaviorController& behaviorController)
: QObject()
, pimpl_(std::make_unique<ConversationModelPimpl>(*this, lrc, db, callbacksHandler, behaviorController))
, owner(owner)
{

}

ConversationModel::~ConversationModel()
{

}

const ConversationModel::ConversationQueue&
ConversationModel::allFilteredConversations() const
{
    if (!pimpl_->dirtyConversations.first)
        return pimpl_->filteredConversations;

    pimpl_->filteredConversations = pimpl_->conversations;

    auto it = std::copy_if(
        pimpl_->conversations.begin(), pimpl_->conversations.end(),
        pimpl_->filteredConversations.begin(),
        [this] (const conversation::Info& entry) {
            auto contactInfo = owner.contactModel->getContact(entry.participants.front());
            // Check type
            if (pimpl_->typeFilter != profile::Type::PENDING) {
                // Remove pending contacts and get the temporary item if filter is not empty
                if (contactInfo.profileInfo.type == profile::Type::PENDING)
                    return false;
                if (contactInfo.profileInfo.type == profile::Type::TEMPORARY)
                    return not contactInfo.profileInfo.alias.empty() || not contactInfo.profileInfo.uri.empty();
            } else {
                // We only want pending requests matching with the filter
                if (contactInfo.profileInfo.type != profile::Type::PENDING)
                    return false;
            }

            // Check contact
            try {
                auto regexFilter = std::regex(pimpl_->filter, std::regex_constants::icase);
                bool result = std::regex_search(contactInfo.profileInfo.uri, regexFilter)
                | std::regex_search(contactInfo.profileInfo.alias, regexFilter)
                | std::regex_search(contactInfo.registeredName, regexFilter);
                return result;
            } catch(std::regex_error&) {
                // If the regex is incorrect, just test if filter is a substring
                // of the uri or the alias.
                return contactInfo.profileInfo.alias.find(pimpl_->filter) != std::string::npos
                && contactInfo.profileInfo.uri.find(pimpl_->filter) != std::string::npos
                && contactInfo.registeredName.find(pimpl_->filter) != std::string::npos;
            }
        });
    pimpl_->filteredConversations.resize(std::distance(pimpl_->filteredConversations.begin(), it));
    pimpl_->dirtyConversations.first = false;
    return pimpl_->filteredConversations;
}

const ConversationModel::ConversationQueue&
ConversationModel::getFilteredConversations(const profile::Type& filter) const
{
    if (pimpl_->customTypeFilter == filter && !pimpl_->dirtyConversations.second)
        return pimpl_->customFilteredConversations;

    pimpl_->customTypeFilter = filter;
    pimpl_->customFilteredConversations = pimpl_->conversations;

    auto it = std::copy_if(
        pimpl_->conversations.begin(), pimpl_->conversations.end(),
        pimpl_->customFilteredConversations.begin(),
        [this] (const conversation::Info& entry) {
            auto contactInfo = owner.contactModel->getContact(entry.participants.front());
            return (contactInfo.profileInfo.type == pimpl_->customTypeFilter);
        });
    pimpl_->customFilteredConversations.resize(std::distance(pimpl_->customFilteredConversations.begin(), it));
    pimpl_->dirtyConversations.second = false;
    return pimpl_->customFilteredConversations;
}

conversation::Info
ConversationModel::filteredConversation(const unsigned int row) const
{
    const auto& conversations = allFilteredConversations();
    if (row >= conversations.size())
        return conversation::Info();

    auto conversationInfo = conversations.at(row);
    conversationInfo.unreadMessages = pimpl_->getNumberOfUnreadMessagesFor(conversationInfo.uid);

    return conversationInfo;
}

void
ConversationModel::makePermanent(const std::string& uid)
{
    auto conversationIdx = pimpl_->indexOf(uid);
    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    if (conversation.participants.empty()) {
        // Should not
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
        if (not conversation.confId.empty()) {
            emit pimpl_->behaviorController.showCallView(owner.id, conversation);
        } else {
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
        }
    } catch (const std::out_of_range&) {
        emit pimpl_->behaviorController.showChatView(owner.id, conversation);
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
        qDebug() << "ConversationModel::removeConversation can't remove a conversation without participant";
        return;
    }

    // Remove contact from daemon
    // NOTE: this will also remove the conversation into the database.
    for (const auto& participant: conversation.participants)
        owner.contactModel->removeContact(participant, banned);
}

void
ConversationModel::deleteObsoleteHistory(int days)
{
    if(days < 1)
        return; // unlimited history

    auto currentTime = static_cast<long int>(std::time(nullptr)); // since epoch, in seconds...
    auto date = currentTime - (days * 86400);

    database::deleteObsoleteHistory(pimpl_->db, date);
}

void
ConversationModelPimpl::placeCall(const std::string& uid, bool isAudioOnly)
{
    auto conversationIdx = indexOf(uid);

    if (conversationIdx == -1)
        return;

    auto& conversation = conversations.at(conversationIdx);
    if (conversation.participants.empty()) {
        // Should not
        qDebug() << "ConversationModel::placeCall can't call a conversation without participant";
        return;
    }

    // Disallow multiple call
    if (!conversation.callId.empty()) {
        try  {
            auto call = linked.owner.callModel->getCall(conversation.callId);
            switch (call.status) {
                case call::Status::INCOMING_RINGING:
                case call::Status::OUTGOING_RINGING:
                case call::Status::CONNECTING:
                case call::Status::SEARCHING:
                case call::Status::PAUSED:
                case call::Status::PEER_PAUSED:
                case call::Status::CONNECTED:
                case call::Status::IN_PROGRESS:
                case call::Status::OUTGOING_REQUESTED:
                case call::Status::AUTO_ANSWERING:
                    return;
                case call::Status::INVALID:
                case call::Status::INACTIVE:
                case call::Status::ENDED:
                case call::Status::TERMINATING:
                default:
                    break;
            }
        } catch (const std::out_of_range&) {
        }
    }

    auto convId = uid;
    auto accountId = accountProfileId;

    auto participant = conversation.participants.front();
    bool isTemporary = participant.empty();
    auto contactInfo = linked.owner.contactModel->getContact(participant);
    auto url = contactInfo.profileInfo.uri;

    if (url.empty())
        return; // Incorrect item

    sendContactRequest(participant);

    if (linked.owner.profileInfo.type != profile::Type::SIP) {
        url = "ring:" + url; // Add the ring: before or it will fail.
    }

    // If call is with temporary contact, conversation has been removed and must be updated
    int contactIndex;
    if (isTemporary && (contactIndex = indexOfContact(convId)) < 0) {
        qDebug() << "Can't place call: Other participant is not a contact (most likely banned)";
        return;
    }

    auto& newConv = isTemporary ? conversations.at(contactIndex) : conversation;
    convId = newConv.uid;

    newConv.callId = linked.owner.callModel->createCall(url, isAudioOnly);

    dirtyConversations = {true, true};
    emit behaviorController.showIncomingCallView(linked.owner.id, newConv);
}

void
ConversationModel::placeAudioOnlyCall(const std::string& uid)
{
    pimpl_->placeCall(uid, true);
}

void
ConversationModel::placeCall(const std::string& uid)
{
    pimpl_->placeCall(uid);
}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body)
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
    bool isTemporary = conversation.participants.front() == "";

    // Send interaction to all participants
    // NOTE: conferences are not implemented yet, so we have only one participant
    uint64_t daemonMsgId = 0;
    auto status = interaction::Status::SENDING;
    for (const auto& participant: conversation.participants) {
        auto contactInfo = owner.contactModel->getContact(participant);
        pimpl_->sendContactRequest(participant);

        QStringList callLists = CallManager::instance().getCallList(); // no auto
        // workaround: sometimes, it may happen that the daemon delete a call, but lrc don't. We check if the call is
        //             still valid every time the user want to send a message.
        if (not conversation.callId.empty() and not callLists.contains(conversation.callId.c_str()))
            conversation.callId.clear();

        if (not conversation.callId.empty()
            and call::canSendSIPMessage(owner.callModel->getCall(conversation.callId))) {
            status = interaction::Status::UNKNOWN;
            owner.callModel->sendSipMessage(conversation.callId, body);

        } else
            daemonMsgId = owner.contactModel->sendDhtMessage(contactInfo.profileInfo.uri, body);

    }

    // If first interaction with temporary contact, we have to update the conversations info
    // at this stage
    int contactIndex;
    if (isTemporary && (contactIndex = pimpl_->indexOfContact(convId)) < 0) {
        qDebug() << "Can't send message: Other participant is not a contact (most likely banned)";
        return;
    }

    auto& newConv = isTemporary ? pimpl_->conversations.at(contactIndex) : conversation;
    convId = newConv.uid;

    // Add interaction to database
    auto msg = interaction::Info {accountId, body, std::time(nullptr),
                                  interaction::Type::TEXT, status};
    int msgId = database::addMessageToConversation(pimpl_->db, accountId, convId, msg);
    // Update conversation
    if (status == interaction::Status::SENDING) {
        // Because the daemon already give an id for the message, we need to store it.
        database::addDaemonMsgId(pimpl_->db, std::to_string(msgId), std::to_string(daemonMsgId));
    }
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
        newConv.interactions.insert(std::pair<uint64_t, interaction::Info>(msgId, msg));
    }
    newConv.lastMessageUid = msgId;
    pimpl_->dirtyConversations = {true, true};
    // Emit this signal for chatview in the client
    emit newInteraction(convId, msgId, msg);
    // This conversation is now at the top of the list
    pimpl_->sortConversations();
    // The order has changed, informs the client to redraw the list
    emit modelSorted();
}

void
ConversationModel::setFilter(const std::string& filter)
{
    pimpl_->filter = filter;
    pimpl_->dirtyConversations = {true, true};
    // Will update the temporary contact in the contactModel
    owner.contactModel->searchContact(filter);
    emit filterChanged();
}

void
ConversationModel::setFilter(const profile::Type& filter)
{
    // Switch between PENDING, RING and SIP contacts.
    pimpl_->typeFilter = filter;
    pimpl_->dirtyConversations = {true, true};
    emit filterChanged();
}

void
ConversationModel::joinConversations(const std::string& uidA, const std::string& uidB)
{
    auto conversationAIdx = pimpl_->indexOf(uidA);
    auto conversationBIdx = pimpl_->indexOf(uidB);
    if (conversationAIdx == -1 || conversationBIdx == -1)
        return;
    auto& conversationA = pimpl_->conversations[conversationAIdx];
    auto& conversationB = pimpl_->conversations[conversationBIdx];

    if (conversationA.callId.empty() || conversationB.callId.empty())
        return;

    if (conversationA.confId.empty()) {
        if(conversationB.confId.empty()){
            owner.callModel->joinCalls(conversationA.callId, conversationB.callId);
        }else{
            owner.callModel->joinCalls(conversationA.callId, conversationB.confId);
            conversationA.confId = conversationB.confId;
        }
    } else {
        if(conversationB.confId.empty()){
            owner.callModel->joinCalls(conversationA.confId, conversationB.callId);
            conversationB.confId = conversationA.confId;
        }else{
            owner.callModel->joinCalls(conversationA.confId, conversationB.confId);
            conversationB.confId = conversationA.confId;
        }
    }
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
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[uid]);
        conversation.interactions.clear();
    }
    database::getHistory(pimpl_->db, conversation); // will contains "Conversation started"
    pimpl_->sortConversations();
    emit modelSorted();
    emit conversationCleared(uid);
}

void
ConversationModel::clearAllHistory()
{
    database::clearAllHistoryFor(pimpl_->db, owner.profileInfo.uri);

    for (auto& conversation : pimpl_->conversations) {
        {
            std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[conversation.uid]);
            conversation.interactions.clear();
        }
        database::getHistory(pimpl_->db, conversation);
    }
}

void
ConversationModel::setInteractionRead(const std::string& convId,
                                      const uint64_t& interactionId)
{
    auto conversationIdx = pimpl_->indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        auto newStatus = interaction::Status::READ;
        {
            std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
            auto& interactions = pimpl_->conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                if (it->second.status != interaction::Status::UNREAD) return;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            pimpl_->dirtyConversations = {true, true};
            database::updateInteractionStatus(pimpl_->db, interactionId, newStatus);
            emit interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

ConversationModelPimpl::ConversationModelPimpl(const ConversationModel& linked,
                                               Lrc& lrc,
                                               Database& db,
                                               const CallbacksHandler& callbacksHandler,
                                               const BehaviorController& behaviorController)
: linked(linked)
, lrc {lrc}
, db(db)
, callbacksHandler(callbacksHandler)
, typeFilter(profile::Type::INVALID)
, customTypeFilter(profile::Type::INVALID)
, accountProfileId(database::getProfileId(db, linked.owner.profileInfo.uri))
, behaviorController(behaviorController)
{
    initConversations();

    // Contact related
    connect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
            this, &ConversationModelPimpl::slotContactModelUpdated);
    connect(&*linked.owner.contactModel, &ContactModel::contactAdded,
            this, &ConversationModelPimpl::slotContactAdded);
    connect(&*linked.owner.contactModel, &ContactModel::pendingContactAccepted,
            this, &ConversationModelPimpl::slotPendingContactAccepted);
    connect(&*linked.owner.contactModel, &ContactModel::contactRemoved,
            this, &ConversationModelPimpl::slotContactRemoved);

    // Messages related
    connect(&*linked.owner.contactModel, &lrc::ContactModel::newAccountMessage,
            this, &ConversationModelPimpl::slotNewAccountMessage);
    connect(&callbacksHandler, &CallbacksHandler::incomingCallMessage,
            this, &ConversationModelPimpl::slotIncomingCallMessage);
    connect(&callbacksHandler, &CallbacksHandler::accountMessageStatusChanged,
            this, &ConversationModelPimpl::slotUpdateInteractionStatus);


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
    connect(&*linked.owner.callModel,
            &lrc::api::NewCallModel::callAddedToConference,
            this,
            &ConversationModelPimpl::slotCallAddedToConference);
    connect(&callbacksHandler,
            &CallbacksHandler::conferenceRemoved,
            this,
            &ConversationModelPimpl::slotConferenceRemoved);

    // data transfer
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusCreated,
            this,
            &ConversationModelPimpl::slotTransferStatusCreated);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusCanceled,
            this,
            &ConversationModelPimpl::slotTransferStatusCanceled);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusAwaitingPeer,
            this,
            &ConversationModelPimpl::slotTransferStatusAwaitingPeer);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusAwaitingHost,
            this,
            &ConversationModelPimpl::slotTransferStatusAwaitingHost);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusOngoing,
            this,
            &ConversationModelPimpl::slotTransferStatusOngoing);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusFinished,
            this,
            &ConversationModelPimpl::slotTransferStatusFinished);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusError,
            this,
            &ConversationModelPimpl::slotTransferStatusError);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusUnjoinable,
            this,
            &ConversationModelPimpl::slotTransferStatusUnjoinable);
}

ConversationModelPimpl::~ConversationModelPimpl()
{
    // Contact related
    disconnect(&*linked.owner.contactModel, &ContactModel::modelUpdated,
               this, &ConversationModelPimpl::slotContactModelUpdated);
    disconnect(&*linked.owner.contactModel, &ContactModel::contactAdded,
               this, &ConversationModelPimpl::slotContactAdded);
    disconnect(&*linked.owner.contactModel, &ContactModel::pendingContactAccepted,
               this, &ConversationModelPimpl::slotPendingContactAccepted);
    disconnect(&*linked.owner.contactModel, &ContactModel::contactRemoved,
               this, &ConversationModelPimpl::slotContactRemoved);

    // Messages related
    disconnect(&*linked.owner.contactModel, &lrc::ContactModel::newAccountMessage,
               this, &ConversationModelPimpl::slotNewAccountMessage);
    disconnect(&callbacksHandler, &CallbacksHandler::incomingCallMessage,
               this, &ConversationModelPimpl::slotIncomingCallMessage);
    disconnect(&callbacksHandler, &CallbacksHandler::accountMessageStatusChanged,
               this, &ConversationModelPimpl::slotUpdateInteractionStatus);


    // Call related
    disconnect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
               this, &ConversationModelPimpl::slotIncomingCall);
    disconnect(&*linked.owner.contactModel, &ContactModel::incomingCallFromPending,
               this, &ConversationModelPimpl::slotIncomingCall);
    disconnect(&*linked.owner.callModel, &lrc::api::NewCallModel::callStatusChanged,
               this, &ConversationModelPimpl::slotCallStatusChanged);
    disconnect(&*linked.owner.callModel, &lrc::api::NewCallModel::callStarted,
               this, &ConversationModelPimpl::slotCallStarted);
    disconnect(&*linked.owner.callModel, &lrc::api::NewCallModel::callEnded,
               this, &ConversationModelPimpl::slotCallEnded);
    disconnect(&*linked.owner.callModel, &lrc::api::NewCallModel::callAddedToConference,
               this, &ConversationModelPimpl::slotCallAddedToConference);
    disconnect(&callbacksHandler, &CallbacksHandler::conferenceRemoved,
               this, &ConversationModelPimpl::slotConferenceRemoved);

    // data transfer
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusCreated,
               this, &ConversationModelPimpl::slotTransferStatusCreated);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusCanceled,
               this, &ConversationModelPimpl::slotTransferStatusCanceled);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusAwaitingPeer,
               this, &ConversationModelPimpl::slotTransferStatusAwaitingPeer);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusAwaitingHost,
               this, &ConversationModelPimpl::slotTransferStatusAwaitingHost);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusOngoing,
               this, &ConversationModelPimpl::slotTransferStatusOngoing);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusFinished,
               this, &ConversationModelPimpl::slotTransferStatusFinished);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusError,
               this, &ConversationModelPimpl::slotTransferStatusError);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusUnjoinable,
               this, &ConversationModelPimpl::slotTransferStatusUnjoinable);
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
        auto common = database::getConversationsBetween(db, accountProfileId, contactProfileId);
        if (common.empty()) {
            // Can't find a conversation with this contact. Start it.
            auto newConversationsId = database::beginConversationsBetween(db, accountProfileId, contactProfileId);
            common.emplace_back(std::move(newConversationsId));
        }

        addConversationWith(common[0], c.first);

        auto convIdx = indexOf(common[0]);

        // Check if file transfer interactions were left in an incorrect state
        std::lock_guard<std::mutex> lk(interactionsLocks[conversations[convIdx].uid]);
        for(auto& interaction : conversations[convIdx].interactions) {
            if (interaction.second.status == interaction::Status::TRANSFER_CREATED
                || interaction.second.status == interaction::Status::TRANSFER_AWAITING_HOST
                || interaction.second.status == interaction::Status::TRANSFER_AWAITING_PEER
                || interaction.second.status == interaction::Status::TRANSFER_ONGOING
                || interaction.second.status == interaction::Status::TRANSFER_ACCEPTED) {
                // If a datatransfer was left in a non-terminal status in DB, we switch this status to ERROR
                // TODO : Improve for DBus clients as daemon and transfer may still be ongoing
                database::updateInteractionStatus(db, interaction.first, interaction::Status::TRANSFER_ERROR);
                interaction.second.status = interaction::Status::TRANSFER_ERROR;
            }
        }
    }

    sortConversations();
    filteredConversations = conversations;
    dirtyConversations.first = false;
}

void
ConversationModelPimpl::sortConversations()
{
    std::sort(
        conversations.begin(), conversations.end(),
        [this](const auto& conversationA, const auto& conversationB)
        {
            // A or B is a temporary contact
            if (conversationA.participants.empty()) return true;
            if (conversationB.participants.empty()) return false;

            std::unique_lock<std::mutex> lockConvA(interactionsLocks[conversationA.uid], std::defer_lock);
            std::unique_lock<std::mutex> lockConvB(interactionsLocks[conversationB.uid], std::defer_lock);
            std::lock(lockConvA, lockConvB);

            auto historyA = conversationA.interactions;
            auto historyB = conversationB.interactions;
            // A or B is a new conversation (without CONTACT interaction)
            if (conversationA.uid.empty() || conversationB.uid.empty()) return conversationA.uid.empty();
            if (historyA.empty()) return false;
            if (historyB.empty()) return true;
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
                return false;
            }
        });
    dirtyConversations = {true, true};
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
        std::string interaction = "";
        try {
            auto contact = linked.owner.contactModel->getContact(uri);
            interaction = contact.profileInfo.type == profile::Type::PENDING ?
                QObject::tr("Invitation received").toStdString() :
                QObject::tr("Contact added").toStdString();
        } catch (...) {}

        // pass conversation UID through only element
        conv.emplace_back(
            database::beginConversationsBetween(db, accountProfileId,
                contactProfileId, interaction
            )
        );

    }
    // Add the conversation if not already here
    if (indexOf(conv[0]) == -1) {
        addConversationWith(conv[0], uri);
        emit linked.newConversation(conv[0]);
    }

    // delete temporary conversation if it exists and it has the uri of the added contact as uid
    if (indexOf(uri) >= 0) {
        conversations.erase(conversations.begin() + indexOf(uri));
    }

    sortConversations();
    emit linked.modelSorted();
}

void
ConversationModelPimpl::slotPendingContactAccepted(const std::string& uri)
{
    auto contactProfileId = database::getOrInsertProfile(db, uri);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conv.empty()) {
        conv.emplace_back(
            database::beginConversationsBetween(db, accountProfileId,
                contactProfileId, QObject::tr("Invitation accepted").toStdString()
            )
        );
    } else {
        try {
            auto contact = linked.owner.contactModel->getContact(uri);
            auto msg = interaction::Info {accountProfileId,
                                          QObject::tr("Invitation accepted").toStdString(),
                                          std::time(nullptr), interaction::Type::CONTACT,
                                          interaction::Status::SUCCEED};
            auto msgId = database::addMessageToConversation(db, accountProfileId, conv[0], msg);
            auto convIdx = indexOf(conv[0]);
            {
                std::lock_guard<std::mutex> lk(interactionsLocks[conversations[convIdx].uid]);
                conversations[convIdx].interactions.emplace(msgId, msg);
            }
            dirtyConversations = {true, true};
            emit linked.newInteraction(conv[0], msgId, msg);
        } catch (std::out_of_range& e) {
            qDebug() << "ConversationModelPimpl::slotContactAdded can't find contact";
        }
    }
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
    dirtyConversations = {true, true};
    emit linked.conversationRemoved(conversationUid);
    emit linked.modelSorted();
}

void
ConversationModelPimpl::slotContactModelUpdated(const std::string& uri)
{
    // We don't create newConversationItem if we already filter on pending
    conversation::Info newConversationItem;
    if (!filter.empty()) {
        // Create a conversation with the temporary item
        conversation::Info conversationInfo;
        auto& temporaryContact = linked.owner.contactModel->getContact("");
        conversationInfo.uid = temporaryContact.profileInfo.uri;
        conversationInfo.participants.emplace_back("");
        conversationInfo.accountId = linked.owner.id;

        // if temporary contact is already present, its alias is not empty (namely "Searching ..."),
        // or its registeredName is set because it was found on the nameservice.
        if (not temporaryContact.profileInfo.alias.empty() || not temporaryContact.registeredName.empty()) {
            if (!conversations.empty()) {
                auto firstContactUri = conversations.front().participants.front();
                //if first conversation has uri it is already a contact
                // then we must add temporary item
                if (not firstContactUri.empty()) {
                    conversations.emplace_front(conversationInfo);
                } else if (not conversationInfo.uid.empty()) {
                    // If firstContactUri is empty it means that we have to update
                    // this element as it is the temporary.
                    // Only when we have found an uri.
                    conversations.front() = conversationInfo;
                }
            } else {
                // no conversation, add temporaryItem
                conversations.emplace_front(conversationInfo);
            }
            dirtyConversations = {true, true};
            emit linked.modelSorted();
            return;
        }
    } else {
        // No filter, so we can remove the newConversationItem
        if (!conversations.empty()) {
            auto firstContactUri = conversations.front().participants.front();

            if (firstContactUri.empty()) {
                conversations.pop_front();
                dirtyConversations = {true, true};
                emit linked.modelSorted();
                return;
            }
        }
    }
    // trigger dirtyConversation in all cases to flush emptied temporary element due to filtered contact present in list
    // TL:DR : avoid duplicates and empty elements
    dirtyConversations = {true, true};
    int index = indexOfContact(uri);
    if (index != -1) {
        if (!conversations.empty() && conversations.front().participants.front().empty()) {
            // In this case, contact is present in list, so temporary item does not longer exists
            emit linked.modelSorted();
        } else {
            // In this case, a presence is updated
            emit linked.conversationUpdated(conversations.at(index).uid);
        }
    }
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
    std::vector<std::function<void(void)>> slotLambdas;
    {
        std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
        for (auto& interaction: conversation.interactions) {
            if (interaction.second.status == interaction::Status::SENDING) {
                // Get the message status from daemon, else unknown
                auto id = database::getDaemonIdByInteractionId(db, std::to_string(interaction.first));
                int status = 0;
                if (!id.empty()) {
                    auto msgId = std::stoull(id);
                    status = ConfigurationManager::instance().getMessageStatus(msgId);
                }
                slotLambdas.emplace_back([=]() -> void {
                    slotUpdateInteractionStatus(linked.owner.id, std::stoull(id), contactUri, status);
                });
            }
        }
    }
    for (const auto& l: slotLambdas) { l(); }

    conversation.unreadMessages = getNumberOfUnreadMessagesFor(convId);
    conversations.emplace_back(conversation);
    dirtyConversations = {true, true};
}

int
ConversationModelPimpl::indexOf(const std::string& uid) const
{
    for (unsigned int i = 0; i < conversations.size(); ++i) {
        if (conversations.at(i).uid == uid) return i;
    }
    return -1;
}

int
ConversationModelPimpl::indexOfContact(const std::string& uri) const
{
    for (unsigned int i = 0; i < conversations.size(); ++i) {
        if (conversations.at(i).participants.front() == uri) return i;
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
    dirtyConversations = {true, true};
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
    try {
        auto call = linked.owner.callModel->getCall(callId);
        if (call.isOutgoing)
            addOrUpdateCallMessage(callId, QObject::tr("📞 Outgoing call").toStdString());
        else
            addOrUpdateCallMessage(callId, QObject::tr("📞 Incoming call").toStdString());
    } catch (std::out_of_range& e) {
        qDebug() << "ConversationModelPimpl::slotCallStarted can't start inexistant call";
    }
}

void
ConversationModelPimpl::slotCallEnded(const std::string& callId)
{
    try {
        auto call = linked.owner.callModel->getCall(callId);
        if (call.startTime.time_since_epoch().count() != 0) {
            if (call.isOutgoing)
                addOrUpdateCallMessage(callId, QObject::tr("📞 Outgoing call - ").toStdString()
                    + linked.owner.callModel->getFormattedCallDuration(callId));
            else
                addOrUpdateCallMessage(callId, QObject::tr("📞 Incoming call - ").toStdString()
                    + linked.owner.callModel->getFormattedCallDuration(callId));
        } else {
            if (call.isOutgoing)
                addOrUpdateCallMessage(callId, QObject::tr("🕽 Missed outgoing call").toStdString());
            else
                addOrUpdateCallMessage(callId, QObject::tr("🕽 Missed incoming call").toStdString());
        }

        // reset the callId stored in the conversation
        for (auto& conversation: conversations)
            if (conversation.callId == callId) {
                try {
                    conversation.callId = linked.owner.callModel->getCallFromURI(conversation.participants[0], true).id;
                } catch (std::out_of_range& e) {
                    conversation.callId = "";
                }
                dirtyConversations = {true, true};
                linked.selectConversation(conversation.uid);
            }
    } catch (std::out_of_range& e) {
        qDebug() << "ConversationModelPimpl::slotCallEnded can't end inexistant call";
    }
}

void
ConversationModelPimpl::addOrUpdateCallMessage(const std::string& callId, const std::string& body)
{
    // Get conversation
    for (auto& conversation: conversations) {
        if (conversation.callId == callId) {
            auto uid = conversation.uid;
            auto msg = interaction::Info {accountProfileId, body, std::time(nullptr),
                                         interaction::Type::CALL, interaction::Status::SUCCEED};
            int msgId = database::addOrUpdateMessage(db, accountProfileId, conversation.uid, msg, callId);
            auto newInteraction = conversation.interactions.find(msgId) == conversation.interactions.end();
            if (newInteraction) {
                conversation.lastMessageUid = msgId;
                std::lock_guard<std::mutex> lk(interactionsLocks[conversation.uid]);
                conversation.interactions.emplace(msgId, msg);
            } else {
                std::lock_guard<std::mutex> lk(interactionsLocks[conversation.uid]);
                conversation.interactions[msgId] = msg;
            }
            dirtyConversations = {true, true};
            if (newInteraction)
                emit linked.newInteraction(conversation.uid, msgId, msg);
            else
                emit linked.interactionStatusUpdated(conversation.uid, msgId, msg);
            sortConversations();
            emit linked.modelSorted();
        }
    }
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

    auto& call = linked.owner.callModel->getCall(callId);
    if (call.type == call::Type::CONFERENCE) {
        // Show messages in all conversations for conferences.
        for (const auto& conversation: conversations) {
            if (conversation.confId == callId) {
                if (conversation.participants.empty()) continue;
                auto authorProfileId = database::getOrInsertProfile(db, from);
                addIncomingMessage(conversation.participants.front(), body, authorProfileId);
            }
        }
    } else {
        addIncomingMessage(from, body);
    }

}

void
ConversationModelPimpl::addIncomingMessage(const std::string& from,
                                           const std::string& body,
                                           const std::string& authorProfileId)
{
    auto contactProfileId = database::getOrInsertProfile(db, from);
    auto accountProfileId = database::getProfileId(db, linked.owner.profileInfo.uri);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conv.empty()) {
        conv.emplace_back(database::beginConversationsBetween(
            db, accountProfileId, contactProfileId,
            QObject::tr("Invitation received").toStdString()
        ));
    }
    auto authorId = authorProfileId.empty()? contactProfileId: authorProfileId;
    auto msg = interaction::Info {authorId, body, std::time(nullptr),
                                  interaction::Type::TEXT, interaction::Status::UNREAD};
    int msgId = database::addMessageToConversation(db, accountProfileId, conv[0], msg);
    auto conversationIdx = indexOf(conv[0]);
    // Add the conversation if not already here
    if (conversationIdx == -1) {
        addConversationWith(conv[0], from);
        emit linked.newConversation(conv[0]);
    } else {
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            conversations[conversationIdx].interactions.emplace(msgId, msg);
        }
        conversations[conversationIdx].lastMessageUid = msgId;
    }
    dirtyConversations = {true, true};
    emit linked.newInteraction(conv[0], msgId, msg);
    sortConversations();
    emit linked.modelSorted();
}

void
ConversationModelPimpl::slotCallAddedToConference(const std::string& callId, const std::string& confId)
{
    for (auto& conversation: conversations) {
        if (conversation.callId == callId) {
            conversation.confId = confId;
            dirtyConversations = {true, true};
            emit linked.selectConversation(conversation.uid);
        }
    }
}

void
ConversationModelPimpl::slotUpdateInteractionStatus(const std::string& accountId,
                                                    const uint64_t id,
                                                    const std::string& to,
                                                    int status)
{
    if (accountId != linked.owner.id) return;
    auto newStatus = interaction::Status::INVALID;
    switch (static_cast<DRing::Account::MessageStates>(status))
    {
    case DRing::Account::MessageStates::SENDING:
        newStatus = interaction::Status::SENDING;
        break;
    case DRing::Account::MessageStates::SENT:
        newStatus = interaction::Status::SUCCEED;
        break;
    case DRing::Account::MessageStates::FAILURE:
        newStatus = interaction::Status::FAILED;
        break;
    case DRing::Account::MessageStates::READ:
        newStatus = interaction::Status::READ;
        break;
    case DRing::Account::MessageStates::UNKNOWN:
    default:
        newStatus = interaction::Status::UNKNOWN;
        break;
    }
    // Update database
    auto interactionId = database::getInteractionIdByDaemonId(db, std::to_string(id));
    if (interactionId.empty()) return;
    auto msgId = std::stoull(interactionId);
    database::updateInteractionStatus(db, msgId, newStatus);
    // Update conversations
    auto contactProfileId = database::getProfileId(db, to);
    auto accountProfileId = database::getProfileId(db, linked.owner.profileInfo.uri);
    auto conv = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (!conv.empty()) {
        auto conversationIdx = indexOf(conv[0]);
        interaction::Info itCopy;
        bool emitUpdated = false;
        if (conversationIdx != -1) {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(msgId);
            if (it != interactions.end()) {
                it->second.status = newStatus;
                emitUpdated = true;
                itCopy = it->second;
            }
        }
        if (emitUpdated)
            emit linked.interactionStatusUpdated(conv[0], msgId, itCopy);
    }
}

void
ConversationModelPimpl::slotConferenceRemoved(const std::string& confId)
{
    // Get conversation
    for(auto& i : conversations){
        if (i.confId == confId)
            i.confId = "";
    }
}

int
ConversationModelPimpl::getNumberOfUnreadMessagesFor(const std::string& uid)
{
    return database::countUnreadFromInteractions(db, uid);
}


void
ConversationModel::sendFile(const std::string& convUid,
                            const std::string& path,
                            const std::string& filename)
{
    auto conversationIdx = pimpl_->indexOf(convUid);
    if (conversationIdx == -1)
        return;

    const auto& peerUri = pimpl_->conversations[conversationIdx].participants.front();
    if (peerUri.empty())
        return;

    pimpl_->sendContactRequest(peerUri);
    pimpl_->lrc.getDataTransferModel().sendFile(owner.id.c_str(),
                                                peerUri.c_str(),
                                                path.c_str(),
                                                filename.c_str());
}

void
ConversationModel::acceptTransfer(const std::string& convUid, uint64_t interactionId, const std::string& path)
{
    pimpl_->lrc.getDataTransferModel().accept(interactionId, path, 0);
    database::updateInteractionBody(pimpl_->db, interactionId, path);
    database::updateInteractionStatus(pimpl_->db, interactionId, interaction::Status::TRANSFER_ACCEPTED);

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = pimpl_->indexOf(convUid);
    interaction::Info itCopy;
    bool emitUpdated = false;
    if (conversationIdx != -1) {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convUid]);
        auto& interactions = pimpl_->conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            it->second.body = path;
            it->second.status = interaction::Status::TRANSFER_ACCEPTED;
            pimpl_->sendContactRequest(pimpl_->conversations[conversationIdx].participants.front());
            pimpl_->dirtyConversations = {true, true};
            emitUpdated = true;
            itCopy = it->second;
        }
    }
    if (emitUpdated)
        emit interactionStatusUpdated(convUid, interactionId, itCopy);
}

void
ConversationModel::cancelTransfer(const std::string& convUid, uint64_t interactionId)
{
    // For this action, we change interaction status before effective canceling as daemon will
    // emit Finished event code immediatly (before leaving this method) in non-DBus mode.
    auto conversationIdx = pimpl_->indexOf(convUid);
    if (conversationIdx != -1) {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convUid]);
        auto& interactions = pimpl_->conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            it->second.status = interaction::Status::TRANSFER_CANCELED;

            // update information in the db
            database::updateInteractionStatus(pimpl_->db, interactionId, interaction::Status::TRANSFER_CANCELED);

            // Forward cancel action to daemon
            pimpl_->lrc.getDataTransferModel().cancel(interactionId);
            pimpl_->dirtyConversations = {true, true};
            emit interactionStatusUpdated(convUid, interactionId, it->second);
        }
    }
}

void
ConversationModel::getTransferInfo(uint64_t interactionId, datatransfer::Info& info)
{
    try {
        auto dringId = pimpl_->lrc.getDataTransferModel().getDringIdFromInteractionId(interactionId);
        pimpl_->lrc.getDataTransferModel().transferInfo(dringId, info);
    } catch (...) {
        info.status = datatransfer::Status::INVALID;
    }
}

int
ConversationModel::getNumberOfUnreadMessagesFor(const std::string& convUid)
{
    return pimpl_->getNumberOfUnreadMessagesFor(convUid);
}

bool
ConversationModelPimpl::usefulDataFromDataTransfer(long long dringId, const datatransfer::Info& info,
                                                   int& interactionId, std::string& convId)
{
    try {
        interactionId = lrc.getDataTransferModel().getInteractionIdFromDringId(dringId);
    } catch (...) {
        return false;
    }

    convId = database::conversationIdFromInteractionId(db, interactionId);
    return true;
}

void
ConversationModelPimpl::slotTransferStatusCreated(long long dringId, datatransfer::Info info)
{
    // check if transfer is for the current account
    if (info.accountId != linked.owner.id) return;

    const auto* account = AccountModel::instance().getById(info.accountId.c_str());
    if (not account)
        return;

    auto accountProfileId = database::getProfileId(db, linked.owner.profileInfo.uri);
    auto contactProfileId = database::getProfileId(db, info.peerUri);

    // create a new conversation if needed
    auto conversation_list = database::getConversationsBetween(db, accountProfileId, contactProfileId);
    if (conversation_list.empty()) {
        conversation_list.emplace_back(database::beginConversationsBetween(
                                           db, accountProfileId, contactProfileId,
                                           QObject::tr("Invitation received").toStdString()));
    }

    // add interaction to the db
    const auto& convId = conversation_list[0];
    auto interactionId = database::addDataTransferToConversation(db, accountProfileId, convId, info);

    // map dringId and interactionId for latter retrivial from client (that only known the interactionId)
    lrc.getDataTransferModel().registerTransferId(dringId, interactionId);

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convId);
    if (conversationIdx == -1) {
        addConversationWith(conversation_list[0], info.peerUri);
        emit linked.newConversation(convId);
    } else {
        auto& interactions = conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end())
            return;

        auto interactioType = info.isOutgoing ?
            interaction::Type::OUTGOING_DATA_TRANSFER :
            interaction::Type::INCOMING_DATA_TRANSFER;
        auto interaction = interaction::Info {info.isOutgoing? accountProfileId : contactProfileId,
                                              info.isOutgoing? info.path : info.displayName,
                                              std::time(nullptr),
                                              interactioType,
                                              interaction::Status::TRANSFER_CREATED};
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            interactions.emplace(interactionId, interaction);
        }
        conversations[conversationIdx].lastMessageUid = interactionId;
        dirtyConversations = {true, true};
        emit linked.newInteraction(convId, interactionId, interaction);
    }
    sortConversations();
    emit linked.modelSorted();
}

void
ConversationModelPimpl::slotTransferStatusAwaitingPeer(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    auto newStatus = interaction::Status::TRANSFER_AWAITING_PEER;
    database::updateInteractionStatus(db, interactionId, newStatus);

    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusAwaitingHost(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    auto newStatus = interaction::Status::TRANSFER_AWAITING_HOST;
    database::updateInteractionStatus(db, interactionId, newStatus);

    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusOngoing(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    auto newStatus = interaction::Status::TRANSFER_ONGOING;
    database::updateInteractionStatus(db, interactionId, newStatus);

    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            auto* timer = new QTimer();
            connect(timer, &QTimer::timeout,
                    [=] { updateTransfer(timer, convId, conversationIdx, interactionId); });
            timer->start(1000);
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusFinished(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        auto newStatus = interaction::Status::TRANSFER_FINISHED;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                // We need to check if current status is ONGOING as CANCELED must not be transformed into FINISHED
                if (it->second.status == interaction::Status::TRANSFER_ONGOING) {
                    emitUpdated = true;
                    it->second.status = newStatus;
                    itCopy = it->second;
                }
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            database::updateInteractionStatus(db, interactionId, newStatus);
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusCanceled(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    auto newStatus = interaction::Status::TRANSFER_CANCELED;
    database::updateInteractionStatus(db, interactionId, newStatus);

    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusError(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    // update information in the db
    auto newStatus = interaction::Status::TRANSFER_ERROR;
    database::updateInteractionStatus(db, interactionId, newStatus);

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusUnjoinable(long long dringId, datatransfer::Info info)
{
    int interactionId;
    std::string convId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, convId))
        return;

    // update information in the db
    auto newStatus = interaction::Status::TRANSFER_UNJOINABLE_PEER;
    database::updateInteractionStatus(db, interactionId, newStatus);

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                emitUpdated = true;
                it->second.status = newStatus;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            dirtyConversations = {true, true};
            emit linked.interactionStatusUpdated(convId, interactionId, itCopy);
        }
    }
}

void
ConversationModelPimpl::updateTransfer(QTimer* timer, const std::string& conversation,
                                       int conversationIdx, int interactionId)
{
    try {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            const auto& interactions = conversations[conversationIdx].interactions;
            const auto& it = interactions.find(interactionId);
            if (it != std::cend(interactions)
                and it->second.status == interaction::Status::TRANSFER_ONGOING) {
                emitUpdated = true;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            emit linked.interactionStatusUpdated(conversation, interactionId, itCopy);
            return;
        }
    } catch (...) {}

    timer->stop();
    delete timer;
}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

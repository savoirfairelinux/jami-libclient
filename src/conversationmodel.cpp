/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
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
#include "containerview.h"
#include "authority/storagehelper.h"
#include "uri.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"

// daemon
#include <account_const.h>
#include <datatransfer_interface.h>

// Qt
#include <QtCore/QTimer>
#include <QFileInfo>

// std
#include <algorithm>
#include <mutex>
#include <regex>
#include <fstream>
#include <sstream>

namespace lrc {

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

    using FilterPredicate = std::function<bool(const conversation::Info& convInfo)>;

    /**
     * return a conversation index from conversations or -1 if no index is found.
     * @param uid of the contact to search.
     * @return an int.
     */
    int indexOf(const QString& uid) const;

    /**
     * return a reference to a conversation with given filter
     * @param pred a unary comparison predicate with which to find the conversation
     * @param searchResultIncluded if need to search in contacts and userSearch
     * @return a reference to a conversation with given uid
     */
    std::reference_wrapper<conversation::Info> getConversation(
        const FilterPredicate& pred, const bool searchResultIncluded = false);

    /**
     * return a reference to a conversation with given uid.
     * @param conversation uid.
     * @param searchResultIncluded if need to search in contacts and userSearch.
     * @return a reference to a conversation with the given uid.
     */
    std::reference_wrapper<conversation::Info> getConversationForUid(
        const QString& uid, const bool searchResultIncluded = false);

    /**
     * return a reference to a conversation with participant.
     * @param participant uri.
     * @param searchResultIncluded if need to search in contacts and userSearch.
     * @return a reference to a conversation with the given peer uri.
     */
    std::reference_wrapper<conversation::Info> getConversationForPeerUri(
        const QString& uri, const bool searchResultIncluded = false);

    /**
     * return a conversation index from conversations or -1 if no index is found.
     * @param uri of the contact to search.
     * @return an int.
     */
    int indexOfContact(const QString& uri) const;

    /**
     * Initialize conversations_ and filteredConversations_
     */
    void initConversations();
    /**
     * Filter all conversations
     */
    bool filter(const conversation::Info& conv);
    /**
     * Sort conversation by last action
     */
    bool sort(const conversation::Info& convA, const conversation::Info& convB);
    /**
     * Call contactModel.addContact if necessary
     * @param contactUri
     */
    void sendContactRequest(const QString& contactUri);
    /**
     * Add a conversation with contactUri
     * @param convId
     * @param contactUri
     */
    void addConversationWith(const QString& convId, const QString& contactUri);
    /**
     * Add a swarm conversation to conversation list
     * @param convId
     */
    void addSwarmConversation(const QString& convI);
    /**
     * Add call interaction for conversation with callId
     * @param callId
     * @param duration
     */
    void addOrUpdateCallMessage(const QString& callId,
                                const QString& from,
                                bool incoming,
                                const std::time_t& duration = -1);
    /**
     * Add a new message from a peer in the database
     * @param from          the author uri
     * @param body          the content of the message
     * @param timestamp     the timestamp of the message
     * @param daemonId      the daemon id
     * @return msgId generated (in db)
     */
    QString addIncomingMessage(const QString& from,
                               const QString& body,
                               const uint64_t& timestamp = 0,
                               const QString& daemonId = "");
    /**
     * Change the status of an interaction. Listen from callbacksHandler
     * @param accountId, account linked
     * @param messageId, interaction to update
     * @param conversationId, conversation
     * @param peer, peer uri
     * @param status, new status for this interaction
     */
    void slotUpdateInteractionStatus(const QString& accountId,
                                     const QString& conversationId,
                                     const QString& peer,
                                     const QString& messageId,
                                     int status);

    /**
     * place a call
     * @param uid, conversation id
     * @param isAudioOnly, allow to specify if the call is only audio. Set to false by default.
     */
    void placeCall(const QString& uid, bool isAudioOnly = false);

    /**
     * get number of unread messages
     */
    int getNumberOfUnreadMessagesFor(const QString& uid);

    /**
     * Handle data transfer progression
     */
    void updateTransfer(QTimer* timer,
                        const QString& conversation,
                        int conversationIdx,
                        const QString& interactionId);

    bool usefulDataFromDataTransfer(DataTransferId dringId,
                                    const datatransfer::Info& info,
                                    QString& interactionId,
                                    QString& conversationId);
    void awaitingHost(DataTransferId dringId, datatransfer::Info info);

    bool hasOneOneSwarmWith(const QString& participant);

    /**
     * accept a file transfer
     * @param convUid
     * @param interactionId
     * @param final name of the file
     */
    void acceptTransfer(const QString& convUid, const QString& interactionId, const QString& path);
    void addConversationRequest(const MapStringString& convRequest);
    void addContactRequest(const QString& contactUri);
    void removePendingConversation(const QString& contactUri);
    void insertSwarmInteraction(const QString& interactionId,
                                const interaction::Info& interaction,
                                conversation::Info& conversation,
                                bool insertAtBegin);

    void invalidateModel();

    const ConversationModel& linked;
    Lrc& lrc;
    Database& db;
    const CallbacksHandler& callbacksHandler;
    const BehaviorController& behaviorController;

    ConversationModel::ConversationQueue conversations; ///< non-filtered conversations
    ConversationModel::ConversationQueue searchResults;

    ConversationModel::ConversationQueueProxy filteredConversations;
    ConversationModel::ConversationQueueProxy customFilteredConversations;

    QString currentFilter;
    FilterType typeFilter;
    FilterType customTypeFilter;

    std::map<QString, std::mutex> interactionsLocks; ///< {convId, mutex}
    MapStringString transfIdToDbIntId;

public Q_SLOTS:
    /**
     * Listen from contactModel when updated (like new alias, avatar, etc.)
     */
    void slotContactModelUpdated(const QString& uri, bool needsSorted);
    /**
     * Listen from contactModel when a new contact is added
     * @param uri
     */
    void slotContactAdded(const QString& contactUri);
    /**
     * Listen from contactModel when receive a new contact request
     * @param uri
     */
    void slotIncomingContactRequest(const QString& contactUri);
    /**
     * Listen from contactModel when a pending contact is accepted
     * @param uri
     */
    void slotPendingContactAccepted(const QString& uri);
    /**
     * Listen from contactModel when aa new contact is removed
     * @param uri
     */
    void slotContactRemoved(const QString& uri);
    /**
     * Listen from callmodel for new calls.
     * @param fromId caller uri
     * @param callId
     */
    void slotIncomingCall(const QString& fromId, const QString& callId);
    /**
     * Listen from callmodel for calls status changed.
     * @param callId
     */
    void slotCallStatusChanged(const QString& callId, int code);
    /**
     * Listen from callmodel for writing "Call started"
     * @param callId
     */
    void slotCallStarted(const QString& callId);
    /**
     * Listen from callmodel for writing "Call ended"
     * @param callId
     */
    void slotCallEnded(const QString& callId);
    /**
     * Listen from CallbacksHandler for new incoming interactions;
     * @param accountId
     * @param msgId
     * @param from uri
     * @param payloads body
     */
    void slotNewAccountMessage(const QString& accountId,
                               const QString& from,
                               const QString& msgId,
                               const MapStringString& payloads);
    /**
     * Listen from CallbacksHandler for new messages in a SIP call
     * @param callId call linked to the interaction
     * @param from author uri
     * @param body of the message
     */
    void slotIncomingCallMessage(const QString& callId, const QString& from, const QString& body);
    /**
     * Listen from CallModel when a call is added to a conference
     * @param callId
     * @param confId
     */
    void slotCallAddedToConference(const QString& callId, const QString& confId);
    /**
     * Listen from CallbacksHandler when a conference is deleted.
     * @param confId
     */
    void slotConferenceRemoved(const QString& confId);
    /**
     * Listen for when a contact is composing
     * @param accountId
     * @param contactUri
     * @param isComposing
     */
    void slotComposingStatusChanged(const QString& accountId,
                                    const QString& convId,
                                    const QString& contactUri,
                                    bool isComposing);

    void slotTransferStatusCreated(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusCanceled(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusAwaitingPeer(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusAwaitingHost(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusOngoing(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusFinished(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusError(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusTimeoutExpired(DataTransferId dringId, api::datatransfer::Info info);
    void slotTransferStatusUnjoinable(DataTransferId dringId, api::datatransfer::Info info);
    bool updateTransferStatus(DataTransferId dringId,
                              datatransfer::Info info,
                              interaction::Status newStatus,
                              bool& updated);
    void slotConversationLoaded(uint32_t loadingRequestId,
                                const QString& accountId,
                                const QString& conversationId,
                                VectorMapStringString messages);
    void slotMessageReceived(const QString& accountId,
                             const QString& conversationId,
                             MapStringString message);
    void slotConversationRequestReceived(const QString& accountId,
                                         const QString& conversationId,
                                         MapStringString metadatas);
    void slotConversationMemberEvent(const QString& accountId,
                                     const QString& conversationId,
                                     const QString& memberUri,
                                     int event);
    void slotConversationReady(const QString& accountId, const QString& conversationId);
    void slotConversationRemoved(const QString& accountId, const QString& conversationId);
};

ConversationModel::ConversationModel(const account::Info& owner,
                                     Lrc& lrc,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler,
                                     const BehaviorController& behaviorController)
    : QObject(nullptr)
    , pimpl_(std::make_unique<ConversationModelPimpl>(*this,
                                                      lrc,
                                                      db,
                                                      callbacksHandler,
                                                      behaviorController))
    , owner(owner)
{}

ConversationModel::~ConversationModel() {}

const ConversationModel::ConversationQueueProxy&
ConversationModel::allFilteredConversations() const
{
    if (!pimpl_->filteredConversations.isDirty())
        return pimpl_->filteredConversations;

    return pimpl_->filteredConversations.filter().sort().validate();
}

QMap<ConferenceableItem, ConferenceableValue>
ConversationModel::getConferenceableConversations(const QString& convId, const QString& filter) const
{
    auto conversationIdx = pimpl_->indexOf(convId);
    if (conversationIdx == -1 || !owner.enabled) {
        return {};
    }
    QMap<ConferenceableItem, ConferenceableValue> result;
    ConferenceableValue callsVector, contactsVector;

    auto currentConfId = pimpl_->conversations.at(conversationIdx).confId;
    auto currentCallId = pimpl_->conversations.at(conversationIdx).callId;
    auto calls = pimpl_->lrc.getCalls();
    auto conferences = pimpl_->lrc.getConferences();
    auto& conversations = pimpl_->conversations;
    auto currentAccountID = pimpl_->linked.owner.id;
    // add contacts for current account
    for (const auto& conv : conversations) {
        // conversations with calls will be added in call section
        if (!conv.callId.isEmpty() || !conv.confId.isEmpty()) {
            continue;
        }
        try {
            auto contact = owner.contactModel->getContact(conv.participants.front());
            if (contact.isBanned || contact.profileInfo.type == profile::Type::PENDING) {
                continue;
            }
            QVector<AccountConversation> cv;
            AccountConversation accConv = {conv.uid, currentAccountID};
            cv.push_back(accConv);
            if (filter.isEmpty()) {
                contactsVector.push_back(cv);
                continue;
            }
            bool result = contact.profileInfo.alias.contains(filter)
                          || contact.profileInfo.uri.contains(filter)
                          || contact.registeredName.contains(filter);
            if (result) {
                contactsVector.push_back(cv);
            }
        } catch (const std::out_of_range& e) {
            qDebug() << e.what();
            continue;
        }
    }

    if (calls.empty() && conferences.empty()) {
        result.insert(ConferenceableItem::CONTACT, contactsVector);
        return result;
    }

    // filter out calls from conference
    for (const auto& c : conferences) {
        for (const auto& subcal : pimpl_->lrc.getConferenceSubcalls(c)) {
            auto position = std::find(calls.begin(), calls.end(), subcal);
            if (position != calls.end()) {
                calls.erase(position);
            }
        }
    }

    // found conversations and account for calls and conferences
    QMap<QString, QVector<AccountConversation>> tempConferences;
    for (const auto& account_id : pimpl_->lrc.getAccountModel().getAccountList()) {
        try {
            auto& accountInfo = pimpl_->lrc.getAccountModel().getAccountInfo(account_id);
            auto type = accountInfo.profileInfo.type == profile::Type::SIP ? FilterType::SIP
                                                                           : FilterType::JAMI;
            auto accountConv = accountInfo.conversationModel->getFilteredConversations(type);
            accountConv.for_each([filter,
                                  &accountInfo,
                                  account_id,
                                  currentCallId,
                                  currentConfId,
                                  &conferences,
                                  &calls,
                                  &tempConferences,
                                  &callsVector](const conversation::Info& conv) {
                bool confFilterPredicate = !conv.confId.isEmpty() && conv.confId != currentConfId
                                           && std::find(conferences.begin(),
                                                        conferences.end(),
                                                        conv.confId)
                                                  != conferences.end();
                bool callFilterPredicate = !conv.callId.isEmpty() && conv.callId != currentCallId
                                           && std::find(calls.begin(), calls.end(), conv.callId)
                                                  != calls.end();

                if (!confFilterPredicate && !callFilterPredicate) {
                    return;
                }

                // vector of conversationID accountID pair
                // for call has only one entry, for conference multyple
                QVector<AccountConversation> cv;
                AccountConversation accConv = {conv.uid, account_id};
                cv.push_back(accConv);

                bool isConference = !conv.confId.isEmpty();
                // call could be added if it is not conference and in active state
                bool shouldAddCall = false;
                if (!isConference && accountInfo.callModel->hasCall(conv.callId)) {
                    const auto& call = accountInfo.callModel->getCall(conv.callId);
                    shouldAddCall = call.status == lrc::api::call::Status::PAUSED
                                    || call.status == lrc::api::call::Status::IN_PROGRESS;
                }

                auto contact = accountInfo.contactModel->getContact(conv.participants.front());
                // check if contact satisfy filter
                bool result = (filter.isEmpty() || isConference)
                                  ? true
                                  : (contact.profileInfo.alias.contains(filter)
                                     || contact.profileInfo.uri.contains(filter)
                                     || contact.registeredName.contains(filter));
                if (!result) {
                    return;
                }
                if (isConference && tempConferences.count(conv.confId)) {
                    tempConferences.find(conv.confId).value().push_back(accConv);
                } else if (isConference) {
                    tempConferences.insert(conv.confId, cv);
                } else if (shouldAddCall) {
                    callsVector.push_back(cv);
                }
            });
        } catch (...) {
        }
    }
    for (auto it : tempConferences.toStdMap()) {
        if (filter.isEmpty()) {
            callsVector.push_back(it.second);
            continue;
        }
        for (AccountConversation accConv : it.second) {
            try {
                auto& account = pimpl_->lrc.getAccountModel().getAccountInfo(accConv.accountId);
                auto& conv = account.conversationModel->getConversationForUid(accConv.convId)->get();
                auto cont = account.contactModel->getContact(conv.participants.front());
                if (cont.profileInfo.alias.contains(filter) || cont.profileInfo.uri.contains(filter)
                    || cont.registeredName.contains(filter)) {
                    callsVector.push_back(it.second);
                    continue;
                }
            } catch (...) {
            }
        }
    }
    result.insert(ConferenceableItem::CALL, callsVector);
    result.insert(ConferenceableItem::CONTACT, contactsVector);
    return result;
}

const ConversationModel::ConversationQueue&
ConversationModel::getAllSearchResults() const
{
    return pimpl_->searchResults;
}

const ConversationModel::ConversationQueueProxy&
ConversationModel::getFilteredConversations(const FilterType& filter,
                                            bool forceUpdate,
                                            const bool includeBanned) const
{
    if (pimpl_->customTypeFilter == filter && !pimpl_->customFilteredConversations.isDirty()
        && !forceUpdate)
        return pimpl_->customFilteredConversations;

    pimpl_->customTypeFilter = filter;
    return pimpl_->customFilteredConversations.reset(pimpl_->conversations)
        .filter([this, &includeBanned](const conversation::Info& entry) {
            try {
                auto contactInfo = owner.contactModel->getContact(entry.participants.front());
                if (!includeBanned && contactInfo.isBanned)
                    return false;
                switch (pimpl_->customTypeFilter) {
                case FilterType::JAMI:
                    return (contactInfo.profileInfo.type == profile::Type::JAMI && !entry.isRequest);
                case FilterType::SIP:
                    return (contactInfo.profileInfo.type == profile::Type::SIP && !entry.isRequest);
                case FilterType::REQUEST:
                    return entry.isRequest;
                default:
                    break;
                }
            } catch (...) {
                return false;
            }
        })
        .validate();
}

OptRef<conversation::Info>
ConversationModel::getConversationForUid(const QString& uid)
{
    try {
        return std::make_optional(pimpl_->getConversationForUid(uid, true));
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

OptRef<conversation::Info>
ConversationModel::getConversationForPeerUri(const QString& uri)
{
    try {
        return std::make_optional(
            pimpl_->getConversation([uri](const conversation::Info& conv)
                                        -> bool { return uri == conv.participants.front(); },
                                    true));
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

OptRef<conversation::Info>
ConversationModel::getConversationForCallId(const QString& callId)
{
    try {
        return std::make_optional(pimpl_->getConversation(
            [callId](const conversation::Info& conv) -> bool {
                return (callId == conv.callId || callId == conv.confId);
            },
            true));
    } catch (const std::out_of_range&) {
        return std::nullopt;
    }
}

OptRef<conversation::Info>
ConversationModel::filteredConversation(unsigned row) const
{
    auto conversations = allFilteredConversations();
    if (row >= conversations.get().size())
        return std::nullopt;

    return std::make_optional(conversations.get().at(row));
}

OptRef<conversation::Info>
ConversationModel::searchResultForRow(unsigned row) const
{
    auto& results = pimpl_->searchResults;
    if (row >= results.size())
        return std::nullopt;

    return std::make_optional(std::ref(results.at(row)));
}

void
ConversationModel::makePermanent(const QString& uid)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(uid, true).get();

        if (conversation.participants.empty()) {
            // Should not
            qDebug() << "ConversationModel::addConversation can't add a conversation with no "
                        "participant";
            return;
        }

        // Send contact request if non used
        pimpl_->sendContactRequest(conversation.participants.front());
    } catch (const std::out_of_range& e) {
        qDebug() << "make permanent failed. conversation not found";
    }
}

void
ConversationModel::selectConversation(const QString& uid) const
{
    try {
        auto& conversation = pimpl_->getConversationForUid(uid, true).get();

        bool callEnded = true;
        if (!conversation.callId.isEmpty()) {
            try {
                auto call = owner.callModel->getCall(conversation.callId);
                callEnded = call.status == call::Status::ENDED;
            } catch (...) {
            }
        }
        if (!conversation.confId.isEmpty() && owner.confProperties.isRendezVous) {
            // If we are on a rendez vous account and we select the conversation,
            // attach to the call.
            CallManager::instance().unholdConference(conversation.confId);
        }

        if (not callEnded and not conversation.confId.isEmpty()) {
            emit pimpl_->behaviorController.showCallView(owner.id, conversation.uid);
        } else if (callEnded) {
            emit pimpl_->behaviorController.showChatView(owner.id, conversation.uid);
        } else {
            try {
                auto call = owner.callModel->getCall(conversation.callId);
                switch (call.status) {
                case call::Status::INCOMING_RINGING:
                case call::Status::OUTGOING_RINGING:
                case call::Status::CONNECTING:
                case call::Status::SEARCHING:
                    // We are currently in a call
                    emit pimpl_->behaviorController.showIncomingCallView(owner.id, conversation.uid);
                    break;
                case call::Status::PAUSED:
                case call::Status::CONNECTED:
                case call::Status::IN_PROGRESS:
                    // We are currently receiving a call
                    emit pimpl_->behaviorController.showCallView(owner.id, conversation.uid);
                    break;
                case call::Status::PEER_BUSY:
                    emit pimpl_->behaviorController.showLeaveMessageView(owner.id, conversation.uid);
                    break;
                case call::Status::TIMEOUT:
                case call::Status::TERMINATING:
                case call::Status::INVALID:
                case call::Status::INACTIVE:
                    // call just ended
                    emit pimpl_->behaviorController.showChatView(owner.id, conversation.uid);
                    break;
                case call::Status::ENDED:
                default: // ENDED
                    // nothing to do
                    break;
                }
            } catch (const std::out_of_range&) {
                // Should not happen
                emit pimpl_->behaviorController.showChatView(owner.id, conversation.uid);
            }
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "select conversation failed. conversation not exists";
    }
}

void
ConversationModel::removeConversation(const QString& uid, bool banned)
{
    // Get conversation
    auto conversationIdx = pimpl_->indexOf(uid);
    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    if (conversation.participants.empty()) {
        // Should not
        qDebug() << "ConversationModel::removeConversation can't remove a conversation without "
                    "participant";
        return;
    }
    if (conversation.mode != conversation::Mode::NON_SWARM && conversation.participants.size() > 1) {
        ConfigurationManager::instance().removeConversation(owner.id, uid);
        pimpl_->conversations.erase(pimpl_->conversations.begin() + conversationIdx);
        pimpl_->invalidateModel();
        emit conversationRemoved(uid);
        return;
    }

    // Remove contact from daemon
    // NOTE: this will also remove the conversation into the database.
    for (const auto& participant : conversation.participants)
        owner.contactModel->removeContact(participant, banned);
}

void
ConversationModel::deleteObsoleteHistory(int days)
{
    if (days < 1)
        return; // unlimited history

    auto currentTime = static_cast<long int>(std::time(nullptr)); // since epoch, in seconds...
    auto date = currentTime - (days * 86400);

    storage::deleteObsoleteHistory(pimpl_->db, date);
}

void
ConversationModelPimpl::placeCall(const QString& uid, bool isAudioOnly)
{
    try {
        auto& conversation = getConversationForUid(uid, true).get();
        if (conversation.participants.empty()) {
            // Should not
            qDebug()
                << "ConversationModel::placeCall can't call a conversation without participant";
            return;
        }

        // Disallow multiple call
        if (!conversation.callId.isEmpty()) {
            try {
                auto call = linked.owner.callModel->getCall(conversation.callId);
                switch (call.status) {
                case call::Status::INCOMING_RINGING:
                case call::Status::OUTGOING_RINGING:
                case call::Status::CONNECTING:
                case call::Status::SEARCHING:
                case call::Status::PAUSED:
                case call::Status::IN_PROGRESS:
                case call::Status::CONNECTED:
                    return;
                case call::Status::INVALID:
                case call::Status::INACTIVE:
                case call::Status::ENDED:
                case call::Status::PEER_BUSY:
                case call::Status::TIMEOUT:
                case call::Status::TERMINATING:
                default:
                    break;
                }
            } catch (const std::out_of_range&) {
            }
        }

        auto convId = uid;

        auto participant = conversation.participants.front();
        bool isTemporary = participant == convId;
        auto contactInfo = linked.owner.contactModel->getContact(participant);
        auto uri = contactInfo.profileInfo.uri;

        if (uri.isEmpty())
            return; // Incorrect item

        // Don't call banned contact
        if (contactInfo.isBanned) {
            qDebug() << "ContactModel::placeCall: denied, contact is banned";
            return;
        }

        if (linked.owner.profileInfo.type != profile::Type::SIP) {
            uri = "ring:" + uri; // Add the ring: before or it will fail.
        }

        auto cb = std::function<void(QString, QString)>(
            [this, isTemporary, uri, isAudioOnly, &conversation](QString conversationId,
                                                                 QString participantURI) {
                if (indexOf(conversationId) < 0) {
                    qDebug() << "Can't place call: conversation  not exists";
                    return;
                }

                auto& newConv = isTemporary ? getConversationForUid(conversationId).get()
                                            : conversation;

                newConv.callId = linked.owner.callModel->createCall(uri, isAudioOnly);
                if (newConv.callId.isEmpty()) {
                    qDebug() << "Can't place call (daemon side failure ?)";
                    return;
                }

                invalidateModel();

                emit behaviorController.showIncomingCallView(linked.owner.id, newConv.uid);
            });

        if (isTemporary) {
            QMetaObject::Connection* const connection = new QMetaObject::Connection;
            *connection = connect(&this->linked,
                                  &ConversationModel::conversationReady,
                                  [cb, connection, convId](QString conversationId,
                                                           QString participantURI) {
                                      if (participantURI != convId) {
                                          return;
                                      }
                                      cb(conversationId, participantURI);
                                      QObject::disconnect(*connection);
                                      if (connection) {
                                          delete connection;
                                      }
                                  });
        }

        sendContactRequest(participant);

        if (!isTemporary) {
            cb(convId, participant);
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "could not place call to not existing conversation";
    }
}

void
ConversationModel::placeAudioOnlyCall(const QString& uid)
{
    pimpl_->placeCall(uid, true);
}

void
ConversationModel::placeCall(const QString& uid)
{
    pimpl_->placeCall(uid);
}

MapStringString
ConversationModel::getConversationInfos(const QString& conversationId)
{
    MapStringString ret = ConfigurationManager::instance().conversationInfos(owner.id,
                                                                             conversationId);
    return ret;
}

void
ConversationModel::createConversation(const VectorString& participants, const QString& title)
{
    auto convUid = ConfigurationManager::instance().startConversation(owner.id);
    for (auto participant : participants) {
        ConfigurationManager::instance().addConversationMember(owner.id, convUid, participant);
    }
    if (!title.isEmpty()) {
        MapStringString info = getConversationInfos(convUid);
        info["title"] = title;
        updateConversationInfo(convUid, info);
    }
    pimpl_->addSwarmConversation(convUid);
    emit newConversation(convUid);
    pimpl_->invalidateModel();
    emit modelChanged();
}
void
ConversationModel::updateConversationInfo(const QString& conversationId, const MapStringString info)
{
    ConfigurationManager::instance().updateConversationInfos(owner.id, conversationId, info);
}

bool
ConversationModel::hasPendingRequests() const
{
    return pendingRequestCount() > 0;
}

int
ConversationModel::pendingRequestCount() const
{
    int pendingRequestCount = 0;
    std::for_each(pimpl_->conversations.begin(),
                  pimpl_->conversations.end(),
                  [&pendingRequestCount](const auto& c) { pendingRequestCount += c.isRequest; });
    return pendingRequestCount;
}

void
ConversationModel::sendMessage(const QString& uid, const QString& body, const QString& parentId)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(uid, true).get();

        if (conversation.participants.empty()) {
            // Should not
            qDebug() << "ConversationModel::sendMessage can't send a interaction to a conversation "
                        "with no participant";
            return;
        }

        auto convId = uid;
        // for temporary contact conversation id is the same as participant uri
        bool isTemporary = conversation.participants.front() == uid;

        /* Make a copy of participants list: if current conversation is temporary,
         it might me destroyed while we are reading it */
        const auto participants = conversation.participants;
        auto cb = std::function<void(QString, QString)>([this,
                                                         isTemporary,
                                                         body,
                                                         &conversation,
                                                         parentId,
                                                         convId](QString conversationId,
                                                                 QString participantURI) {
            if (pimpl_->indexOf(conversationId) < 0) {
                return;
            }
            auto& newConv = isTemporary ? pimpl_->getConversationForUid(conversationId).get()
                                        : conversation;

            if (newConv.mode != conversation::Mode::NON_SWARM) {
                ConfigurationManager::instance().sendMessage(owner.id, conversationId, body, parentId);
                return;
            }

            uint64_t daemonMsgId = 0;
            auto status = interaction::Status::SENDING;
            auto convId = newConv.uid;

            // Send interaction to each participant
            for (const auto& participant : newConv.participants) {
                auto contactInfo = owner.contactModel->getContact(participant);

                QStringList callLists = CallManager::instance().getCallList(); // no auto
                // workaround: sometimes, it may happen that the daemon delete a call, but lrc
                // don't. We check if the call is
                //             still valid every time the user want to send a message.
                if (not newConv.callId.isEmpty() and not callLists.contains(newConv.callId))
                    newConv.callId.clear();

                if (not newConv.callId.isEmpty()
                    and call::canSendSIPMessage(owner.callModel->getCall(newConv.callId))) {
                    status = interaction::Status::UNKNOWN;
                    owner.callModel->sendSipMessage(newConv.callId, body);

                } else {
                    daemonMsgId = owner.contactModel->sendDhtMessage(contactInfo.profileInfo.uri,
                                                                     body);
                }
            }

            // Add interaction to database
            interaction::Info
                msg {{}, body, std::time(nullptr), 0, interaction::Type::TEXT, status, true};
            auto msgId = storage::addMessageToConversation(pimpl_->db, convId, msg);

            // Update conversation
            if (status == interaction::Status::SENDING) {
                // Because the daemon already give an id for the message, we need to store it.
                storage::addDaemonMsgId(pimpl_->db, msgId, toQString(daemonMsgId));
            }

            bool ret = false;

            {
                std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
                ret = newConv.interactions.insert(std::pair<QString, interaction::Info>(msgId, msg))
                          .second;
            }

            if (!ret) {
                qDebug()
                    << "ConversationModel::sendMessage failed to send message because an existing "
                       "key was already present in the database key ="
                    << msgId;
                return;
            }

            newConv.lastMessageUid = msgId;
            // Emit this signal for chatview in the client
            emit newInteraction(convId, msgId, msg);
            // This conversation is now at the top of the list
            // The order has changed, informs the client to redraw the list
            pimpl_->invalidateModel();
            emit modelChanged();
        });

        if (isTemporary) {
            QMetaObject::Connection* const connection = new QMetaObject::Connection;
            *connection = connect(this,
                                  &ConversationModel::conversationReady,
                                  [cb, connection, convId](QString conversationId,
                                                           QString participantURI) {
                                      if (participantURI != convId) {
                                          return;
                                      }
                                      cb(conversationId, participantURI);
                                      QObject::disconnect(*connection);
                                      if (connection) {
                                          delete connection;
                                      }
                                  });
        }

        /* Check participants list, send contact request if needed.
         NOTE: conferences are not implemented yet, so we have only one participant */
        for (const auto& participant : participants) {
            auto contactInfo = owner.contactModel->getContact(participant);

            if (contactInfo.isBanned) {
                qDebug() << "ContactModel::sendMessage: denied, contact is banned";
                return;
            }

            pimpl_->sendContactRequest(participant);
        }

        if (!isTemporary) {
            cb(convId, conversation.participants.front());
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "could not send message to not existing conversation";
    }
}

void
ConversationModel::refreshFilter()
{
    pimpl_->invalidateModel();
    emit filterChanged();
}

void
ConversationModel::updateSearchStatus(const QString& status) const
{
    emit searchStatusChanged(status);
}

void
ConversationModel::setFilter(const QString& filter)
{
    pimpl_->currentFilter = filter;
    pimpl_->invalidateModel();
    pimpl_->searchResults.clear();
    emit searchResultUpdated();
    owner.contactModel->searchContact(filter);
    emit filterChanged();
}

void
ConversationModel::setFilter(const FilterType& filter)
{
    // Switch between PENDING, RING and SIP contacts.
    pimpl_->typeFilter = filter;
    pimpl_->invalidateModel();
    emit filterChanged();
}

void
ConversationModel::joinConversations(const QString& uidA, const QString& uidB)
{
    auto conversationAIdx = pimpl_->indexOf(uidA);
    auto conversationBIdx = pimpl_->indexOf(uidB);
    if (conversationAIdx == -1 || conversationBIdx == -1 || !owner.enabled)
        return;
    auto& conversationA = pimpl_->conversations[conversationAIdx];
    auto& conversationB = pimpl_->conversations[conversationBIdx];

    if (conversationA.callId.isEmpty() || conversationB.callId.isEmpty())
        return;

    if (conversationA.confId.isEmpty()) {
        if (conversationB.confId.isEmpty()) {
            owner.callModel->joinCalls(conversationA.callId, conversationB.callId);
        } else {
            owner.callModel->joinCalls(conversationA.callId, conversationB.confId);
            conversationA.confId = conversationB.confId;
        }
    } else {
        if (conversationB.confId.isEmpty()) {
            owner.callModel->joinCalls(conversationA.confId, conversationB.callId);
            conversationB.confId = conversationA.confId;
        } else {
            owner.callModel->joinCalls(conversationA.confId, conversationB.confId);
            conversationB.confId = conversationA.confId;
        }
    }
}

void
ConversationModel::clearHistory(const QString& uid)
{
    auto conversationIdx = pimpl_->indexOf(uid);
    if (conversationIdx == -1)
        return;

    auto& conversation = pimpl_->conversations.at(conversationIdx);
    // Remove all TEXT interactions from database
    storage::clearHistory(pimpl_->db, uid);
    // Update conversation
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[uid]);
        conversation.interactions.clear();
    }
    storage::getHistory(pimpl_->db, conversation); // will contains "Conversation started"

    emit modelChanged();
    emit conversationCleared(uid);
}

void
ConversationModel::clearInteractionFromConversation(const QString& convId,
                                                    const QString& interactionId)
{
    auto conversationIdx = pimpl_->indexOf(convId);
    if (conversationIdx == -1)
        return;

    auto erased_keys = 0;
    bool lastInteractionUpdated = false;
    bool updateDisplayedUid = false;
    QString newDisplayedUid = 0;
    QString participantURI = "";
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
        try {
            auto& conversation = pimpl_->conversations.at(conversationIdx);
            if (conversation.mode != conversation::Mode::NON_SWARM) {
                return;
            }
            storage::clearInteractionFromConversation(pimpl_->db, convId, interactionId);
            erased_keys = conversation.interactions.erase(interactionId);
            auto messageId = conversation.lastDisplayedMessageUid.find(
                conversation.participants.front());

            if (messageId != conversation.lastDisplayedMessageUid.end()
                && messageId->second == interactionId) {
                // Update lastDisplayedMessageUid
                for (auto iter = conversation.interactions.find(interactionId);
                     iter != conversation.interactions.end();
                     --iter) {
                    if (isOutgoing(iter->second) && iter->first != interactionId) {
                        newDisplayedUid = iter->first;
                        break;
                    }
                }
                updateDisplayedUid = true;
                participantURI = conversation.participants.front();
                conversation.lastDisplayedMessageUid.at(conversation.participants.front())
                    = newDisplayedUid;
            }

            if (conversation.lastMessageUid == interactionId) {
                // Update lastMessageUid
                auto newLastId = QString::number(0);
                if (!conversation.interactions.empty())
                    newLastId = conversation.interactions.rbegin()->first;
                conversation.lastMessageUid = newLastId;
                lastInteractionUpdated = true;
            }

        } catch (const std::out_of_range& e) {
            qDebug() << "can't clear interaction from conversation: " << e.what();
        }
    }
    if (updateDisplayedUid) {
        emit displayedInteractionChanged(convId, participantURI, interactionId, newDisplayedUid);
    }
    if (erased_keys > 0) {
        pimpl_->filteredConversations.invalidate();
        emit interactionRemoved(convId, interactionId);
    }
    if (lastInteractionUpdated) {
        // last interaction as changed, so the order can changes.
        emit modelChanged();
    }
}

void
ConversationModel::retryInteraction(const QString& convId, const QString& interactionId)
{
    auto conversationIdx = pimpl_->indexOf(convId);
    if (conversationIdx == -1)
        return;

    auto interactionType = interaction::Type::INVALID;
    QString body = {};
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
        try {
            auto& conversation = pimpl_->conversations.at(conversationIdx);
            if (conversation.mode != conversation::Mode::NON_SWARM) {
                return;
            }

            auto& interactions = conversation.interactions;
            auto it = interactions.find(interactionId);
            if (it == interactions.end())
                return;

            if (!interaction::isOutgoing(it->second))
                return; // Do not retry non outgoing info

            if (it->second.type == interaction::Type::TEXT
                || (it->second.type == interaction::Type::DATA_TRANSFER
                    && interaction::isOutgoing(it->second))) {
                body = it->second.body;
                interactionType = it->second.type;
            } else
                return;

            storage::clearInteractionFromConversation(pimpl_->db, convId, interactionId);
            conversation.interactions.erase(interactionId);
        } catch (const std::out_of_range& e) {
            qDebug() << "can't find interaction from conversation: " << e.what();
            return;
        }
    }
    emit interactionRemoved(convId, interactionId);

    // Send a new interaction like the previous one
    if (interactionType == interaction::Type::TEXT) {
        sendMessage(convId, body);
    } else {
        // send file
        QFileInfo f(body);
        sendFile(convId, body, f.fileName());
    }
}

bool
ConversationModel::isLastDisplayed(const QString& convId,
                                   const QString& interactionId,
                                   const QString participant)
{
    auto conversationIdx = pimpl_->indexOf(convId);
    try {
        auto& conversation = pimpl_->conversations.at(conversationIdx);
        if (conversation.lastDisplayedMessageUid.find(participant)
            != conversation.lastDisplayedMessageUid.end()) {
            return conversation.lastDisplayedMessageUid.find(participant)->second == interactionId;
        }
    } catch (const std::out_of_range& e) {
    }
    return false;
}

void
ConversationModel::clearAllHistory()
{
    storage::clearAllHistory(pimpl_->db);

    for (auto& conversation : pimpl_->conversations) {
        {
            if (conversation.mode != conversation::Mode::NON_SWARM) {
                continue;
            }
            std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[conversation.uid]);
            conversation.interactions.clear();
        }
        storage::getHistory(pimpl_->db, conversation);
    }
    emit modelChanged();
}

void
ConversationModel::setInteractionRead(const QString& convId, const QString& interactionId)
{
    auto conversationIdx = pimpl_->indexOf(convId);
    if (conversationIdx == -1) {
        return;
    }
    bool emitUpdated = false;
    interaction::Info itCopy;
    {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
        auto& interactions = pimpl_->conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            emitUpdated = true;
            if (it->second.isRead) {
                return;
            }
            it->second.isRead = true;
            if (pimpl_->conversations[conversationIdx].unreadMessages != 0)
                pimpl_->conversations[conversationIdx].unreadMessages -= 1;
            itCopy = it->second;
        }
    }
    if (emitUpdated) {
        pimpl_->invalidateModel();
        if (pimpl_->conversations[conversationIdx].mode != conversation::Mode::NON_SWARM) {
            ConfigurationManager::instance().setMessageDisplayed(owner.id,
                                                                 "swarm:" + convId,
                                                                 interactionId,
                                                                 3);
        } else {
            auto daemonId = storage::getDaemonIdByInteractionId(pimpl_->db, interactionId);
            if (owner.profileInfo.type != profile::Type::SIP) {
                ConfigurationManager::instance().setMessageDisplayed(
                    owner.id,
                    "jami:" + pimpl_->conversations[conversationIdx].participants.front(),
                    daemonId,
                    3);
            }
            storage::setInteractionRead(pimpl_->db, interactionId);
        }
        emit interactionStatusUpdated(convId, interactionId, itCopy);
        emit pimpl_->behaviorController.newReadInteraction(owner.id, convId, interactionId);
    }
}

void
ConversationModel::clearUnreadInteractions(const QString& convId)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(convId).get();
        bool emitUpdated = false;
        QString lastDisplayed;
        {
            std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convId]);
            auto& interactions = conversation.interactions;
            std::for_each(interactions.begin(),
                          interactions.end(),
                          [&](decltype(*interactions.begin())& it) {
                              if (!it.second.isRead) {
                                  emitUpdated = true;
                                  it.second.isRead = true;
                                  if (conversation.isSwarm) {
                                      lastDisplayed = it.first;
                                      return;
                                  }
                                  if (owner.profileInfo.type != profile::Type::SIP)
                                      lastDisplayed = storage::getDaemonIdByInteractionId(pimpl_->db,
                                                                                          it.first);
                                  storage::setInteractionRead(pimpl_->db, it.first);
                              }
                          });
        }
        if (!lastDisplayed.isEmpty()) {
            auto to = conversation.mode != conversation::Mode::NON_SWARM ? "swarm:" + convId
                                           : "jami:" + conversation.participants.first();
            ConfigurationManager::instance().setMessageDisplayed(owner.id, to, lastDisplayed, 3);
        }
        if (emitUpdated) {
            conversation.unreadMessages = 0;
            pimpl_->invalidateModel();
            emit conversationUpdated(convId);
        }
    } catch (std::out_of_range& e) {
    }
}

int
ConversationModel::loadConversationMessages(const QString& conversationId, const int size)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(conversationId).get();
        if (conversation.allMessagesLoaded) {
            return -1;
        }
        auto lastMsgId = conversation.interactions.empty()
                             ? ""
                             : conversation.interactions.front().first;
        return ConfigurationManager::instance().loadConversationMessages(owner.id,
                                                                         conversationId,
                                                                         lastMsgId,
                                                                         size);
    } catch (std::out_of_range& e) {
        return -1;
    }
}

void
ConversationModel::acceptConversationRequest(const QString& conversationId)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(conversationId).get();
        auto participant = conversation.getOneToOneParticipant(owner.profileInfo.uri);
        if (conversation.mode == conversation::Mode::NON_SWARM) {
            pimpl_->sendContactRequest(participant);
            return;
        }
        // for swarm conversation add contact if not added yet. Otherwise accept request
        if (!participant.isEmpty()) {
            auto contact = owner.contactModel->getContact(participant);
            auto isNotUsed = contact.profileInfo.type == profile::Type::TEMPORARY
                             || contact.profileInfo.type == profile::Type::PENDING;
            if (isNotUsed) {
                owner.contactModel->addContact(contact);
                return;
            }
        }
        ConfigurationManager::instance().acceptConversationRequest(owner.id, conversationId);
    } catch (std::out_of_range& e) {
    }
}

void
ConversationModel::declineConversationRequest(const QString& conversationId, bool banned)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(conversationId).get();
        auto participant = conversation.getOneToOneParticipant(owner.profileInfo.uri);
        // for non-swarm and one-to-one conversation remove contact.
        if (!participant.isEmpty()) {
            removeConversation(conversationId, banned);
        } else {
            ConfigurationManager::instance().declineConversationRequest(owner.id, conversationId);
        }
    } catch (std::out_of_range& e) {
    }
}

void
ConversationModel::addConversationMember(const QString& conversationId, const QString& memberURI)
{
    ConfigurationManager::instance().addConversationMember(owner.id, conversationId, memberURI);
}

void
ConversationModel::removeConversationMember(const QString& conversationId, const QString& memberURI)
{
    ConfigurationManager::instance().removeConversationMember(owner.id, conversationId, memberURI);
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
    , typeFilter(FilterType::INVALID)
    , customTypeFilter(FilterType::INVALID)
    , behaviorController(behaviorController)
{
    filteredConversations.bindSortCallback(this, &ConversationModelPimpl::sort);
    filteredConversations.bindFilterCallback(this, &ConversationModelPimpl::filter);

    initConversations();

    // Contact related
    connect(&*linked.owner.contactModel,
            &ContactModel::modelUpdated,
            this,
            &ConversationModelPimpl::slotContactModelUpdated);
    connect(&*linked.owner.contactModel,
            &ContactModel::contactAdded,
            this,
            &ConversationModelPimpl::slotContactAdded);
    connect(&*linked.owner.contactModel,
            &ContactModel::incomingContactRequest,
            this,
            &ConversationModelPimpl::slotIncomingContactRequest);
    connect(&*linked.owner.contactModel,
            &ContactModel::pendingContactAccepted,
            this,
            &ConversationModelPimpl::slotPendingContactAccepted);
    connect(&*linked.owner.contactModel,
            &ContactModel::contactRemoved,
            this,
            &ConversationModelPimpl::slotContactRemoved);

    // Messages related
    connect(&*linked.owner.contactModel,
            &lrc::ContactModel::newAccountMessage,
            this,
            &ConversationModelPimpl::slotNewAccountMessage);
    connect(&callbacksHandler,
            &CallbacksHandler::incomingCallMessage,
            this,
            &ConversationModelPimpl::slotIncomingCallMessage);
    connect(&callbacksHandler,
            &CallbacksHandler::accountMessageStatusChanged,
            this,
            &ConversationModelPimpl::slotUpdateInteractionStatus);

    // Call related
    connect(&*linked.owner.contactModel,
            &ContactModel::incomingCall,
            this,
            &ConversationModelPimpl::slotIncomingCall);
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
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::composingStatusChanged,
            this,
            &ConversationModelPimpl::slotComposingStatusChanged);

    // data transfer
    connect(&*linked.owner.contactModel,
            &ContactModel::newAccountTransfer,
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
            &CallbacksHandler::transferStatusTimeoutExpired,
            this,
            &ConversationModelPimpl::slotTransferStatusTimeoutExpired);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusUnjoinable,
            this,
            &ConversationModelPimpl::slotTransferStatusUnjoinable);
    // swarm conversations
    connect(&callbacksHandler,
            &CallbacksHandler::conversationLoaded,
            this,
            &ConversationModelPimpl::slotConversationLoaded);
    connect(&callbacksHandler,
            &CallbacksHandler::messageReceived,
            this,
            &ConversationModelPimpl::slotMessageReceived);
    connect(&callbacksHandler,
            &CallbacksHandler::conversationRequestReceived,
            this,
            &ConversationModelPimpl::slotConversationRequestReceived);
    connect(&callbacksHandler,
            &CallbacksHandler::conversationReady,
            this,
            &ConversationModelPimpl::slotConversationReady);
    connect(&callbacksHandler,
            &CallbacksHandler::conversationRemoved,
            this,
            &ConversationModelPimpl::slotConversationRemoved);
    connect(&callbacksHandler,
            &CallbacksHandler::conversationMemberEvent,
            this,
            &ConversationModelPimpl::slotConversationMemberEvent);
}

ConversationModelPimpl::~ConversationModelPimpl()
{
    // Contact related
    disconnect(&*linked.owner.contactModel,
               &ContactModel::modelUpdated,
               this,
               &ConversationModelPimpl::slotContactModelUpdated);
    disconnect(&*linked.owner.contactModel,
               &ContactModel::contactAdded,
               this,
               &ConversationModelPimpl::slotContactAdded);
    disconnect(&*linked.owner.contactModel,
               &ContactModel::incomingContactRequest,
               this,
               &ConversationModelPimpl::slotIncomingContactRequest);
    disconnect(&*linked.owner.contactModel,
               &ContactModel::pendingContactAccepted,
               this,
               &ConversationModelPimpl::slotPendingContactAccepted);
    disconnect(&*linked.owner.contactModel,
               &ContactModel::contactRemoved,
               this,
               &ConversationModelPimpl::slotContactRemoved);

    // Messages related
    disconnect(&*linked.owner.contactModel,
               &lrc::ContactModel::newAccountMessage,
               this,
               &ConversationModelPimpl::slotNewAccountMessage);
    disconnect(&callbacksHandler,
               &CallbacksHandler::incomingCallMessage,
               this,
               &ConversationModelPimpl::slotIncomingCallMessage);
    disconnect(&callbacksHandler,
               &CallbacksHandler::accountMessageStatusChanged,
               this,
               &ConversationModelPimpl::slotUpdateInteractionStatus);

    // Call related
    disconnect(&*linked.owner.contactModel,
               &ContactModel::incomingCall,
               this,
               &ConversationModelPimpl::slotIncomingCall);
    disconnect(&*linked.owner.callModel,
               &lrc::api::NewCallModel::callStatusChanged,
               this,
               &ConversationModelPimpl::slotCallStatusChanged);
    disconnect(&*linked.owner.callModel,
               &lrc::api::NewCallModel::callStarted,
               this,
               &ConversationModelPimpl::slotCallStarted);
    disconnect(&*linked.owner.callModel,
               &lrc::api::NewCallModel::callEnded,
               this,
               &ConversationModelPimpl::slotCallEnded);
    disconnect(&*linked.owner.callModel,
               &lrc::api::NewCallModel::callAddedToConference,
               this,
               &ConversationModelPimpl::slotCallAddedToConference);
    disconnect(&callbacksHandler,
               &CallbacksHandler::conferenceRemoved,
               this,
               &ConversationModelPimpl::slotConferenceRemoved);
    disconnect(&ConfigurationManager::instance(),
               &ConfigurationManagerInterface::composingStatusChanged,
               this,
               &ConversationModelPimpl::slotComposingStatusChanged);

    // data transfer
    disconnect(&*linked.owner.contactModel,
               &ContactModel::newAccountTransfer,
               this,
               &ConversationModelPimpl::slotTransferStatusCreated);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusCanceled,
               this,
               &ConversationModelPimpl::slotTransferStatusCanceled);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusAwaitingPeer,
               this,
               &ConversationModelPimpl::slotTransferStatusAwaitingPeer);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusAwaitingHost,
               this,
               &ConversationModelPimpl::slotTransferStatusAwaitingHost);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusOngoing,
               this,
               &ConversationModelPimpl::slotTransferStatusOngoing);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusFinished,
               this,
               &ConversationModelPimpl::slotTransferStatusFinished);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusError,
               this,
               &ConversationModelPimpl::slotTransferStatusError);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusTimeoutExpired,
               this,
               &ConversationModelPimpl::slotTransferStatusTimeoutExpired);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusUnjoinable,
               this,
               &ConversationModelPimpl::slotTransferStatusUnjoinable);
    // swarm conversations
    disconnect(&callbacksHandler,
               &CallbacksHandler::conversationLoaded,
               this,
               &ConversationModelPimpl::slotConversationLoaded);
    disconnect(&callbacksHandler,
               &CallbacksHandler::messageReceived,
               this,
               &ConversationModelPimpl::slotMessageReceived);
    disconnect(&callbacksHandler,
               &CallbacksHandler::conversationRequestReceived,
               this,
               &ConversationModelPimpl::slotConversationRequestReceived);
    disconnect(&callbacksHandler,
               &CallbacksHandler::conversationReady,
               this,
               &ConversationModelPimpl::slotConversationReady);
    disconnect(&callbacksHandler,
               &CallbacksHandler::conversationRemoved,
               this,
               &ConversationModelPimpl::slotConversationRemoved);
    disconnect(&callbacksHandler,
               &CallbacksHandler::conversationMemberEvent,
               this,
               &ConversationModelPimpl::slotConversationMemberEvent);
}

void
ConversationModelPimpl::initConversations()
{
    const MapStringString accountDetails = ConfigurationManager::instance().getAccountDetails(
        linked.owner.id);
    if (accountDetails.empty())
        return;

    // Fill swarm conversations
    QStringList swarm = ConfigurationManager::instance().getConversations(linked.owner.id);
    for (auto& swarmConv : swarm) {
        addSwarmConversation(swarmConv);
    }

    VectorMapStringString conversationsRequests = ConfigurationManager::instance()
                                                      .getConversationRequests(linked.owner.id);
    for (auto& request : conversationsRequests) {
        addConversationRequest(request);
    }

    // Fill conversations
    for (auto const& c : linked.owner.contactModel->getAllContacts().toStdMap()) {
        auto conv = storage::getConversationsWithPeer(db, c.second.profileInfo.uri);
        if (hasOneOneSwarmWith(c.second.profileInfo.uri))
            continue;
        if (conv.empty()) {
            // Can't find a conversation with this contact
            // add pending not swarm conversation
            if (c.second.profileInfo.type == profile::Type::PENDING) {
                addContactRequest(c.second.profileInfo.uri);
                continue;
            }
            conv.push_back(storage::beginConversationWithPeer(db, c.second.profileInfo.uri));
        }
        addConversationWith(conv[0], c.first);

        auto convIdx = indexOf(conv[0]);

        // Check if file transfer interactions were left in an incorrect state
        std::lock_guard<std::mutex> lk(interactionsLocks[conversations[convIdx].uid]);
        for (auto& interaction : conversations[convIdx].interactions) {
            if (interaction.second.status == interaction::Status::TRANSFER_CREATED
                || interaction.second.status == interaction::Status::TRANSFER_AWAITING_HOST
                || interaction.second.status == interaction::Status::TRANSFER_AWAITING_PEER
                || interaction.second.status == interaction::Status::TRANSFER_ONGOING
                || interaction.second.status == interaction::Status::TRANSFER_ACCEPTED) {
                // If a datatransfer was left in a non-terminal status in DB, we switch this status
                // to ERROR
                // TODO : Improve for DBus clients as daemon and transfer may still be ongoing
                storage::updateInteractionStatus(db,
                                                 interaction.first,
                                                 interaction::Status::TRANSFER_ERROR);
                interaction.second.status = interaction::Status::TRANSFER_ERROR;
            }
        }
    }
    invalidateModel();

    filteredConversations.reset(conversations).sort();

    // Load all non treated messages for this account
    QVector<Message> messages = ConfigurationManager::instance()
                                    .getLastMessages(linked.owner.id, storage::getLastTimestamp(db));
    for (const auto& message : messages) {
        uint64_t timestamp = 0;
        try {
            timestamp = static_cast<uint64_t>(message.received);
        } catch (...) {
        }
        addIncomingMessage(message.from, message.payloads["text/plain"], timestamp);
    }
}

bool
ConversationModelPimpl::filter(const conversation::Info& entry)
{
    try {
        auto contactInfo = linked.owner.contactModel->getContact(entry.participants.front());

        auto uri = URI(currentFilter);
        bool stripScheme = (uri.schemeType() < URI::SchemeType::COUNT__);
        FlagPack<URI::Section> flags = URI::Section::USER_INFO | URI::Section::HOSTNAME
                                       | URI::Section::PORT;
        if (!stripScheme) {
            flags |= URI::Section::SCHEME;
        }

        currentFilter = uri.format(flags);

        // Check contact
        // If contact is banned, only match if filter is a perfect match
        if (contactInfo.isBanned) {
            if (currentFilter == "")
                return false;
            return contactInfo.profileInfo.uri == currentFilter
                   || contactInfo.profileInfo.alias == currentFilter
                   || contactInfo.registeredName == currentFilter;
        }

        std::regex regexFilter;
        auto isValidReFilter = true;
        try {
            regexFilter = std::regex(currentFilter.toStdString(), std::regex_constants::icase);
        } catch (std::regex_error&) {
            isValidReFilter = false;
        }

        auto filterUriAndReg = [regexFilter, isValidReFilter](auto contact, auto filter) {
            auto result = contact.profileInfo.uri.contains(filter)
                          || contact.registeredName.contains(filter);
            if (!result) {
                auto regexFound = isValidReFilter
                                      ? (!contact.profileInfo.uri.isEmpty()
                                         && std::regex_search(contact.profileInfo.uri.toStdString(),
                                                              regexFilter))
                                            || std::regex_search(contact.registeredName.toStdString(),
                                                                 regexFilter)
                                      : false;
                result |= regexFound;
            }
            return result;
        };

        // Check type
        switch (typeFilter) {
        case FilterType::JAMI:
        case FilterType::SIP:
            if (entry.isRequest)
                return false;
            if (contactInfo.profileInfo.type == profile::Type::TEMPORARY)
                return filterUriAndReg(contactInfo, currentFilter);
            break;
        case FilterType::REQUEST:
            if (!entry.isRequest)
                return false;
            break;
        default:
            break;
        }

        // Otherwise perform usual regex search
        bool result = contactInfo.profileInfo.alias.contains(currentFilter);
        if (!result && isValidReFilter)
            result |= std::regex_search(contactInfo.profileInfo.alias.toStdString(), regexFilter);
        if (!result)
            result |= filterUriAndReg(contactInfo, currentFilter);
        return result;
    } catch (std::out_of_range&) {
        // getContact() failed
        return false;
    }
}

bool
ConversationModelPimpl::sort(const conversation::Info& convA, const conversation::Info& convB)
{
    // A or B is a temporary contact
    if (convA.participants.isEmpty())
        return true;
    if (convB.participants.isEmpty())
        return false;

    if (convA.uid == convB.uid)
        return false;

    auto& mtxA = interactionsLocks[convA.uid];
    auto& mtxB = interactionsLocks[convB.uid];
    std::lock(mtxA, mtxB);
    std::lock_guard<std::mutex> lockConvA(mtxA, std::adopt_lock);
    std::lock_guard<std::mutex> lockConvB(mtxB, std::adopt_lock);

    auto historyA = convA.interactions;
    auto historyB = convB.interactions;

    // A or B is a new conversation (without CONTACT interaction)
    if (convA.uid.isEmpty() || convB.uid.isEmpty())
        return convA.uid.isEmpty();

    if (historyA.empty() && historyB.empty()) {
        // If no information to compare, sort by Ring ID
        return convA.participants.front() > convB.participants.front();
    }
    if (historyA.empty())
        return false;
    if (historyB.empty())
        return true;
    // Sort by last Interaction
    try {
        auto lastMessageA = historyA.at(convA.lastMessageUid);
        auto lastMessageB = historyB.at(convB.lastMessageUid);
        return lastMessageA.timestamp > lastMessageB.timestamp;
    } catch (const std::exception& e) {
        qDebug() << "ConversationModel::sortConversations(), can't get lastMessage";
        return false;
    }
}

void
ConversationModelPimpl::sendContactRequest(const QString& contactUri)
{
    auto contact = linked.owner.contactModel->getContact(contactUri);
    auto isNotUsed = contact.profileInfo.type == profile::Type::TEMPORARY
                     || contact.profileInfo.type == profile::Type::PENDING;
    if (isNotUsed)
        linked.owner.contactModel->addContact(contact);
}
void
ConversationModelPimpl::slotConversationLoaded(uint32_t loadingRequestId,
                                               const QString& accountId,
                                               const QString& conversationId,
                                               VectorMapStringString messages)
{
    if (accountId != linked.owner.id) {
        return;
    }
    try {
        auto& conversation = getConversationForUid(conversationId).get();
        auto size = messages.size();
        for (int i = size - 1; i >= 0; --i) {
            auto message = messages[i];
            if (message["type"].isEmpty()) {
                continue;
            }
            if (message["type"] == "initial") {
                conversation.allMessagesLoaded = true;
                if (message.find("invited") == message.end()) {
                    continue;
                }
            }
            auto msgId = message["id"];
            auto msg = interaction::Info(message, linked.owner.profileInfo.uri);
            if (msg.type == interaction::Type::DATA_TRANSFER) {
                auto daemponID = message["tid"];
                storage::updateDataTransferInteractionForDaemonId(db, daemponID, msg);
            } else if (msg.type == interaction::Type::CALL) {
                msg.body = storage::getCallInteractionString(msg.authorUri, msg.duration);
            }
            insertSwarmInteraction(msgId, msg, conversation, true);
        }

        for (int i = conversation.interactions.size() - 1; i >= 0; i--) {
            if (conversation.interactions.atIndex(i).second.type != interaction::Type::MERGE) {
                conversation.lastMessageUid = conversation.interactions.atIndex(i).first;
                break;
            }
        }
        invalidateModel();
        emit linked.modelChanged();
        emit linked.newMessagesAvailable(linked.owner.id, conversationId);
    } catch (const std::exception& e) {
        qDebug() << "messages loaded for not existing conversation";
    }
}

void
ConversationModelPimpl::slotMessageReceived(const QString& accountId,
                                            const QString& conversationId,
                                            MapStringString message)
{
    if (accountId != linked.owner.id) {
        return;
    }
    try {
        auto& conversation = getConversationForUid(conversationId).get();
        if (message["type"].isEmpty()) {
            return;
        }
        if (message["type"] == "initial") {
            conversation.allMessagesLoaded = true;
            if (message.find("invited") == message.end()) {
                return;
            }
        }
        auto msgId = message["id"];
        auto msg = interaction::Info(message, linked.owner.profileInfo.uri);
        api::datatransfer::Info info;
        DataTransferId transferIdInt;
        if (msg.type == interaction::Type::DATA_TRANSFER) {
            // save data transfer interaction to db and assosiate daemon id with interaction id,
            // conversation id and db id
            QString transferId = message["tid"];
            transferIdInt = std::stoull(message["tid"].toStdString());
            lrc.getDataTransferModel().transferInfo(accountId, conversationId, transferIdInt, info);
            // create db entry for valid data transfer
            if (info.status != datatransfer::Status::INVALID) {
                msg.body = info.path;
                msg.status = (info.status == datatransfer::Status::on_connection
                              && !interaction::isOutgoing(msg))
                                 ? interaction::Status::TRANSFER_AWAITING_HOST
                                 : interaction::Status::TRANSFER_CREATED;
                auto interactionId = storage::addDataTransferToConversation(db,
                                                                            conversationId,
                                                                            info);
                transfIdToDbIntId[transferId] = interactionId;
                lrc.getDataTransferModel().registerTransferId(transferIdInt, msgId);
            }
        } else if (msg.type == interaction::Type::CALL) {
            msg.body = storage::getCallInteractionString(msg.authorUri, msg.duration);
        } else if (msg.type == interaction::Type::TEXT
                   && msg.authorUri != linked.owner.profileInfo.uri) {
            conversation.unreadMessages++;
        }
        insertSwarmInteraction(msgId, msg, conversation, false);
        if (msg.type == interaction::Type::MERGE) {
            invalidateModel();
            return;
        }
        conversation.lastMessageUid = msgId;
        invalidateModel();
        if (!interaction::isOutgoing(msg)) {
            emit behaviorController.newUnreadInteraction(linked.owner.id,
                                                         conversationId,
                                                         msgId,
                                                         msg);
        }
        emit linked.newInteraction(conversationId, msgId, msg);
        emit linked.modelChanged();
        if (msg.status == interaction::Status::TRANSFER_AWAITING_HOST) {
            awaitingHost(transferIdInt, info);
        }
    } catch (const std::exception& e) {
        qDebug() << "messages received for not existing conversation";
    }
}

void
ConversationModelPimpl::insertSwarmInteraction(const QString& interactionId,
                                               const interaction::Info& interaction,
                                               conversation::Info& conversation,
                                               bool insertAtBegin)
{
    std::lock_guard<std::mutex> lk(interactionsLocks[conversation.uid]);
    int index = conversation.interactions.indexOfMessage(interaction.parentId);
    if (index >= 0) {
        conversation.interactions.insert(index + 1, qMakePair(interactionId, interaction));
    } else {
        conversation.interactions.insert(std::make_pair(interactionId, interaction), insertAtBegin);
        conversation.parentsId[interactionId] = interaction.parentId;
    }
    if (!conversation.parentsId.values().contains(interactionId)) {
        return;
    }
    auto msgIds = conversation.parentsId.keys(interactionId);
    conversation.interactions.moveMessages(msgIds, interactionId);
    for (auto& msg : msgIds) {
        conversation.parentsId.remove(msg);
    }
}

void
ConversationModelPimpl::slotConversationRequestReceived(const QString& accountId,
                                                        const QString& conversationId,
                                                        MapStringString metadatas)
{
    if (accountId != linked.owner.id) {
        return;
    }
    addConversationRequest(metadatas);
}

void
ConversationModelPimpl::slotConversationReady(const QString& accountId,
                                              const QString& conversationId)
{
    // we receive this signal after we accept conversation request or after we send conversation request
    if (accountId != linked.owner.id) {
        return;
    }
    // remove non swarm conversation that was added from slotContactAdded
    const VectorMapStringString& members = ConfigurationManager::instance()
                                               .getConversationMembers(accountId, conversationId);
    auto accountURI = linked.owner.profileInfo.uri;
    VectorString uris;
    for (auto& member : members) {
        if (member["uri"] != accountURI) {
            uris.append(member["uri"]);
        }
        try {
            auto& conversation = getConversationForPeerUri(member["uri"]).get();
            if (conversation.mode == conversation::Mode::NON_SWARM or conversation.needsSyncing) {
                conversations.erase(conversations.begin() + indexOf(conversation.uid));
                storage::removeContact(db, member["uri"]);
                invalidateModel();
                emit linked.conversationRemoved(conversation.uid);
            }
        } catch (...) {
        }
    }
    if (indexOf(conversationId) < 0) {
        addSwarmConversation(conversationId);
    } else {
        // if swarm request already exists, update participnts
        auto& conversation = getConversationForUid(conversationId).get();
        conversation.participants = uris;
        const MapStringString& details = ConfigurationManager::instance()
        .conversationInfos(accountId, conversationId);
        conversation.mode = conversation::to_mode(details["mode"].toInt());
        conversation.isRequest = false;
        auto id = ConfigurationManager::instance().loadConversationMessages(linked.owner.id,
                                                                            conversationId,
                                                                            "",
                                                                            0);
    }
    invalidateModel();
    emit linked.conversationReady(conversationId, uris.front());
    emit linked.newConversation(conversationId);
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotConversationRemoved(const QString& accountId,
                                                const QString& conversationId)
{
    auto conversationIndex = indexOf(conversationId);
    if (accountId != linked.owner.id || conversationIndex < 0) {
        return;
    }
    try {
        auto& conversation = getConversationForUid(conversationId).get();
        auto contactUri = conversation.participants.first();
        // remove swarm conversation
        conversations.erase(conversations.begin() + conversationIndex);
        // if swarm conversation removed but we still have contact we create non swarm conversation.
        auto contact = linked.owner.contactModel->getContact(contactUri);
        if (contact.profileInfo.type != api::profile::Type::JAMI) {
            return;
        }
        auto conv = storage::getConversationsWithPeer(db, contactUri);
        if (conv.empty()) {
            conv.push_back(storage::beginConversationWithPeer(db, contactUri));
        }
        addConversationWith(conv[0], contactUri);
        invalidateModel();
        emit linked.conversationRemoved(conversationId);
        emit linked.newConversation(conv[0]);
        emit linked.modelChanged();
    } catch (...) {
    }
}

void
ConversationModelPimpl::slotConversationMemberEvent(const QString& accountId,
                                                    const QString& conversationId,
                                                    const QString& memberUri,
                                                    int event)
{
    if (accountId != linked.owner.id || indexOf(conversationId) < 0) {
        return;
    }
    switch (event) {
    case 0: // add
        // clear search result
        for (unsigned int i = 0; i < searchResults.size(); ++i) {
            if (searchResults.at(i).uid == memberUri)
                searchResults.erase(searchResults.begin() + i);
        }
        break;
    case 1: // joins
        break;
    case 2: // leave
        break;
    case 3: // banned
        break;
    }
    // update participants
    auto& conversation = getConversationForUid(conversationId).get();
    const VectorMapStringString& members
        = ConfigurationManager::instance().getConversationMembers(linked.owner.id, conversationId);
    VectorString uris;
    auto accountURI = linked.owner.profileInfo.uri;

    for (auto& member : members) {
        if (member["uri"] != accountURI) {
            uris.append(member["uri"]);
        }
    }
    conversation.participants = uris;
    invalidateModel();
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotIncomingContactRequest(const QString& contactUri) {
    // It is contact request. But for compatibility with swarm conversations we add it like non swarm conversation request.
    addContactRequest(contactUri);
}

void
ConversationModelPimpl::slotContactAdded(const QString& contactUri)
{
    auto conv = storage::getConversationsWithPeer(db, contactUri);
    bool addConversation = false;
    try {
        auto& conversation = getConversationForPeerUri(contactUri).get();
        // swarm conversation we update when receive conversation ready.
        if (conversation.mode != conversation::Mode::NON_SWARM) {
            QStringList swarms = ConfigurationManager::instance().getConversations(linked.owner.id);
            conversation.needsSyncing = swarms.indexOf(conversation.uid) == -1;
            invalidateModel();
            emit linked.modelChanged();
            return;
        }
        if (conv.empty()) {
            conv.push_back(storage::beginConversationWithPeer(db, contactUri));
        }
        // remove temporary conversation that was added when receiving an incoming request
        if (indexOf(contactUri) >= 0) {
            conversations.erase(conversations.begin() + indexOf(contactUri));
        }
        // add a conversation if not exists
        addConversation = indexOf(conv[0]) == -1;
    } catch (std::out_of_range&) {
        /*
         if the conversation does not exists we save it to DB and add non-swarm
         conversation to the conversion list. After receiving a conversation request or
         conversation ready signal swarm conversation should be updated and removed from DB.
         */
        addConversation = true;
        if (conv.empty()) {
            conv.push_back(storage::beginConversationWithPeer(db, contactUri));
        }
    }
    if (addConversation) {
        addConversationWith(conv[0], contactUri);
        invalidateModel();
        emit linked.conversationReady(conv[0], contactUri);
        emit linked.newConversation(conv[0]);
        emit linked.modelChanged();
    }
}

void
ConversationModelPimpl::removePendingConversation(const QString& contactUri)
{
    if (indexOf(contactUri) < 0) {
        return;
    }
    emit linked.creatingConversationEvent(linked.owner.id, contactUri, contactUri, 0);
    auto cb = std::function<void(QString, QString)>([this](QString conversationId,
                                                           QString participantURI) {
        if (indexOf(participantURI) < 0) {
            return;
        }
        conversations.erase(conversations.begin() + indexOf(participantURI));
        emit linked.modelChanged();
        emit linked.creatingConversationEvent(linked.owner.id, conversationId, participantURI, 1);
    });

    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = connect(&this->linked,
                          &ConversationModel::conversationReady,
                          [connection, cb, contactUri](QString convId, QString participantURI) {
                              if (contactUri != participantURI) {
                                  return;
                              }
                              cb(convId, participantURI);
                              QObject::disconnect(*connection);
                              if (connection) {
                                  delete connection;
                              }
                          });
}

void
ConversationModelPimpl::addContactRequest(const QString& contactUri) {
    try {
        auto& conv = getConversationForPeerUri(contactUri).get();
        // request from contact already exists, return
        return;
    } catch (std::out_of_range&) {
        // no conversation exists. Add contact request
        conversation::Info conversation;
        conversation.uid = contactUri;
        conversation.accountId = linked.owner.id;
        conversation.participants = {contactUri};
        conversation.mode = conversation::Mode::NON_SWARM;
        conversation.isRequest = true;
        conversations.emplace_back(std::move(conversation));
        invalidateModel();
        emit linked.newConversation(contactUri);
        emit linked.modelChanged();
    }
}

void
ConversationModelPimpl::addConversationRequest(const MapStringString& convRequest) {
    auto convId = convRequest["id"];
    auto convIdx = indexOf(convId);
    if (convIdx != -1)
        return;
    
    auto peer = convRequest["from"];
    auto mode = conversation::to_mode(convRequest["mode"].toInt());
    try {
        // check if we have contact request for peer
        auto& conv = getConversationForPeerUri(peer).get();
        if (conv.mode == conversation::Mode::NON_SWARM) {
            // update conversation and remoe conversation from db
            conv.mode = mode;
            conv.uid = convId;
            storage::removeContact(db, peer);
            invalidateModel();
            emit linked.modelChanged();
            return;
        }
    } catch (std::out_of_range&) {}
    conversation::Info conversation;
    conversation.uid = convId;
    conversation.accountId = linked.owner.id;
    conversation.participants = {peer};
    conversation.mode = mode;
    conversation.isRequest = true;
    conversations.emplace_back(std::move(conversation));
    invalidateModel();
    emit linked.newConversation(peer);
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotPendingContactAccepted(const QString& uri)
{
    auto type = linked.owner.profileInfo.type;
    try {
        type = linked.owner.contactModel->getContact(uri).profileInfo.type;
    } catch (std::out_of_range& e) {
    }
    profile::Info profileInfo {uri, {}, {}, type};
    storage::createOrUpdateProfile(linked.owner.id, profileInfo, true);
    auto convs = storage::getConversationsWithPeer(db, uri);
    if (!convs.empty()) {
        try {
            auto contact = linked.owner.contactModel->getContact(uri);
            auto interaction = interaction::Info {uri,
                                                  {},
                                                  std::time(nullptr),
                                                  0,
                                                  interaction::Type::CONTACT,
                                                  interaction::Status::SUCCESS,
                                                  true};
            auto msgId = storage::addMessageToConversation(db, convs[0], interaction);
            interaction.body = storage::getContactInteractionString(uri,
                                                                    interaction::Status::SUCCESS);
            auto convIdx = indexOf(convs[0]);
            if (convIdx >= 0) {
                std::lock_guard<std::mutex> lk(interactionsLocks[conversations[convIdx].uid]);
                conversations[convIdx].interactions.emplace(msgId, interaction);
            }
            filteredConversations.invalidate();
            emit linked.newInteraction(convs[0], msgId, interaction);
        } catch (std::out_of_range& e) {
            qDebug() << "ConversationModelPimpl::slotContactAdded can't find contact";
        }
    }
}

void
ConversationModelPimpl::slotContactRemoved(const QString& uri)
{
    auto conversationIdx = indexOfContact(uri);
    if (conversationIdx == -1) {
        qDebug() << "ConversationModelPimpl::slotContactRemoved, but conversation not found";
        return; // Not a contact
    }
    auto& conversationUid = conversations[conversationIdx].uid;
    conversations.erase(conversations.begin() + conversationIdx);
    invalidateModel();
    emit linked.conversationRemoved(conversationUid);
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotContactModelUpdated(const QString& uri, bool needsSorted)
{
    // presence updated
    if (!needsSorted) {
        try {
            auto& conversation = getConversationForPeerUri(uri, true).get();
            invalidateModel();
            emit linked.conversationUpdated(conversation.uid);
        } catch (std::out_of_range&) {
            qDebug() << "contact updated for not existing conversation";
        }
        return;
    }

    if (currentFilter.isEmpty()) {
        if (searchResults.empty())
            return;
        searchResults.clear();
        emit linked.searchResultUpdated();
        return;
    }
    searchResults.clear();
    auto users = linked.owner.contactModel->getSearchResults();
    for (auto& user : users) {
        conversation::Info conversationInfo;
        conversationInfo.uid = user.profileInfo.uri;
        conversationInfo.participants.push_back(user.profileInfo.uri);
        conversationInfo.accountId = linked.owner.id;
        searchResults.emplace_front(std::move(conversationInfo));
    }
    emit linked.searchResultUpdated();
}

void
ConversationModelPimpl::addSwarmConversation(const QString& convId)
{
    VectorString uris;
    const VectorMapStringString& members = ConfigurationManager::instance()
                                               .getConversationMembers(linked.owner.id, convId);
    auto accountURI = linked.owner.profileInfo.uri;
    for (auto& member : members) {
        if (member["uri"] != accountURI) {
            uris.append(member["uri"]);
        }
    }
    if (uris.isEmpty()) {
        return;
    }
    conversation::Info conversation;
    conversation.uid = convId;
    conversation.accountId = linked.owner.id;
    conversation.participants = uris;
    const MapStringString& details = ConfigurationManager::instance()
    .conversationInfos(linked.owner.id, convId);
    conversation.mode = conversation::to_mode(details["mode"].toInt());
    conversation.needsSyncing = false;
    try {
        conversation.confId = linked.owner.callModel->getConferenceFromURI(uris.first()).id;
    } catch (...) {
        conversation.confId = "";
    }
    try {
        conversation.callId = linked.owner.callModel->getCallFromURI(uris.first()).id;
    } catch (...) {
        conversation.callId = "";
    }
    auto participant = conversation.getOneToOneParticipant(linked.owner.profileInfo.uri);
    // remove non swarm conversation
    if (!participant.isEmpty()) {
        try {
            auto& conv = getConversationForPeerUri(participant).get();
            if (conv.mode == conversation::Mode::NON_SWARM && !conv.needsSyncing) {
                conversations.erase(conversations.begin() + indexOf(conv.uid));
                storage::removeContact(db, participant);
            }
        } catch (...) {
        }
    }
    conversations.emplace_back(std::move(conversation));
    auto id = ConfigurationManager::instance().loadConversationMessages(linked.owner.id,
                                                                        convId,
                                                                        "",
                                                                        5);
}

void
ConversationModelPimpl::addConversationWith(const QString& convId, const QString& contactUri)
{
    conversation::Info conversation;
    conversation.uid = convId;
    conversation.accountId = linked.owner.id;
    conversation.participants = {contactUri};
    try {
        // TODO will be true when we drop non swarm support
        auto contactInfo = linked.owner.contactModel->getContact(contactUri);
        conversation.mode = contactInfo.conversationId.isEmpty() ?  conversation::Mode::NON_SWARM : conversation::Mode::ONE_TO_ONE;
        conversation.needsSyncing = !contactInfo.conversationId.isEmpty();
    } catch (...) {
        conversation.mode = conversation::Mode::NON_SWARM;
    }
    try {
        conversation.confId = linked.owner.callModel->getConferenceFromURI(contactUri).id;
    } catch (...) {
        conversation.confId = "";
    }
    try {
        conversation.callId = linked.owner.callModel->getCallFromURI(contactUri).id;
    } catch (...) {
        conversation.callId = "";
    }
    storage::getHistory(db, conversation);
    std::vector<std::function<void(void)>> updateSlots;
    {
        std::lock_guard<std::mutex> lk(interactionsLocks[convId]);
        for (auto& interaction : conversation.interactions) {
            if (interaction.second.status != interaction::Status::SENDING) {
                continue;
            }
            // Get the message status from daemon, else unknown
            auto id = storage::getDaemonIdByInteractionId(db, interaction.first);
            int status = 0;
            if (id.isEmpty()) {
                continue;
            }
            try {
                auto msgId = std::stoull(id.toStdString());
                status = ConfigurationManager::instance().getMessageStatus(msgId);
                updateSlots.emplace_back([this, convId, contactUri, id, status]() -> void {
                    auto accId = linked.owner.id;
                    slotUpdateInteractionStatus(accId, convId, contactUri, id, status);
                });
            } catch (const std::exception& e) {
                qDebug() << "message id was invalid";
            }
        }
    }
    for (const auto& s : updateSlots) {
        s();
    }

    conversation.unreadMessages = getNumberOfUnreadMessagesFor(convId);
    conversations.emplace_back(std::move(conversation));
    invalidateModel();
}

int
ConversationModelPimpl::indexOf(const QString& uid) const
{
    for (unsigned int i = 0; i < conversations.size(); ++i) {
        if (conversations.at(i).uid == uid)
            return i;
    }
    return -1;
}

std::reference_wrapper<conversation::Info>
ConversationModelPimpl::getConversation(const FilterPredicate& pred, const bool searchResultIncluded)
{
    auto conv = std::find_if(conversations.cbegin(), conversations.cend(), pred);
    if (conv != conversations.cend()) {
        return std::remove_const_t<conversation::Info&>(*conv);
    }

    if (searchResultIncluded) {
        auto sr = std::find_if(searchResults.cbegin(), searchResults.cend(), pred);
        if (sr != searchResults.cend()) {
            return std::remove_const_t<conversation::Info&>(*sr);
        }
    }

    throw std::out_of_range("Conversation out of range");
}

std::reference_wrapper<conversation::Info>
ConversationModelPimpl::getConversationForUid(const QString& uid, const bool searchResultIncluded)
{
    return getConversation([uid](const conversation::Info& conv) -> bool { return uid == conv.uid; },
                           searchResultIncluded);
}

std::reference_wrapper<conversation::Info>
ConversationModelPimpl::getConversationForPeerUri(const QString& uri,
                                                  const bool searchResultIncluded)
{
    return getConversation([uri](const conversation::Info& conv)
                               -> bool { return uri == conv.participants.front(); },
                           searchResultIncluded);
}

int
ConversationModelPimpl::indexOfContact(const QString& uri) const
{
    for (unsigned int i = 0; i < conversations.size(); ++i) {
        if (conversations.at(i).participants.front() == uri
            && conversations.at(i).participants.size() == 1)
            return i;
    }
    return -1;
}

void
ConversationModelPimpl::slotIncomingCall(const QString& fromId, const QString& callId)
{
    auto convIds = storage::getConversationsWithPeer(db, fromId);
    if (convIds.empty()) {
        // in case if we receive call after removing contact add conversation request;
        try {
            auto contact = linked.owner.contactModel->getContact(fromId);
            if (contact.profileInfo.type == profile::Type::PENDING && !contact.isBanned) {
                addContactRequest(fromId);
            }
        } catch (const std::out_of_range&) {
        }
    }
    auto conversationIdx = indexOfContact(fromId);

    if (conversationIdx == -1) {
        qDebug() << "ConversationModelPimpl::slotIncomingCall, but conversation not found";
        return; // Not a contact
    }

    auto& conversation = conversations.at(conversationIdx);

    qDebug() << "Add call to conversation with " << fromId;
    conversation.callId = callId;
    invalidateModel();
    emit behaviorController.showIncomingCallView(linked.owner.id, conversation.uid);
}

void
ConversationModelPimpl::slotCallStatusChanged(const QString& callId, int code)
{
    Q_UNUSED(code)
    // Get conversation
    auto i = std::find_if(conversations.begin(),
                          conversations.end(),
                          [callId](const conversation::Info& conversation) {
                              return conversation.callId == callId;
                          });

    try {
        auto call = linked.owner.callModel->getCall(callId);
        if (i == conversations.end()) {
            // In this case, the user didn't pass through placeCall
            // This means that a participant was invited to a call
            // or a call was placed via dbus.
            // We have to update the model
            for (auto& conversation : conversations) {
                if (conversation.participants.front() == call.peerUri) {
                    conversation.callId = callId;
                    invalidateModel();
                    emit linked.conversationUpdated(conversation.uid);
                }
            }
        } else if (call.status == call::Status::PEER_BUSY) {
            emit behaviorController.showLeaveMessageView(linked.owner.id, i->uid);
        }
    } catch (std::out_of_range& e) {
        qDebug() << "ConversationModelPimpl::slotCallStatusChanged can't get inexistant call";
    }
}

void
ConversationModelPimpl::slotCallStarted(const QString& callId)
{
    try {
        auto call = linked.owner.callModel->getCall(callId);
        addOrUpdateCallMessage(callId, call.peerUri.remove("ring:"), !call.isOutgoing);
    } catch (std::out_of_range& e) {
        qDebug() << "ConversationModelPimpl::slotCallStarted can't start inexistant call";
    }
}

void
ConversationModelPimpl::slotCallEnded(const QString& callId)
{
    try {
        auto call = linked.owner.callModel->getCall(callId);
        // get duration
        std::time_t duration = 0;
        if (call.startTime.time_since_epoch().count() != 0) {
            auto duration_ns = std::chrono::steady_clock::now() - call.startTime;
            duration = std::chrono::duration_cast<std::chrono::seconds>(duration_ns).count();
        }
        // add or update call interaction with duration
        addOrUpdateCallMessage(callId, call.peerUri.remove("ring:"), !call.isOutgoing, duration);
        /* Reset the callId stored in the conversation.
           Do not call selectConversation() since it is already done in slotCallStatusChanged. */
        for (auto& conversation : conversations)
            if (conversation.callId == callId) {
                conversation.callId = "";
                conversation.confId = ""; // The participant is detached
                invalidateModel();
                emit linked.conversationUpdated(conversation.uid);
            }
    } catch (std::out_of_range& e) {
        qDebug() << "ConversationModelPimpl::slotCallEnded can't end inexistant call";
    }
}

void
ConversationModelPimpl::addOrUpdateCallMessage(const QString& callId,
                                               const QString& from,
                                               bool incoming,
                                               const std::time_t& duration)
{
    // do not save call interaction for swarm conversation
    auto convIds = storage::getConversationsWithPeer(db, from);
    if (convIds.empty()) {
        // in case if we receive call after removing contact add conversation request;
        try {
            auto contact = linked.owner.contactModel->getContact(from);
            if (contact.profileInfo.type == profile::Type::PENDING && !contact.isBanned) {
                addContactRequest(from);
            }
        } catch (const std::out_of_range&) {
        }
        return;
    }
    // Get conversation
    auto conv_it = std::find_if(conversations.begin(),
                                conversations.end(),
                                [&callId](const conversation::Info& conversation) {
                                    return conversation.callId == callId;
                                });
    if (conv_it == conversations.end()) {
        return;
    }
    auto uid = conv_it->uid;
    auto uriString = incoming ? storage::prepareUri(from, linked.owner.profileInfo.type) : "";
    auto msg = interaction::Info {uriString,
                                  {},
                                  std::time(nullptr),
                                  duration,
                                  interaction::Type::CALL,
                                  interaction::Status::SUCCESS,
                                  true};
    // update the db
    auto msgId = storage::addOrUpdateMessage(db, conv_it->uid, msg, callId);
    // now set the formatted call message string in memory only
    msg.body = storage::getCallInteractionString(uriString, duration);
    auto newInteraction = conv_it->interactions.find(msgId) == conv_it->interactions.end();
    if (newInteraction) {
        conv_it->lastMessageUid = msgId;
        std::lock_guard<std::mutex> lk(interactionsLocks[conv_it->uid]);
        conv_it->interactions.emplace(msgId, msg);
    } else {
        std::lock_guard<std::mutex> lk(interactionsLocks[conv_it->uid]);
        conv_it->interactions[msgId] = msg;
    }

    if (newInteraction)
        emit linked.newInteraction(conv_it->uid, msgId, msg);
    else
        emit linked.interactionStatusUpdated(conv_it->uid, msgId, msg);

    invalidateModel();
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotNewAccountMessage(const QString& accountId,
                                              const QString& from,
                                              const QString& msgId,
                                              const MapStringString& payloads)
{
    if (accountId != linked.owner.id)
        return;

    for (const auto& payload : payloads.keys()) {
        if (payload.contains("text/plain")) {
            addIncomingMessage(from, payloads.value(payload), 0, msgId);
        }
    }
}

void
ConversationModelPimpl::slotIncomingCallMessage(const QString& callId,
                                                const QString& from,
                                                const QString& body)
{
    if (not linked.owner.callModel->hasCall(callId))
        return;

    auto& call = linked.owner.callModel->getCall(callId);
    if (call.type == call::Type::CONFERENCE) {
        // Show messages in all conversations for conferences.
        for (const auto& conversation : conversations) {
            if (conversation.confId == callId) {
                if (conversation.participants.empty()) {
                    continue;
                }
                addIncomingMessage(from, body);
            }
        }
    } else {
        addIncomingMessage(from, body);
    }
}

QString
ConversationModelPimpl::addIncomingMessage(const QString& from,
                                           const QString& body,
                                           const uint64_t& timestamp,
                                           const QString& daemonId)
{
    auto convIds = storage::getConversationsWithPeer(db, from);
    if (convIds.empty()) {
        // in case if we receive message after removing contact add conversation request;
        try {
            auto contact = linked.owner.contactModel->getContact(from);
            if (contact.profileInfo.type == profile::Type::PENDING && !contact.isBanned) {
                addContactRequest(from);
            }
        } catch (const std::out_of_range&) {
        }
        return "";
    }
    auto msg = interaction::Info {from,
                                  body,
                                  timestamp == 0 ? std::time(nullptr)
                                                 : static_cast<time_t>(timestamp),
                                  0,
                                  interaction::Type::TEXT,
                                  interaction::Status::SUCCESS,
                                  false};
    auto msgId = storage::addMessageToConversation(db, convIds[0], msg);
    if (!daemonId.isEmpty()) {
        storage::addDaemonMsgId(db, msgId, daemonId);
    }
    auto conversationIdx = indexOf(convIds[0]);
    // Add the conversation if not already here
    if (conversationIdx == -1) {
        addConversationWith(convIds[0], from);
        emit linked.newConversation(convIds[0]);
    } else {
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            conversations[conversationIdx].interactions.emplace(msgId, msg);
        }
        conversations[conversationIdx].lastMessageUid = msgId;
        conversations[conversationIdx].unreadMessages = getNumberOfUnreadMessagesFor(convIds[0]);
    }

    emit behaviorController.newUnreadInteraction(linked.owner.id, convIds[0], msgId, msg);
    emit linked.newInteraction(convIds[0], msgId, msg);

    invalidateModel();
    emit linked.modelChanged();

    return msgId;
}

void
ConversationModelPimpl::slotCallAddedToConference(const QString& callId, const QString& confId)
{
    for (auto& conversation : conversations) {
        if (conversation.callId == callId && conversation.confId != confId) {
            conversation.confId = confId;
            invalidateModel();
            // Refresh the conference status only if attached
            MapStringString confDetails = CallManager::instance().getConferenceDetails(confId);
            if (confDetails["STATE"] == "ACTIVE_ATTACHED")
                emit linked.selectConversation(conversation.uid);
        }
    }
}

void
ConversationModelPimpl::slotUpdateInteractionStatus(const QString& accountId,
                                                    const QString& conversationId,
                                                    const QString& peer,
                                                    const QString& messageId,
                                                    int status)
{
    if (accountId != linked.owner.id) {
        return;
    }
    // it may be not swarm conversation check in db
    if (conversationId.isEmpty() || conversationId == linked.owner.profileInfo.uri) {
        auto convIds = storage::getConversationsWithPeer(db, peer);
        if (convIds.empty()) {
            return;
        }
        auto conversationIdx = indexOf(convIds[0]);
        auto& conversation = conversations[conversationIdx];
        auto newStatus = interaction::Status::INVALID;
        switch (static_cast<DRing::Account::MessageStates>(status)) {
        case DRing::Account::MessageStates::SENDING:
            newStatus = interaction::Status::SENDING;
            break;
        case DRing::Account::MessageStates::CANCELLED:
            newStatus = interaction::Status::TRANSFER_CANCELED;
            break;
        case DRing::Account::MessageStates::SENT:
            newStatus = interaction::Status::SUCCESS;
            break;
        case DRing::Account::MessageStates::FAILURE:
            newStatus = interaction::Status::FAILURE;
            break;
        case DRing::Account::MessageStates::DISPLAYED:
            newStatus = interaction::Status::DISPLAYED;
            break;
        case DRing::Account::MessageStates::UNKNOWN:
        default:
            newStatus = interaction::Status::UNKNOWN;
            break;
        }
        auto idString = messageId;
        // for not swarm conversation messageId in hexdesimal string format. Convert to normal string
        // TODO messageId should be received from daemon in string format
        if (static_cast<DRing::Account::MessageStates>(status)
            == DRing::Account::MessageStates::DISPLAYED) {
            std::istringstream ss(messageId.toStdString());
            ss >> std::hex;
            uint64_t id;
            if (!(ss >> id))
                return;
            idString = QString::number(id);
        }
        // Update database
        auto interactionId = storage::getInteractionIdByDaemonId(db, idString);
        if (interactionId.isEmpty()) {
            return;
        }
        auto msgId = interactionId;
        storage::updateInteractionStatus(db, msgId, newStatus);
        // Update conversations
        interaction::Info itCopy;
        bool emitUpdated = false;
        bool updateDisplayedUid = false;
        QString oldDisplayedUid = 0;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversation.uid]);
            auto& interactions = conversation.interactions;
            auto it = interactions.find(msgId);
            auto messageId = conversation.lastDisplayedMessageUid.find(peer);
            if (it != interactions.end()) {
                it->second.status = newStatus;
                bool interactionDisplayed = newStatus == interaction::Status::DISPLAYED
                                            && isOutgoing(it->second);
                if (messageId != conversation.lastDisplayedMessageUid.end()) {
                    auto lastDisplayedIt = interactions.find(messageId->second);
                    bool interactionIsLast = lastDisplayedIt == interactions.end()
                                             || lastDisplayedIt->second.timestamp
                                                    < it->second.timestamp;
                    updateDisplayedUid = interactionDisplayed && interactionIsLast;
                    if (updateDisplayedUid) {
                        oldDisplayedUid = messageId->second;
                        conversation.lastDisplayedMessageUid.at(peer) = it->first;
                    }
                } else {
                    oldDisplayedUid = "";
                    conversation.lastDisplayedMessageUid[peer] = it->first;
                    updateDisplayedUid = true;
                }
                emitUpdated = true;
                itCopy = it->second;
            }
        }
        if (updateDisplayedUid) {
            emit linked.displayedInteractionChanged(conversation.uid, peer, oldDisplayedUid, msgId);
        }
        if (emitUpdated) {
            invalidateModel();
            emit linked.interactionStatusUpdated(conversation.uid, msgId, itCopy);
        }
        return;
    }
    try {
        auto& conversation = getConversationForUid(conversationId).get();
        if (conversation.mode != conversation::Mode::NON_SWARM) {
            if (static_cast<DRing::Account::MessageStates>(status)
                == DRing::Account::MessageStates::DISPLAYED) {
                if (conversation.lastDisplayedMessageUid.find(peer)
                    == conversation.lastDisplayedMessageUid.end()) {
                    conversation.lastDisplayedMessageUid[peer] = messageId;
                    emit linked.displayedInteractionChanged(conversationId, peer, "", messageId);
                } else if (conversation.interactions.indexOfMessage(
                               conversation.lastDisplayedMessageUid.find(peer)->second)
                           < conversation.interactions.indexOfMessage(messageId)) {
                    auto lastDisplayedMsg = conversation.lastDisplayedMessageUid.find(peer)->second;
                    conversation.lastDisplayedMessageUid[peer] = messageId;
                    emit linked.displayedInteractionChanged(conversationId,
                                                            peer,
                                                            lastDisplayedMsg,
                                                            messageId);
                }
            }
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "could not update message status for not existing conversation";
    }
}

void
ConversationModelPimpl::slotConferenceRemoved(const QString& confId)
{
    // Get conversation
    for (auto& i : conversations) {
        if (i.confId == confId) {
            i.confId = "";
        }
    }
}

void
ConversationModelPimpl::slotComposingStatusChanged(const QString& accountId,
                                                   const QString& convId,
                                                   const QString& contactUri,
                                                   bool isComposing)
{
    if (accountId != linked.owner.id)
        return;
    emit linked.composingStatusChanged(convId, contactUri, isComposing);
}

int
ConversationModelPimpl::getNumberOfUnreadMessagesFor(const QString& uid)
{
    return storage::countUnreadFromInteractions(db, uid);
}

void
ConversationModel::setIsComposing(const QString& convUid, bool isComposing)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(convUid).get();
        QString to = conversation.mode != conversation::Mode::NON_SWARM ? "swarm:" + convUid : "jami:" + conversation.participants.first();
        ConfigurationManager::instance().setIsComposing(owner.id, to, isComposing);
    } catch (...) {
    }
}

void
ConversationModel::sendFile(const QString& convUid, const QString& path, const QString& filename)
{
    try {
        auto& conversation = pimpl_->getConversationForUid(convUid, true).get();
        bool isSwarm = conversation.mode != conversation::Mode::NON_SWARM;

        const auto peerUri = conversation.participants.front();
        bool isTemporary = peerUri == convUid;

        /* It is necessary to make a copy of convUid since it may very well point to
         a field in the temporary conversation, which is going to be destroyed by
         slotContactAdded() (indirectly triggered by sendContactrequest(). Not doing
         so may result in use after free/crash. */
        auto convUidCopy = convUid;

        pimpl_->sendContactRequest(peerUri);

        auto cb = std::function<void(QString, QString)>(
            [this, isTemporary, peerUri, path, filename, isSwarm](QString conversationId,
                                                                  QString participantURI) {
                if (pimpl_->indexOf(conversationId) < 0 || participantURI != peerUri) {
                    qDebug() << "Can't send file";
                    return;
                }

                auto contactInfo = owner.contactModel->getContact(participantURI);
                if (contactInfo.isBanned) {
                    qDebug() << "ContactModel::sendFile: denied, contact is banned";
                    return;
                }

                auto to = isSwarm ? conversationId : participantURI;
                pimpl_->lrc.getDataTransferModel().sendFile(owner.id, to, path, filename);
            });

        if (isTemporary) {
            QMetaObject::Connection* const connection = new QMetaObject::Connection;
            *connection = connect(this,
                                  &ConversationModel::conversationReady,
                                  [cb, connection, convUidCopy](QString conversationId,
                                                                QString participantURI) {
                                      if (participantURI != convUidCopy) {
                                          return;
                                      }
                                      cb(conversationId, participantURI);
                                      QObject::disconnect(*connection);
                                      if (connection) {
                                          delete connection;
                                      }
                                  });
        } else {
            cb(convUidCopy, peerUri);
        }
    } catch (const std::out_of_range& e) {
        qDebug() << "could not send file to not existing conversation";
    }
}

void
ConversationModel::acceptTransfer(const QString& convUid, const QString& interactionId)
{
    lrc::api::datatransfer::Info info = {};
    getTransferInfo(convUid, interactionId, info);
    acceptTransfer(convUid, interactionId, info.displayName);
}

void
ConversationModel::acceptTransfer(const QString& convUid,
                                  const QString& interactionId,
                                  const QString& path)
{
    pimpl_->acceptTransfer(convUid, interactionId, path);
}

void
ConversationModel::cancelTransfer(const QString& convUid, const QString& interactionId)
{
    // For this action, we change interaction status before effective canceling as daemon will
    // emit Finished event code immediately (before leaving this method) in non-DBus mode.
    auto conversationIdx = pimpl_->indexOf(convUid);
    interaction::Info itCopy;
    bool emitUpdated = false;
    if (conversationIdx != -1) {
        std::lock_guard<std::mutex> lk(pimpl_->interactionsLocks[convUid]);
        auto& interactions = pimpl_->conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            it->second.status = interaction::Status::TRANSFER_CANCELED;

            // update information in the db
            storage::updateInteractionStatus(pimpl_->db,
                                             interactionId,
                                             interaction::Status::TRANSFER_CANCELED);
            emitUpdated = true;
            itCopy = it->second;
        }
    }
    if (emitUpdated) {
        // for swarm conversations we need to provide conversation id to accept file, for not swarm
        // conversations we need peer uri
        lrc::api::datatransfer::Info info = {};
        getTransferInfo(convUid, interactionId, info);
        auto identifier = info.conversationId.isEmpty() ? info.peerUri : convUid;
        // Forward cancel action to daemon (will invoke slotTransferStatusCanceled)
        pimpl_->lrc.getDataTransferModel().cancel(owner.id, identifier, interactionId);
        pimpl_->invalidateModel();
        emit interactionStatusUpdated(convUid, interactionId, itCopy);
        emit pimpl_->behaviorController.newReadInteraction(owner.id, convUid, interactionId);
    }
}

void
ConversationModel::getTransferInfo(const QString& conversationId,
                                   const QString& interactionId,
                                   datatransfer::Info& info)
{
    try {
        auto dringId = pimpl_->lrc.getDataTransferModel().getDringIdFromInteractionId(interactionId);
        pimpl_->lrc.getDataTransferModel().transferInfo(owner.id, conversationId, dringId, info);
    } catch (...) {
        info.status = datatransfer::Status::INVALID;
    }
}

int
ConversationModel::getNumberOfUnreadMessagesFor(const QString& convUid)
{
    return pimpl_->getNumberOfUnreadMessagesFor(convUid);
}

bool
ConversationModelPimpl::usefulDataFromDataTransfer(DataTransferId dringId,
                                                   const datatransfer::Info& info,
                                                   QString& interactionId,
                                                   QString& conversationId)
{
    if (info.accountId != linked.owner.id)
        return false;
    try {
        interactionId = lrc.getDataTransferModel().getInteractionIdFromDringId(dringId);
        conversationId = info.conversationId.isEmpty()
                             ? storage::conversationIdFromInteractionId(db, interactionId)
                             : info.conversationId;
    } catch (const std::out_of_range& e) {
        qWarning() << "Couldn't get interaction from daemon Id: " << dringId;
        return false;
    }
    return true;
}

void
ConversationModelPimpl::slotTransferStatusCreated(DataTransferId dringId, datatransfer::Info info)
{
    // check if transfer is for the current account
    if (info.accountId != linked.owner.id)
        return;

    const MapStringString accountDetails = ConfigurationManager::instance().getAccountDetails(
        linked.owner.id);
    if (accountDetails.empty())
        return;
    // create a new conversation if needed
    auto convIds = storage::getConversationsWithPeer(db, info.peerUri);
    if (convIds.empty()) {
        // in case if we receive file after removing contact add conversation request. If we have
        // swarm request this function will do nothing.
        try {
            auto contact = linked.owner.contactModel->getContact(info.peerUri);
            if (contact.profileInfo.type == profile::Type::PENDING && !contact.isBanned) {
                addContactRequest(info.peerUri);
            }
        } catch (const std::out_of_range&) {
        }
        return;
    }

    // add interaction to the db
    const auto& convId = convIds[0];
    auto interactionId = storage::addDataTransferToConversation(db, convId, info);

    // map dringId and interactionId for latter retrivial from client (that only known the interactionId)
    lrc.getDataTransferModel().registerTransferId(dringId, interactionId);

    auto interaction = interaction::Info {info.isOutgoing ? "" : info.peerUri,
                                          info.isOutgoing ? info.path : info.displayName,
                                          std::time(nullptr),
                                          0,
                                          interaction::Type::DATA_TRANSFER,
                                          interaction::Status::TRANSFER_CREATED,
                                          false};

    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convId);
    if (conversationIdx == -1) {
        addConversationWith(convId, info.peerUri);
        emit linked.newConversation(convId);
    } else {
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            conversations[conversationIdx].interactions.emplace(interactionId, interaction);
        }
        conversations[conversationIdx].lastMessageUid = interactionId;
        conversations[conversationIdx].unreadMessages = getNumberOfUnreadMessagesFor(convId);
    }
    emit behaviorController.newUnreadInteraction(linked.owner.id,
                                                 convId,
                                                 interactionId,
                                                 interaction);
    emit linked.newInteraction(convId, interactionId, interaction);

    invalidateModel();
    emit linked.modelChanged();
}

void
ConversationModelPimpl::slotTransferStatusAwaitingPeer(DataTransferId dringId,
                                                       datatransfer::Info info)
{
    bool intUpdated;
    updateTransferStatus(dringId, info, interaction::Status::TRANSFER_AWAITING_PEER, intUpdated);
}

void
ConversationModelPimpl::slotTransferStatusAwaitingHost(DataTransferId dringId,
                                                       datatransfer::Info info)
{
    awaitingHost(dringId, info);
}

bool
ConversationModelPimpl::hasOneOneSwarmWith(const QString& participant)
{
    try {
        auto& conversation = getConversationForPeerUri(participant).get();
        return conversation.mode == conversation::Mode::ONE_TO_ONE;
    } catch (std::out_of_range&) {
        return false;
    }
}

void
ConversationModelPimpl::awaitingHost(DataTransferId dringId, datatransfer::Info info)
{
    QString interactionId;
    QString conversationId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, conversationId))
        return;

    bool intUpdated;

    if (!updateTransferStatus(dringId,
                              info,
                              interaction::Status::TRANSFER_AWAITING_HOST,
                              intUpdated)) {
        return;
    }
    if (!intUpdated) {
        return;
    }
    auto conversationIdx = indexOf(conversationId);
    // Only accept if contact is added
    if (!lrc.getDataTransferModel().acceptFromUnstrusted) {
        try {
            auto contactUri = conversations[conversationIdx].participants.front();
            auto contactInfo = linked.owner.contactModel->getContact(contactUri);
            if (contactInfo.profileInfo.type == profile::Type::PENDING)
                return;
        } catch (...) {
            return;
        }
    }
    // If it's an accepted file type and less than 20 MB, accept transfer.
    if (lrc.getDataTransferModel().automaticAcceptTransfer) {
        if (lrc.getDataTransferModel().acceptBehindMb == 0
            || info.totalSize < lrc.getDataTransferModel().acceptBehindMb * 1024 * 1024) {
            acceptTransfer(conversationId, interactionId, info.displayName);
        }
    }
}

void
ConversationModelPimpl::acceptTransfer(const QString& convUid,
                                       const QString& interactionId,
                                       const QString& path)
{
    auto destinationDir = lrc.getDataTransferModel().downloadDirectory;
    if (destinationDir.isEmpty()) {
        return;
    }
#ifdef Q_OS_WIN
    if (destinationDir.right(1) != '/') {
        destinationDir += "/";
    }
#endif
    QDir dir = QFileInfo(destinationDir + path).absoluteDir();
    if (!dir.exists())
        dir.mkpath(".");
    auto& conversation = getConversationForUid(convUid).get();
    // for swarm conversation we need converstion id to accept file and for not swarm participant uri
    auto identifier = conversation.mode != conversation::Mode::NON_SWARM ? convUid : conversation.participants.first();
    auto acceptedFilePath = lrc.getDataTransferModel().accept(linked.owner.id,
                                                              identifier,
                                                              interactionId,
                                                              destinationDir + path,
                                                              0);
    auto dringId = lrc.getDataTransferModel().getDringIdFromInteractionId(interactionId);
    if (transfIdToDbIntId.find(QString::number(dringId)) != transfIdToDbIntId.end()) {
        auto dbInteractionId = transfIdToDbIntId[QString::number(dringId)];
        storage::updateInteractionBody(db, dbInteractionId, acceptedFilePath);
        storage::updateInteractionStatus(db,
                                         dbInteractionId,
                                         interaction::Status::TRANSFER_ACCEPTED);
    } else {
        storage::updateInteractionBody(db, interactionId, acceptedFilePath);
        storage::updateInteractionStatus(db, interactionId, interaction::Status::TRANSFER_ACCEPTED);
    }
    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(convUid);
    interaction::Info itCopy;
    bool emitUpdated = false;
    if (conversationIdx != -1) {
        std::lock_guard<std::mutex> lk(interactionsLocks[convUid]);
        auto& interactions = conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            it->second.body = acceptedFilePath;
            it->second.status = interaction::Status::TRANSFER_ACCEPTED;
            emitUpdated = true;
            itCopy = it->second;
        }
    }
    if (emitUpdated) {
        sendContactRequest(conversations[conversationIdx].participants.front());
        invalidateModel();
        emit linked.interactionStatusUpdated(convUid, interactionId, itCopy);
        emit behaviorController.newReadInteraction(linked.owner.id, convUid, interactionId);
    }
}

void
ConversationModelPimpl::invalidateModel()
{
    filteredConversations.invalidate();
    customFilteredConversations.invalidate();
}

void
ConversationModelPimpl::slotTransferStatusOngoing(DataTransferId dringId, datatransfer::Info info)
{
    QString interactionId;
    QString conversationId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, conversationId))
        return;
    bool intUpdated;

    if (!updateTransferStatus(dringId, info, interaction::Status::TRANSFER_ONGOING, intUpdated)) {
        return;
    }
    if (!intUpdated) {
        return;
    }
    auto conversationIdx = indexOf(conversationId);
    auto* timer = new QTimer();
    connect(timer, &QTimer::timeout, [=] {
        updateTransfer(timer, conversationId, conversationIdx, interactionId);
    });
    timer->start(1000);
}

void
ConversationModelPimpl::slotTransferStatusFinished(DataTransferId dringId, datatransfer::Info info)
{
    QString interactionId;
    QString conversationId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, conversationId))
        return;
    // prepare interaction Info and emit signal for the client
    auto conversationIdx = indexOf(conversationId);
    if (conversationIdx != -1) {
        bool emitUpdated = false;
        auto newStatus = interaction::Status::TRANSFER_FINISHED;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversationId]);
            auto& interactions = conversations[conversationIdx].interactions;
            auto it = interactions.find(interactionId);
            if (it != interactions.end()) {
                // We need to check if current status is ONGOING as CANCELED must not be
                // transformed into FINISHED
                if (it->second.status == interaction::Status::TRANSFER_ONGOING) {
                    emitUpdated = true;
                    it->second.status = newStatus;
                    itCopy = it->second;
                }
            }
        }
        if (emitUpdated) {
            invalidateModel();
            if (conversations[conversationIdx].mode != conversation::Mode::NON_SWARM) {
                if (transfIdToDbIntId.find(QString::number(dringId)) != transfIdToDbIntId.end()) {
                    auto dbIntId = transfIdToDbIntId[QString::number(dringId)];
                    storage::updateInteractionStatus(db, dbIntId, newStatus);
                }
            } else {
                storage::updateInteractionStatus(db, interactionId, newStatus);
            }
            emit linked.interactionStatusUpdated(conversationId, interactionId, itCopy);
            transfIdToDbIntId.remove(QString::number(dringId));
        }
    }
}

void
ConversationModelPimpl::slotTransferStatusCanceled(DataTransferId dringId, datatransfer::Info info)
{
    bool intUpdated;
    updateTransferStatus(dringId, info, interaction::Status::TRANSFER_CANCELED, intUpdated);
}

void
ConversationModelPimpl::slotTransferStatusError(DataTransferId dringId, datatransfer::Info info)
{
    bool intUpdated;
    updateTransferStatus(dringId, info, interaction::Status::TRANSFER_ERROR, intUpdated);
}

void
ConversationModelPimpl::slotTransferStatusUnjoinable(DataTransferId dringId, datatransfer::Info info)
{
    bool intUpdated;
    updateTransferStatus(dringId, info, interaction::Status::TRANSFER_UNJOINABLE_PEER, intUpdated);
}

void
ConversationModelPimpl::slotTransferStatusTimeoutExpired(DataTransferId dringId,
                                                         datatransfer::Info info)
{
    bool intUpdated;
    updateTransferStatus(dringId, info, interaction::Status::TRANSFER_TIMEOUT_EXPIRED, intUpdated);
}

bool
ConversationModelPimpl::updateTransferStatus(DataTransferId dringId,
                                             datatransfer::Info info,
                                             interaction::Status newStatus,
                                             bool& updated)
{
    QString interactionId;
    QString conversationId;
    if (not usefulDataFromDataTransfer(dringId, info, interactionId, conversationId)) {
        return false;
    }

    auto conversationIdx = indexOf(conversationId);
    if (conversationIdx < 0) {
        return false;
    }
    if (conversations[conversationIdx].mode != conversation::Mode::NON_SWARM) {
        if (transfIdToDbIntId.find(QString::number(dringId)) == transfIdToDbIntId.end()) {
            return false;
        }
        auto dbIntId = transfIdToDbIntId[QString::number(dringId)];
        storage::updateInteractionStatus(db, dbIntId, newStatus);
    } else {
        storage::updateInteractionStatus(db, interactionId, newStatus);
    }
    bool emitUpdated = false;
    interaction::Info itCopy;
    {
        std::lock_guard<std::mutex> lk(interactionsLocks[conversationId]);
        auto& interactions = conversations[conversationIdx].interactions;
        auto it = interactions.find(interactionId);
        if (it != interactions.end()) {
            emitUpdated = true;
            it->second.status = newStatus;
            itCopy = it->second;
        }
    }
    if (emitUpdated) {
        invalidateModel();
        emit linked.interactionStatusUpdated(conversationId, interactionId, itCopy);
    }
    updated = emitUpdated;
    return true;
}

void
ConversationModelPimpl::updateTransfer(QTimer* timer,
                                       const QString& conversation,
                                       int conversationIdx,
                                       const QString& interactionId)
{
    try {
        bool emitUpdated = false;
        interaction::Info itCopy;
        {
            std::lock_guard<std::mutex> lk(interactionsLocks[conversations[conversationIdx].uid]);
            const auto& interactions = conversations[conversationIdx].interactions;
            const auto& it = interactions.find(interactionId);
            if (it != interactions.cend()
                and it->second.status == interaction::Status::TRANSFER_ONGOING) {
                emitUpdated = true;
                itCopy = it->second;
            }
        }
        if (emitUpdated) {
            emit linked.interactionStatusUpdated(conversation, interactionId, itCopy);
            return;
        }
    } catch (...) {
    }

    timer->stop();
    timer->deleteLater();
}

} // namespace lrc

#include "api/moc_conversationmodel.cpp"
#include "conversationmodel.moc"

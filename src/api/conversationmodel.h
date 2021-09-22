/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#pragma once

#include "typedefs.h"

#include "api/conversation.h"
#include "api/profile.h"
#include "api/datatransfer.h"
#include "containerview.h"

#include <QObject>
#include <QVector>
#include <QMap>

#include <memory>
#include <deque>

namespace lrc {

class CallbacksHandler;
class ConversationModelPimpl;
class Database;

namespace api {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

namespace account {
struct Info;
}
namespace interaction {
struct Info;
}

class Lrc;
class BehaviorController;
class NewAccountModel;

enum class ConferenceableItem { CALL, CONTACT };
Q_ENUM_NS(ConferenceableItem)

enum class FilterType { INVALID = -1, JAMI, SIP, REQUEST, COUNT__ };
Q_ENUM_NS(FilterType)

struct AccountConversation
{
    QString convId;
    QString accountId;
};

/*
 * vector of conversationId and accountId.
 * for calls and contacts contain only one element
 * for conferences contains multiple entries
 */
typedef QVector<QVector<AccountConversation>> ConferenceableValue;

/**
 *  @brief Class that manages conversation informations.
 */
class LIB_EXPORT ConversationModel : public QObject
{
    Q_OBJECT
public:
    using ConversationQueue = std::deque<conversation::Info>;
    using ConversationQueueProxy = ContainerView<ConversationQueue>;

    const account::Info& owner;

    ConversationModel(const account::Info& owner,
                      Lrc& lrc,
                      Database& db,
                      const CallbacksHandler& callbacksHandler,
                      const api::BehaviorController& behaviorController);
    ~ConversationModel();

    /**
     * Get unfiltered underlying conversation data. This is intended to
     * serve as the underlying data for QAbstractListModel based objects.
     * The corresponding data mutation signals will need to be responded
     * to with appropriate QAbstractListModel signal forwarding.
     * @return raw conversation queue
     */
    const ConversationQueue& getConversations() const;

    /**
     * Get conversations which should be shown client side
     * @return conversations filtered with the current filter
     */
    const ConversationQueueProxy& allFilteredConversations() const;

    /**
     * Get conversation for a given uid
     * @param uid conversation uid
     * @return reference to conversation info with given uid
     */
    OptRef<conversation::Info> getConversationForUid(const QString& uid) const;

    /**
     * Get conversation for a given peer uri
     * @param uri peer uri
     * @return reference to conversation info with given peer uri
     */
    OptRef<conversation::Info> getConversationForPeerUri(const QString& uri) const;

    /**
     * Get conversation for a given call id
     * @param callId call id
     * @return reference to conversation info with given call id
     */
    OptRef<conversation::Info> getConversationForCallId(const QString& callId) const;

    /**
     * Get conversations that could be added to conference
     * @param  current conversation id
     * @param  search name filter
     * @return filtered conversations
     */
    QMap<ConferenceableItem, ConferenceableValue> getConferenceableConversations(
        const QString& convId, const QString& filter = {}) const;
    /**
     * Get a custom filtered set of conversations
     * @return conversations filtered
     */
    const ConversationQueueProxy& getFilteredConversations(
        const FilterType& filter = FilterType::INVALID,
        bool forceUpdate = false,
        const bool includeBanned = false) const;
    /**
     * Get a custom filtered set of conversations from profile type
     * @return conversations filtered
     */
    const ConversationQueueProxy& getFilteredConversations(
        const profile::Type& profileType = profile::Type::INVALID,
        bool forceUpdate = false,
        const bool includeBanned = false) const;
    /**
     * Get the conversation at row in the filtered conversations
     * @param  row
     * @return a copy of the conversation
     */
    OptRef<conversation::Info> filteredConversation(unsigned row) const;
    /**
     * Get the search results
     * @return a searchResult
     */
    const ConversationQueue& getAllSearchResults() const;

    /**
     * Get the conversation at row in the search results
     * @param  row
     * @return a copy of the conversation
     */
    OptRef<conversation::Info> searchResultForRow(unsigned row) const;

    /**
     * Update the searchResults
     * @param new status
     */
    void updateSearchStatus(const QString& status) const;
    /**
     * Emit a filterChanged signal to force the client to refresh the filter. For instance
     * this is required when a contact was banned or un-banned.
     */
    void refreshFilter();
    /**
     * Make permanent a temporary contact or a pending request.
     * Ensure that given conversation is stored permanently into the system.
     * @param uid of the conversation to change.
     * @exception std::out_of_range if uid doesn't correspond to an existing conversation
     */
    void makePermanent(const QString& uid);
    /**
     * Remove a conversation and the contact if it's a dialog
     * @param uid of the conversation
     * @param banned if we want to ban the contact.
     */
    void removeConversation(const QString& uid, bool banned = false);
    /**
     * Get the action wanted by the user when they click on the conversation
     * @param uid of the conversation
     */
    void selectConversation(const QString& uid) const;
    /**
     * Call contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeCall(const QString& uid);
    /**
     * Perform an audio call with contacts linked to this conversation
     * @param uid of the conversation
     */
    void placeAudioOnlyCall(const QString& uid);
    /**
     * Send a message to the conversation
     * @param uid of the conversation
     * @param body of the message
     * @param parentId id of parent message. Default is "" - last message in conversation.
     */
    void sendMessage(const QString& uid, const QString& body, const QString& parentId = "");
    /**
     * Modify the current filter (will change the result of getFilteredConversations)
     * @param filter the new filter
     */
    void setFilter(const QString& filter);
    /**
     * Modify the current filter (will change the result of getFilteredConversations)
     * @param filter the new filter (example: SIP,  RING,  REQUEST)
     */
    void setFilter(const FilterType& filter = FilterType::INVALID);
    /**
     * Join participants from A to B and vice-versa.
     * @note conversations must be in a call.
     * @param uidA uid of the conversation A
     * @param uidB uid of the conversation B
     */
    void joinConversations(const QString& uidA, const QString& uidB);
    /**
     * Clear the history of a conversation
     * @param uid of the conversation
     */
    void clearHistory(const QString& uid);
    /**
     * change the status of the interaction from UNREAD to READ
     * @param convId, id of the conversation
     * @param msgId, id of the interaction
     */
    void setInteractionRead(const QString& convId, const QString& msgId);
    /**
     * Clears the unread text messages of a conversation
     * @param convId, uid of the conversation
     */
    void clearUnreadInteractions(const QString& convId);
    /**
     * clear all history
     */
    void clearAllHistory();
    /**
     * Clear one interaction from the history
     * @param convId
     * @param interactionId
     */
    void clearInteractionFromConversation(const QString& convId, const QString& interactionId);
    /**
     * Retry to send a message. In fact, will delete the previous interaction and resend a new one.
     * @param convId
     * @param interactionId
     */
    void retryInteraction(const QString& convId, const QString& interactionId);
    /**
     * @param convId
     * @param interactionId
     * @param participant uri
     * @return whether the interaction is last displayed for the conversation
     */
    bool isLastDisplayed(const QString& convId,
                         const QString& interactionId,
                         const QString participant);
    /**
     * delete obsolete history from the database
     * @param days, number of days from today. Below this date, interactions will be deleted
     */
    void deleteObsoleteHistory(int date);

    void sendFile(const QString& convUid, const QString& path, const QString& filename);

    void acceptTransfer(const QString& convUid, const QString& interactionId);

    void acceptTransfer(const QString& convUid, const QString& interactionId, const QString& path);

    void cancelTransfer(const QString& convUid, const QString& interactionId);

    void getTransferInfo(const QString& conversationId,
                         const QString& interactionId,
                         api::datatransfer::Info& info);
    /**
     * @param convUid, uid of the conversation
     * @return the number of unread messages for the conversation
     */
    int getNumberOfUnreadMessagesFor(const QString& convUid);
    /**
     * Send a composing status
     * @param convUid       conversation's id
     * @param isComposing   if is composing
     */
    void setIsComposing(const QString& convUid, bool isComposing);
    /**
     * load messages for conversation
     * @param conversationId conversation's id
     * @param size number of messages should be loaded. Default 1
     * @return id for loading request. -1 if not loaded
     */
    int loadConversationMessages(const QString& conversationId, const int size = 1);
    /**
     * accept request for conversation
     * @param conversationId conversation's id
     */
    void acceptConversationRequest(const QString& conversationId);
    /**
     * decline request for conversation
     * @param conversationId conversation's id
     * @param banned. Used for non-swarm and one-to-one conversation to remove contact
     */
    void declineConversationRequest(const QString& conversationId, bool banned = false);
    /**
     * add member to conversation
     * @param conversationId conversation's id
     * @param memberId members's id
     */
    void addConversationMember(const QString& conversationId, const QString& memberId);
    /**
     * remove member from conversation
     * @param conversationId conversation's id
     * @param memberId members's id
     */
    void removeConversationMember(const QString& conversationId, const QString& memberId);
    /**
     * get conversation info
     * @param conversationId conversation's id
     * @return conversation info
     */
    MapStringString getConversationInfos(const QString& conversationId);
    /**
     * create a new swarm conversation
     * @param participants  conversation's participants
     * @param title conversation title
     */
    void createConversation(const VectorString& participants, const QString& title = "");
    /**
     * update conversation info
     * @param conversationId conversation's id
     * @param info
     */
    void updateConversationInfo(const QString& conversationId, MapStringString info);

    /**
     * @return if conversations requests exists.
     */
    bool hasPendingRequests() const;
    /**
     * @return number of conversations requests
     */
    int pendingRequestCount() const;
    /**
     * @return number of conversations requests + unread
     */
    int notificationsCount() const;
    const VectorString peersForConversation(const QString& conversationId);

    // Presentation

    /**
     * Get conversation title. This means the title to show in the smartlist
     * @param conversationId
     * @return the title to display
     */
    QString title(const QString& conversationId) const;
    /**
     * Get conversation's description.
     * @param conversationId
     * @return the description to display
     */
    QString description(const QString& conversationId) const;

Q_SIGNALS:
    /**
     * Emitted when a conversation receives a new interaction
     * @param uid of conversation
     * @param interactionId
     * @param interactionInfo
     */
    void newInteraction(const QString& uid,
                        QString& interactionId,
                        const interaction::Info& interactionInfo) const;
    /**
     * Emitted when an interaction got a new status
     * @param convUid conversation which owns the interaction
     * @param interactionId
     * @param msg
     */
    void interactionStatusUpdated(const QString& convUid,
                                  const QString& interactionId,
                                  const api::interaction::Info& msg) const;
    /**
     * Emitted when an interaction got removed from the conversation
     * @param convUid conversation which owns the interaction
     * @param interactionId
     */
    void interactionRemoved(const QString& convUid, const QString& interactionId) const;
    /**
     * Emitted when user clear the history of a conversation
     * @param uid
     */
    void conversationCleared(const QString& uid) const;
    /**
     * Emitted when conversation's participant has been updated
     * @param uid
     */
    void conversationUpdated(const QString& uid) const;
    /**
     * Emitted when the conversations list is modified
     */
    void modelChanged() const;
    /**
     * Emitted when filter has changed
     */
    void filterChanged() const;
    /**
     * Emitted when a conversation has been added
     * @param uid
     */
    void newConversation(const QString& uid) const;
    /**
     * Emitted when a conversation has been removed
     * @param uid
     */
    void conversationRemoved(const QString& uid) const;
    /**
     * Emitted after all history were cleared
     * @note the client must connect this signal to know when update the view of the list
     */
    void allHistoryCleared() const;
    /**
     * Emitted at the end of slotContactAdded and at conversationReady for swarm conversation to
     * notify that an existing conversation can be modified
     * @param uid
     */
    void conversationReady(QString uid, QString participantURI) const;
    /**
     * Emitted when a contact in a conversation is composing a message
     * @param uid           conversation's id
     * @param contactUri    contact's uri
     * @param isComposing   if contact is composing a message
     */
    void composingStatusChanged(const QString& uid,
                                const QString& contactUri,
                                bool isComposing) const;
    /**
     * Emitted when last displayed interaction changed
     * @param uid of conversation
     * @param participant URI
     * @param previousUid uid of a previous displayed interaction
     * @param newdUid uid of a new displayed interaction
     */
    void displayedInteractionChanged(const QString& uid,
                                     const QString& participantURI,
                                     const QString& previousUid,
                                     const QString& newdUid) const;

    /**
     * Emitted when search status changed
     * @param status
     */
    void searchStatusChanged(const QString& status) const;
    /**
     * Emitted when search result has been updated
     */
    void searchResultUpdated() const;
    /**
     * Emitted when finish loading messages for conversation
     * @param loadingRequestId  loading request id
     * @param conversationId conversation Id
     */
    void conversationMessagesLoaded(uint32_t loadingRequestId, const QString& conversationId) const;
    /**
     * Emitted when new messages available. When messages loaded from loading request or
     * receiving/sending new interactions
     * @param accountId  account id
     * @param conversationId conversation Id
     */
    void newMessagesAvailable(const QString& accountId, const QString& conversationId) const;

    /**
     * Emitted when creation of conversation started, finished with success or finisfed with error
     * @param accountId  account id
     * @param conversationId conversation Id, when conversation creation started conversationId =
     * participantURI
     * @param participantURI participant uri
     * @param status 0 -started, 1 -created with success, -1 -error
     */
    void creatingConversationEvent(const QString& accountId,
                                   const QString& conversationId,
                                   const QString& participantURI,
                                   int status) const;

    /**
     * The following signals are intended for QAbtractListModel compatibility
     */

    /*!
     * Emitted before conversations are inserted into the underlying queue
     * @param position The starting row of the insertion
     * @param rows The number of items inserted
     */
    void beginInsertRows(int position, int rows = 1) const;

    //! Emitted once insertion is complete
    void endInsertRows() const;

    /*!
     * Emitted before conversations are removed from the underlying queue
     * @param position The starting row of the removal
     * @param rows The number of items removed
     */
    void beginRemoveRows(int position, int rows = 1) const;

    //! Emitted once removal is complete
    void endRemoveRows() const;

    /**
     * Emitted once a conversation has been updated
     * @param position
     */
    void dataChanged(int position) const;

private:
    std::unique_ptr<ConversationModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::ConversationModel*)

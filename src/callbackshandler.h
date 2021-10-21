/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                                  *
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
#include "api/datatransfer.h"
#include "qtwrapper/conversions_wrap.hpp"

#include <QObject>

#include <memory>

namespace lrc {

namespace api {
class Lrc;

namespace account {
enum class Status;
}
} // namespace api

class CallbacksHandler : public QObject
{
    Q_OBJECT

public:
    CallbacksHandler(const api::Lrc& parent);
    ~CallbacksHandler();

    // This connection relies on the behavior controller
    // and needs to be made after the lrc object is constructed
    void subscribeToDebugReceived();

Q_SIGNALS:
    /**
     * Connect this signal to get incoming text interaction from the DHT.
     * @param accountId interaction receiver.
     * @param msgId     interaction's id.
     * @param from      interaction sender.
     * @param payloads.
     */
    void newAccountMessage(const QString& accountId,
                           const QString& from,
                           const QString& msgId,
                           const MapStringString& payloads);
    /**
     * Connect this signal to get information when a peer is online.
     * @param accountId  related account.
     * @param contactUri the peer.
     * @param present if the peer is online.
     */
    void newBuddySubscription(const QString& accountId, const QString& contactUri, bool present);
    /**
     * Connect this signal to get information when peer discovery changes.
     * @param contactUri the peer.
     * @param state is 0 if the peer is added.
     */
    void newPeerSubscription(const QString& accountId,
                             const QString& contactUri,
                             int state,
                             const QString& displayname);
    /**
     * Connect this signal to know when a contact is removed by the daemon.
     * @param accountId the one who lost a contact.
     * @param contactUri the contact removed.
     * @param banned if the contact was banned
     */
    void contactRemoved(const QString& accountId, const QString& contactUri, bool banned);
    /**
     * Connect this signal to know when a contact is added by the daemon.
     * @param accountId the one who got a new contact.
     * @param contactUri the new contact.
     * @param confirmed if the contact is trusted.
     */
    void contactAdded(const QString& accountId, const QString& contactUri, bool confirmed);
    /**
     * Connect this signal to know when an incoming request is added by the daemon
     * @param accountId the one who got the request
     * @param contactUri the peer contact
     * @param payload the VCard
     */
    void incomingContactRequest(const QString& accountId,
                                const QString& conversationId,
                                const QString& contactUri,
                                const QString& payload);
    /**
     * Connect this signal to know when a call arrives
     * @param accountId the one who receives the call
     * @param callId the call id
     * @param fromUri the caller uri
     * @param displayName the display name of incoming call
     * @param mediaList media received
     */
    void incomingCallWithMedia(const QString& accountId,
                               const QString& callId,
                               const QString& fromUri,
                               const QString& displayName,
                               const VectorMapStringString& mediaList);
    /**
     * Connect this signal to know when a call arrives
     * @param accountId the one who receives the call
     * @param callId the call id
     * @param mediaList new media received
     */
    void mediaChangeRequested(const QString& accountId,
                              const QString& callId,
                              const VectorMapStringString& mediaList);
    /**
     * Connect this signal to know when a call is updated
     * @param accountId
     * @param callId the call id
     * @param state the new state
     * @param code
     */
    void callStateChanged(const QString& accountId,
                          const QString& callId,
                          const QString& state,
                          int code);
    /**
     * Connect this signal to know when a call medias are available
     * @param callId the call id
     * @param event
     * @param mediaList new mediaList for the call
     */
    void mediaNegotiationStatus(const QString& callId,
                                const QString& event,
                                const VectorMapStringString& mediaList);
    /**
     * Connect this signal to know when the account details have changed
     * @param accountId the one who changes
     * @param details the new details
     */
    void accountDetailsChanged(const QString& accountId, const MapStringString& details);
    /**
     * Connect this signal to know when the volatile account details have changed
     * @param accountId the one who changes
     * @param details the new details
     */
    void volatileAccountDetailsChanged(const QString& accountId, const MapStringString& details);
    /**
     * Connect this signal to know when the accounts list changed
     */
    void accountsChanged();
    /**
     * Connect this signal to know when the account status changed
     * @param accountId the one who changes
     * @param status the new status
     */
    void accountStatusChanged(const QString& accountId, const api::account::Status status);
    /**
     * Connect this signal to know where a VCard is incoming
     * @param callId the call linked to this VCard
     * @param from the sender URI
     * @param part the number of the part
     * @param numberOfParts of the VCard
     * @param payload content of the VCard
     */
    void incomingVCardChunk(const QString& callId,
                            const QString& from,
                            int part,
                            int numberOfParts,
                            const QString& payload);
    /**
     * Connect this signal to get incoming text interaction from SIP.
     * @param callId the call linked.
     * @param from interaction sender.
     * @param body the text received.
     */
    void incomingCallMessage(const QString& callId, const QString& from, const QString& body) const;
    /**
     * Connect this signal to know when a new conference is created
     * @param callId of the conference
     */
    void conferenceCreated(const QString& accountId, const QString& callId);
    /**
     * Connect this signal to know when a conference is removed
     * @param accountId
     * @param callId of the conference
     */
    void conferenceRemoved(const QString& accountId, const QString& callId);
    /**
     * Connect this signal to know when a message sent get a new status
     * @param accountId, account linked
     * @param messageId id of the message
     * @param conversationId id of the conversation
     * @param peer, peer uri
     * @param status, new status for this message
     */
    void accountMessageStatusChanged(const QString& accountId,
                                     const QString& conversationId,
                                     const QString& peer,
                                     const QString& messageId,
                                     int status);

    void transferStatusCreated(const QString& fileId, api::datatransfer::Info info);
    void transferStatusCanceled(const QString& fileId, api::datatransfer::Info info);
    void transferStatusAwaitingPeer(const QString& fileId, api::datatransfer::Info info);
    void transferStatusAwaitingHost(const QString& fileId, api::datatransfer::Info info);
    void transferStatusOngoing(const QString& fileId, api::datatransfer::Info info);
    void transferStatusFinished(const QString& fileId, api::datatransfer::Info info);
    void transferStatusError(const QString& fileId, api::datatransfer::Info info);
    void transferStatusTimeoutExpired(const QString& fileId, api::datatransfer::Info info);
    void transferStatusUnjoinable(const QString& fileId, api::datatransfer::Info info);

    /**
     * Connect this signal to get when a device name changed or a device is added
     * @param accountId interaction receiver.
     * @param devices A map of device IDs with corresponding labels.
     */
    void knownDevicesChanged(const QString& accountId, const MapStringString& devices);

    /**
     * Emit deviceRevocationEnded
     * @param accountId
     * @param deviceId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, UNKNOWN_DEVICE = 2
     */
    void deviceRevocationEnded(const QString& accountId, const QString& deviceId, const int status);

    /**
     * Account profile has been received
     * @param accountId
     * @param displayName
     * @param userPhoto
     */
    void accountProfileReceived(const QString& accountId,
                                const QString& displayName,
                                const QString& userPhoto);

    /**
     * Emit exportOnRingEnded
     * @param accountId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, NETWORK_ERROR = 2
     * @param pin
     */
    void exportOnRingEnded(const QString& accountId, int status, const QString& pin);

    /**
     * Name registration has ended
     * @param accountId
     * @param status
     * @param name
     */
    void nameRegistrationEnded(const QString& accountId, int status, const QString& name);

    /**
     * Name registration has been found
     * @param accountId
     * @param status
     * @param name
     */
    void registeredNameFound(const QString& accountId,
                             int status,
                             const QString& address,
                             const QString& name);

    /**
     * Migration ended
     * @param accountId
     * @param ok if migration succeed
     */
    void migrationEnded(const QString& accountId, bool ok);

    /**
     * Debug message received
     * @param message
     */
    void debugMessageReceived(const QString& message);

    /**
     * Renderer is started
     * @param id
     * @param shmrenderer
     * @param width
     * @param height
     */
    void startedDecoding(const QString& id, const QString& shmPath, int width, int height);

    /**
     * Renderer is stopped
     * @param id
     * @param shmrenderer
     */
    void stoppedDecoding(const QString& id, const QString& shmPath);

    /**
     * Emitted when a video device is plugged or unplugged
     */
    void deviceEvent();

    /**
     * Emitted when an audio level is plugged or unplugged
     */
    void audioDeviceEvent();

    /**
     * Emitted when an audio level is received
     * @param id of the ringbuffer level
     * @param level
     */
    void audioMeter(const QString& id, float level);

    /**
     * Emitted when an local recorder is finished
     * @param filePath
     */
    void recordPlaybackStopped(const QString& filePath);

    /**
     * Emitted when an audio level is received
     * @param accountId
     * @param newCount
     * @param oldCount
     * @param urgentCount
     */
    void voiceMailNotify(const QString& accountId, int newCount, int oldCount, int urgentCount);

    /**
     * Connect this signal to know when a call is updated
     * @param callId the call id
     * @param callId the contact id
     * @param state the new state
     * @param code
     */
    void remoteRecordingChanged(const QString& callId, const QString& peerNumber, bool state);
    void conversationLoaded(uint32_t requestId,
                            const QString& accountId,
                            const QString& conversationId,
                            const VectorMapStringString& messages);
    void messageReceived(const QString& accountId,
                         const QString& conversationId,
                         const MapStringString& message);
    void conversationRequestReceived(const QString& accountId,
                                     const QString& conversationId,
                                     const MapStringString& metadatas);
    void conversationRequestDeclined(const QString& accountId, const QString& conversationId);
    void conversationReady(const QString& accountId, const QString& conversationId);
    void conversationRemoved(const QString& accountId, const QString& conversationId);
    void conversationMemberEvent(const QString& accountId,
                                 const QString& conversationId,
                                 const QString& memberId,
                                 int event);

private Q_SLOTS:
    /**
     * Emit newAccountMessage
     * @param accountId
     * @param peerId
     * @param msgId
     * @param payloads of the interaction
     */
    void slotNewAccountMessage(const QString& accountId,
                               const QString& peerId,
                               const QString& msgId,
                               const QMap<QString, QString>& payloads);
    /**
     * Emit newBuddySubscription
     * @param accountId
     * @param contactUri
     * @param status if the contact is present
     * @param message unused for now
     */
    void slotNewBuddySubscription(const QString& accountId,
                                  const QString& contactUri,
                                  bool status,
                                  const QString& message);
    /**
     * Emit contactAdded
     * @param accountId account linked
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const QString& accountId, const QString& contactUri, bool confirmed);
    /**
     * Emit contactRemoved
     * @param accountId account linked
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const QString& accountId, const QString& contactUri, bool banned);
    /**
     * Emit incomingContactRequest
     * @param accountId the linked id
     * @param ringId the peer contact
     * @param payload the VCard
     * @param time when the request was received
     */
    void slotIncomingContactRequest(const QString& accountId,
                                    const QString& conversationId,
                                    const QString& ringId,
                                    const QByteArray& payload,
                                    time_t time);
    /**
     * Emit accountDetailsChanged
     * @param accountId
     * @param details
     */
    void slotAccountDetailsChanged(const QString& accountId, const MapStringString& details);
    /**
     * Emit volatileAccountDetailsChanged
     * @param accountId
     * @param details
     */
    void slotVolatileAccountDetailsChanged(const QString& accountId, const MapStringString& details);
    /**
     * Emit accountsChanged
     */
    void slotAccountsChanged();

    /**
     * Emit accountStatusChanged
     * @param accountId
     * @param registration_state
     * @param detail_code
     * @param detail_str
     */
    void slotRegistrationStateChanged(const QString& accountId,
                                      const QString& registration_state,
                                      unsigned detail_code,
                                      const QString& detail_str);
    /**
     * @deprecated Use slotIncomingCallWithMedia instead
     * Get the URI of the peer and emit incomingCall
     * @param accountId account linked
     * @param callId the incoming call id
     * @param fromUri the uri of the peer
     */
    void slotIncomingCall(const QString& accountId, const QString& callId, const QString& fromUri);
    /**
     * Get the URI of the peer and emit incomingCallWithMedia
     * @param accountId account linked
     * @param callId the incoming call id
     * @param fromUri the uri of the peer
     * @param mediaList the mediaList received
     */
    void slotIncomingCallWithMedia(const QString& accountId,
                                   const QString& callId,
                                   const QString& fromUri,
                                   const VectorMapStringString& mediaList);
    /**
     * Get the URI of the peer and emit mediaChangeRequested
     * @param accountId account linked
     * @param callId the incoming call id
     * @param mediaList the mediaList received
     */
    void slotMediaChangeRequested(const QString& accountId,
                                  const QString& callId,
                                  const VectorMapStringString& mediaList);
    /**
     * Emit callStateChanged
     * @param accountId
     * @param callId the call which changes.
     * @param state the new state
     * @param code unused for now
     */
    void slotCallStateChanged(const QString& accountId,
                              const QString& callId,
                              const QString& state,
                              int code);
    /**
     * Emit mediaNegotiationStatus
     * @param callId the call which changes.
     * @param eventstate the new state
     * @param mediaList new mediaList for the call
     */
    void slotMediaNegotiationStatus(const QString& callId,
                                    const QString& event,
                                    const VectorMapStringString& mediaList);
    /**
     * Parse a call message and emit incomingVCardChunk if it's a VCard chunk
     * else incomingCallMessage if it's a text message
     * @param callId call linked
     * @param from the URI
     * @param interaction the content of the Message.
     */
    void slotIncomingMessage(const QString& callId,
                             const QString& from,
                             const QMap<QString, QString>& interaction);
    /**
     * Emit conferenceCreated
     * @param callId of the conference
     */
    void slotConferenceCreated(const QString& callId);
    /**
     * Emit conferenceRemove
     * @param accountId
     * @param callId of the conference
     */
    void slotConferenceRemoved(const QString& accountId, const QString& callId);
    /**
     * Call slotCallStateChanged
     * @param accountId
     * @param callId of the conference
     * @param state, new state
     */
    void slotConferenceChanged(const QString& accountId,
                               const QString& callId,
                               const QString& state);
    /**
     * Emit accountMessageStatusChanged
     * @param accountId, account linked
     * @param messageId id of the message
     * @param conversationId id of the conversation
     * @param peer, peer uri
     * @param status, new status for this message
     */
    void slotAccountMessageStatusChanged(const QString& accountId,
                                         const QString& conversationId,
                                         const QString& peer,
                                         const QString& messageId,
                                         int status);

    void slotDataTransferEvent(const QString& accountId,
                               const QString& conversationId,
                               const QString& interactionId,
                               const QString& fileId,
                               uint code);

    /**
     * Emit knownDevicesChanged
     * @param accountId
     * @param devices A map of device IDs and corresponding labels
     */
    void slotKnownDevicesChanged(const QString& accountId, const MapStringString& devices);

    /**
     * Emit deviceRevocationEnded
     * @param accountId
     * @param deviceId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, UNKNOWN_DEVICE = 2
     */
    void slotDeviceRevokationEnded(const QString& accountId,
                                   const QString& deviceId,
                                   const int status);

    /**
     * Emit account avatar has been received
     * @param accountId
     * @param displayName
     * @param userPhoto
     */
    void slotAccountProfileReceived(const QString& accountId,
                                    const QString& displayName,
                                    const QString& userPhoto);

    /**
     * Emit exportOnRingEnded
     * @param accountId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, NETWORK_ERROR = 2
     * @param pin
     */
    void slotExportOnRingEnded(const QString& accountId, int status, const QString& pin);

    /**
     * Emit nameRegistrationEnded
     * @param accountId
     * @param status
     * @param name
     */
    void slotNameRegistrationEnded(const QString& accountId, int status, const QString& name);

    /**
     * Emit registeredNameFound
     * @param accountId
     * @param status
     * @param name
     */
    void slotRegisteredNameFound(const QString& accountId,
                                 int status,
                                 const QString& address,
                                 const QString& name);

    /**
     * emit migrationEnded
     * @param accountId
     * @param status
     */
    void slotMigrationEnded(const QString& accountId, const QString& status);

    /**
     * emit debugMessageReceived
     * @param message
     */
    void slotDebugMessageReceived(const QString& message);

    /**
     * Renderer is started
     * @param id
     * @param shmrenderer
     * @param width
     * @param height
     */
    void slotStartedDecoding(const QString& id, const QString& shmPath, int width, int height);

    /**
     * Renderer is stopped
     * @param id
     * @param shmrenderer
     */
    void slotStoppedDecoding(const QString& id, const QString& shmPath);

    /**
     * Detect when a video device is plugged or unplugged
     */
    void slotDeviceEvent();

    /**
     * Detect when an audio device is plugged or unplugged
     */
    void slotAudioDeviceEvent();

    /**
     * Called when an audio meter level is received
     * @param id of the ringbuffer level
     * @param level
     */
    void slotAudioMeterReceived(const QString& id, float level);

    /**
     * Emit newPeerSubscription
     * @param accountId
     * @param contactUri
     * @param status if the peer is added or removed
     * @param displayname is the account display name
     */
    void slotNearbyPeerSubscription(const QString& accountId,
                                    const QString& contactUri,
                                    int state,
                                    const QString& displayname);

    /**
     * Emit voiceMailNotify
     * @param accountId
     * @param new VM
     * @param old VM
     * @param new Urgent VM
     */
    void slotVoiceMailNotify(const QString& accountId, int newCount, int oldCount, int urgentCount);

    /**
     * Emit recordPlaybackStopped
     * @param filePath
     */
    void slotRecordPlaybackStopped(const QString& filePath);

    /**
     * Call slotCallStateChanged
     * @param callId of the conference
     * @param state, new state
     */
    void slotRemoteRecordingChanged(const QString& callId, const QString& contactId, bool state);
    void slotConversationLoaded(uint32_t requestId,
                                const QString& accountId,
                                const QString& conversationId,
                                const VectorMapStringString& messages);
    void slotMessageReceived(const QString& accountId,
                             const QString& conversationId,
                             const MapStringString& message);
    void slotConversationRequestReceived(const QString& accountId,
                                         const QString& conversationId,
                                         const MapStringString& metadatas);
    void slotConversationRequestDeclined(const QString& accountId, const QString& conversationId);
    void slotConversationReady(const QString& accountId, const QString& conversationId);
    void slotConversationRemoved(const QString& accountId, const QString& conversationId);
    void slotConversationMemberEvent(const QString& accountId,
                                     const QString& conversationId,
                                     const QString& memberId,
                                     int event);

private:
    const api::Lrc& parent;
};

} // namespace lrc

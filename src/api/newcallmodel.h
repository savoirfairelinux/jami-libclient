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

#include "api/behaviorcontroller.h"
#include "api/call.h"
#include "api/account.h"
#include "typedefs.h"

#include <QObject>

#include <memory>
#include <map>

namespace Video {
class Renderer;
}

namespace lrc {

class CallbacksHandler;
class NewCallModelPimpl;

namespace api {

namespace account {
struct Info;
}
namespace call {
struct Info;
}
class NewAccountModel;

/**
 *  @brief Class that manages call informations.
 */
class LIB_EXPORT NewCallModel : public QObject
{
    Q_OBJECT

public:
    using CallInfoMap = std::map<QString, std::shared_ptr<call::Info>>;

    const account::Info& owner;

    enum class Media { NONE, AUDIO, VIDEO };

    NewCallModel(const account::Info& owner,
                 const CallbacksHandler& callbacksHandler,
                 const BehaviorController& behaviorController);
    ~NewCallModel();

    /**
     * Create a new call with a contact
     * @param  uri of the contact to call
     * @param  isAudioOnly, set to false by default
     * @return the call uid created. Empty string is returned if call couldn't be created.
     */
    QString createCall(const QString& uri, bool isAudioOnly = false);

    /**
     * Get the call from its call id
     * @param  uid
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getCall(const QString& uid) const;

    /**
     * Get the call from the peer uri
     * @param  uri
     * @param  notOver search for a non finished call
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getCallFromURI(const QString& uri, bool notOver = false) const;

    /**
     * Get conference from a peer uri
     * @param  uri
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getConferenceFromURI(const QString& uri) const;

    /**
     * @param  callId to test
     * @return true if callId is presend else false.
     */
    bool hasCall(const QString& callId) const;

    /**
     * Send a text message to a SIP call
     * @param callId
     * @param body of the message
     */
    void sendSipMessage(const QString& callId, const QString& body) const;

    /**
     * Accept a call
     * @param callId
     */
    void accept(const QString& callId) const;

    /**
     * Hang up a call
     * @param callId
     */
    void hangUp(const QString& callId) const;

    /**
     * Refuse a call
     * @param callId
     */
    void refuse(const QString& callId) const;

    /**
     * Toggle audio record on a call
     * @param callId
     */
    void toggleAudioRecord(const QString& callId) const;

    /**
     * Play DTMF in a call
     * @param callId
     * @param value to play
     */
    void playDTMF(const QString& callId, const QString& value) const;

    /**
     * Toggle pause on a call.
     * @warn only use this function for SIP calls
     * @param callId
     */
    void togglePause(const QString& callId) const;

    /**
     * Toggle a media on a call
     * @param callId
     * @param media {AUDIO, VIDEO}
     */
    void toggleMedia(const QString& callId, const NewCallModel::Media media) const;

    /**
     * Not implemented yet
     */
    void setQuality(const QString& callId, const double quality) const;

    /**
     * Blind transfer. Directly transfer a call to a sip number
     * @param callId: the call to transfer
     * @param to: the sip number (for example: "sip:1412")
     */
    void transfer(const QString& callId, const QString& to) const;

    /**
     * Perform an attended. Transfer a call to another call
     * @param callIdSrc: the call to transfer
     * @param callIdDest: the destination's call
     */
    void transferToCall(const QString& callIdSrc, const QString& callIdDest) const;

    /**
     * Create a conference from 2 calls.
     * @param callIdA uid of the call A
     * @param callIdB uid of the call B
     */
    void joinCalls(const QString& callIdA, const QString& callIdB) const;

    /**
     * Call a participant and add it to a call
     * @param uri       URI of the participant
     * @param callId    Call receiving the participant
     * @param audioOnly If the call is audio only
     * @return id for a new call
     */
    QString callAndAddParticipant(const QString uri, const QString& callId, bool audioOnly);

    /**
     * Not implemented yet
     */
    void removeParticipant(const QString& callId, const QString& participant) const;

    /**
     * @param  callId
     * @return a human readable call duration (M:ss)
     */
    QString getFormattedCallDuration(const QString& callId) const;

    /**
     * Get if a call is recording
     * @param callId
     * @return true if the call is recording else false
     */
    bool isRecording(const QString& callId) const;

    /**
     * Close all active calls and conferences
     */
    static void hangupCallsAndConferences();

    /**
     * Extract Status Message From Status Map
     * @param statusCode
     * @return status message
     */
    static QString getSIPCallStatusString(const short& statusCode);

    /**
     * Set a call as the current call (hold other calls)
     */
    void setCurrentCall(const QString& callId) const;

    /**
     * Change the conference layout
     */
    void setConferenceLayout(const QString& confId, const call::Layout& layout);

    /**
     * Set the shown participant
     * @param confId        The call to change
     * @param participant   Use contact URI
     */
    void setActiveParticipant(const QString& confId, const QString& participant);

    /**
     * Check if a participant is a moderator or not
     * @param confId        The conference to check
     * @param uri           Uri of the participant to check (if empty, check current account)
     * @return if moderator
     */
    bool isModerator(const QString& confId, const QString& uri = "");

    /**
     * Set/unset a moderator
     * @param confId        The conference to change
     * @param peerId        Uri of the participant to change
     * @param state         State of the change (true set moderator / false unset moderator)
     */
    void setModerator(const QString& confId, const QString& peerId, const bool& state);

    /**
     * Mute/unmute participant
     * @param confId        The conference to change
     * @param peerId        Uri of the participant to mute
     * @param state         State of the change (true mute participant / false unmute participant)
     */
    void muteParticipant(const QString& confId, const QString& peerId, const bool& state);

    /**
     * Hangup participant
     * @param confId        The call to change
     * @param participant   Use contact URI
     */
    void hangupParticipant(const QString& confId, const QString& participant);

    /**
     * Check if a call is a conference or not
     * @param callId        The call to check
     * @return if conference
     */
    bool isConference(const QString& callId);

Q_SIGNALS:
    /**
     * Emitted when a call state changes
     * @param callId
     */
    void callStatusChanged(const QString& callId, int code) const;
    /**
     * Emitted when the rendered image changed
     * @param confId
     */
    void onParticipantsChanged(const QString& confId) const;
    /**
     * Emitted when a call starts
     * @param callId
     */
    void callStarted(const QString& callId) const;
    /**
     * Emitted when a call is over
     * @param callId
     */
    void callEnded(const QString& callId) const;
    /**
     * Emitted when a call is incoming
     * @param callId
     * @param fromId the peer uri
     * @param displayname
     */
    void newIncomingCall(const QString& fromId,
                         const QString& callId,
                         const QString& displayname) const;
    /**
     * Emitted when a call is added to a conference
     * @param callId
     * @param confId
     */
    void callAddedToConference(const QString& callId, const QString& confId) const;

    /**
     * Emitted when a voice mail notice arrives
     * @param accountId
     * @param newCount
     * @param oldCount
     * @param urgentCount
     */
    void voiceMailNotify(const QString& accountId,
                         int newCount,
                         int oldCount,
                         int urgentCount) const;

    /**
     * Listen from CallbacksHandler when the peer start recording
     * @param callId
     * @param contactId
     * @param peerName
     * @param state the new state
     */
    void remoteRecordingChanged(const QString& callId,
                                const QSet<QString>& peerRec,
                                bool state) const;

private:
    std::unique_ptr<NewCallModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::NewCallModel*)

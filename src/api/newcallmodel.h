/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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

// Std
#include <memory>
#include <string>
#include <map>

// Qt
#include <qobject.h>

// Data
#include "api/call.h"
#include "api/account.h"

// LRC
#include "typedefs.h"

namespace Video {
class Renderer;
}

namespace lrc
{

class CallbacksHandler;
class NewCallModelPimpl;

namespace api
{

namespace account { struct Info; }
namespace call { struct Info; }
class NewAccountModel;

/**
  *  @brief Class that manages call informations.
  */
class LIB_EXPORT NewCallModel : public QObject {
    Q_OBJECT

public:
    using CallInfoMap = std::map<std::string, std::shared_ptr<call::Info>>;

    const account::Info& owner;

    enum class Media {
        NONE,
        AUDIO,
        VIDEO
    };

    NewCallModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewCallModel();

    /**
     * Create a new call with a contact
     * @param  url of the contact to call
     * @param  isAudioOnly, set to false by default
     * @return the call uid created. Empty string is returned if call couldn't be created.
     */
    std::string createCall(const std::string& url, bool isAudioOnly = false);
    /**
     * Get the call from its call id
     * @param  uid
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getCall(const std::string& uid) const;
    /**
     * Get the call from the peer uri
     * @param  uri
     * @param  notOver search for a non finished call
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getCallFromURI(const std::string& uri, bool notOver = false) const;
    /**
     * Get conference from a peer uri
     * @param  uri
     * @return the callInfo
     * @throw out_of_range exception if not found
     */
    const call::Info& getConferenceFromURI(const std::string& uri) const;
    /**
     * @param  callId to test
     * @return true if callId is presend else false.
     */
    bool hasCall(const std::string& callId) const;
    /**
     * Send a text message to a SIP call
     * @param callId
     * @param body of the message
     */
    void sendSipMessage(const std::string& callId, const std::string& body) const;

    /**
     * Accept a call
     * @param callId
     */
    void accept(const std::string& callId) const;
    /**
     * Hang up a call
     * @param callId
     */
    void hangUp(const std::string& callId) const;
    /**
     * Toggle audio record on a call
     * @param callId
     */
    void toggleAudioRecord(const std::string& callId) const;
    /**
     * Play DTMF in a call
     * @param callId
     * @param value to play
     */
    void playDTMF(const std::string& callId, const std::string& value) const;
    /**
     * Toggle pause on a call
     * @param callId
     */
    void togglePause(const std::string& callId) const;
    /**
     * Toggle a media on a call
     * @param callId
     * @param media {AUDIO, VIDEO}
     */
    void toggleMedia(const std::string& callId, const NewCallModel::Media media) const;
    /**
     * Not implemented yet
     */
    void setQuality(const std::string& callId, const double quality) const;
    /**
     * Blind transfer. Directly transfer a call to a sip number
     * @param callId: the call to transfer
     * @param to: the sip number (for example: "sip:1412")
     */
    void transfer(const std::string& callId, const std::string& to) const;
    /**
     * Perform an attended. Transfer a call to another call
     * @param callIdSrc: the call to transfer
     * @param callIdDest: the destination's call
     */
    void transferToCall(const std::string& callIdSrc, const std::string& callIdDest) const;
    /**
     * Create a conference from 2 calls.
     * @param callIdA uid of the call A
     * @param callIdB uid of the call B
     */
    void joinCalls(const std::string& callIdA, const std::string& callIdB) const;
    /**
     * Not implemented yet
     */
    void removeParticipant(const std::string& callId, const std::string& participant) const;
    /**
     * @param  callId
     * @return the renderer linked to a call
     */
    Video::Renderer* getRenderer(const std::string& callId) const;
    /**
     * @param  callId
     * @return a human readable call duration (M:ss)
     */
    std::string getFormattedCallDuration(const std::string& callId) const;

    /**
     * Get if a call is recording
     * @param callId
     * @return true if the call is recording else false
     */
    bool isRecording(const std::string& callId) const;

Q_SIGNALS:
    /**
     * Emitted when a call state changes
     * @param callId
     */
    void callStatusChanged(const std::string& callId) const;
    /**
     * Emitted when a call starts
     * @param callId
     */
    void callStarted(const std::string& callId) const;
    /**
     * Emitted when a call is over
     * @param callId
     */
    void callEnded(const std::string& callId) const;
    /**
     * Emitted when a call is incoming
     * @param callId
     * @param fromId the peer uri
     */
    void newIncomingCall(const std::string& callId, const std::string& fromId) const;
    /**
     * Emitted when the renderer starts
     * @param callId
     * @param renderer
     */
    void remotePreviewStarted(const std::string& callId, Video::Renderer* renderer) const;
    /**
     * Emitted when a call is added to a conference
     * @param callId
     * @param confId
     */
    void callAddedToConference(const std::string& callId, const std::string& confId) const;

private:
    std::unique_ptr<NewCallModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc

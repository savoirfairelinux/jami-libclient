/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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
#include <string>

// Lrc
#include "smartlistitem.h"
#include "contact.h"
#include "contactmethod.h"

namespace Video {
class Renderer;
}

class LIB_EXPORT ContactItem : public SmartListItem {
    Q_OBJECT

    public:
    explicit ContactItem(ContactMethod* contact);
    ContactItem();
    ~ContactItem();

    const std::string getTitle() const override;
    const std::string getAvatar() const override;
    const std::string getLastInteraction() const override;
    long int getLastInteractionTimeStamp() const;
    const std::string getUri() const; // pensez à éventuellement retirer ces fonctions et accéder à l'uri depui getContact
    bool isPresent() const override;
    virtual void activate() override;
    const Contact& getContact() {return contact_;};
    void setContact(const Contact&);

    // message communication
    void sendMessage(std::string message); // manage only dht message for now
    void removeHistory();
    Messages getHistory() const;
    void newMessageAdded(Message msg);

    // video communication (maybe move all or some stuff to a call object)
    void placeCall();
    void setCallId(const std::string);
    void setCallStatus(const CallStatus);
    const std::string getCallId() const { return callId_; };
    const CallStatus getCallStatus() {return callStatus_;};
    static std::string getReadableCallStatus(CallStatus); // changer le parametre pour un ContactItem...
    void rejectIncomingCall() const;
    void acceptIncomingCall() const;
    void cancelOutGoingCall() const;
    Video::Renderer* getRenderer() const; // should be in some other place... ?? e.g. in the coming call object.
    // video call controllers :
    void hangUp() const;
    void togglePause();
    void toggleMuteaUdio();
    void toggleMuteVideo();
    void toggleRecoringdAudio();
    void qualityController() const;

    Q_SIGNALS:
    void CallStatusChanged(const CallStatus);

    public Q_SLOTS:
    void slotPresenceChanged(bool);

protected:
    Contact contact_;
    std::string callId_ = "";
    CallStatus callStatus_ = CallStatus::NONE;

private:
    bool isPaused = false;
    bool isAudioMuted = false;
    bool isVideoMuted = false;
    bool isRecordingAudio = false;
};

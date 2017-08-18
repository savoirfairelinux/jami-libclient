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



// Parent
#include "contactitem.h"

// Lrc
#include "smartlistmodel.h"
#include "availableaccountmodel.h"
#include "video/renderer.h"
#include "private/videorenderermanager.h"
#include "database.h"

// Qt
#include <qstring.h>
#include <QDateTime>

// Debug
#include <qdebug.h>

// Dbus bind for ring daemon
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"

// Ring daemon
#include <media_const.h>

ContactItem::ContactItem(ContactMethod* cm)
: SmartListItem()
{
    this->contact_.uri = cm->uri().toStdString();
    this->contact_.avatar = DataBase::instance().getAvatar(QString(this->contact_.uri.c_str()));
    this->contact_.id = cm->bestId().toStdString();
    this->contact_.registeredName = cm->registeredName().toStdString();
    this->contact_.displayName = cm->bestName().toStdString();
    this->contact_.isPresent = cm->isPresent();
    this->contact_.unreadMessages = 0;

    QObject::connect(cm, &ContactMethod::presentChanged, this, &ContactItem::slotPresenceChanged);
}

ContactItem::ContactItem()
: SmartListItem()
{
}

void
ContactItem::setContact(const Contact& contact)
{
    contact_ = contact;
    emit changed(this);
}

ContactItem::~ContactItem()
{
}

void
ContactItem::setTitle(const std::string title)
{
    if (not _title)
        _title = std::unique_ptr<std::string>(new std::string());

    _title->assign(title);
    emit changed(this);
}

const std::string
ContactItem::getTitle() const
{
    return _title->data();
}

// peut etre serait-il préférable de juste retourner un état et de laisser les clients déterminer s'ils veulent
// lancer un signal... Attention, si on fait ça cela va complexifier les clients, car l'ui qui doit changer en
// fonction d'un tel signal doit pouvoir le connecter... ce n'est pas évident que la semartlist soit visible de
// chat view par exemple...
void
ContactItem::activate()
{
    switch(callStatus_)
    {
        case CallStatus::INCOMING_RINGING :
        case CallStatus::OUTGOING_RINGING :
        case CallStatus::CONNECTING :
        case CallStatus::SEARCHING :
        emit SmartListModel::instance().ShowIncomingCallView(this);
        break;
        case CallStatus::IN_PROGRESS :
        emit SmartListModel::instance().showVideoViewFor(this);
        break;
        case CallStatus::NONE :
        default :
        emit SmartListModel::instance().showConversationView(this);
    }
}

const std::string
ContactItem::getAlias() const
{
    return this->contact_.displayName;
}

const std::string
ContactItem::getAvatar() const
{
    return this->contact_.avatar;
}

void
ContactItem::sendMessage(std::string message)
{
    // il faudera traiter les cas messages durant appels et par dht.

    QMap<QString, QString> payloads;
    payloads["text/plain"] = message.c_str();

    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    auto id = ConfigurationManager::instance().sendTextMessage(account->id(), contact_.uri.c_str(), payloads);

    DataBase::instance().addMessage(
        contact_.uri.c_str(),
        account->id(),
        message.c_str(),
        QString::number(QDateTime::currentMSecsSinceEpoch() / 1000),
        true
    );

    emit lastInteractionChanged(this);
}

void
ContactItem::removeHistory()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;
    DataBase::instance().removeHistory(QString(contact_.id.c_str()), QString(account->id()));
}

Messages
ContactItem::getHistory() const
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return Messages();
    return DataBase::instance().getMessages(QString(contact_.id.c_str()), QString(account->id()));
}

void
ContactItem::placeCall()
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (not account) {
        qDebug() << "placeCall, invalid pointer";
        return;
    }

    auto uri = "ring:" + contact_.uri;

    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = CallManager::instance().placeCall(account->id(), uri.c_str());

    if (callId.isEmpty())
        qDebug() << "no call placed between (account :" << account->id() << ", contact :" << uri.c_str() << ")";

    setCallId(callId.toStdString());
    setCallStatus(CallStatus::SEARCHING);

    emit lastInteractionChanged(this);
    activate();
}

const std::string
ContactItem::getLastInteraction() const
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (not account) {
        qDebug() << "getLastInteraction, invalid pointer";
        return std::string();
    }

    auto messages = DataBase::instance().getMessages(contact_.uri.c_str(), account->id());

    if (messages.size() == 0)
        return std::string();

    return messages.back().body;
}

const long int
ContactItem::getLastInteractionTimeStamp() const
{
    auto account = AvailableAccountModel::instance().currentDefaultAccount();

    if (not account) {
        qDebug() << "getLastInteractionTimeStamp, invalid pointer";
        return 0;
    }

    auto messages = DataBase::instance().getMessages(contact_.uri.c_str(), account->id());

    if (messages.size() == 0)
        return 0;

    return std::stol(messages.back().timestamp);
}

const std::string
ContactItem::getUri() const
{
    return contact_.uri;
}

void
ContactItem::setCallId(const std::string callId)
{
    callId_ = callId;
    emit changed(this);
}

const bool
ContactItem::isPresent() const
{
    return contact_.isPresent;
}


void
ContactItem::slotPresenceChanged(bool presence)
{
    this->contact_.isPresent = presence;
    // TODO emit changed();
}

void
ContactItem::setCallStatus(const CallStatus callStatus)
{
    callStatus_ = callStatus;

    if (callStatus == CallStatus::INCOMING_RINGING)
        qDebug() << "CallStatus::INCOMING_RINGING";

    if (callStatus == CallStatus::IN_PROGRESS)
        qDebug() << "CallStatus::IN_PROGRESS";

    if (callStatus == CallStatus::ENDED)
        qDebug() << "CallStatus::ENDED";

    if (callStatus == CallStatus::OUTGOING_RINGING)
        qDebug() << "CallStatus::OUTGOING_RINGING";

    if (callStatus == CallStatus::SEARCHING)
        qDebug() << "CallStatus::SEARCHING";

    if (callStatus == CallStatus::PAUSED)
        qDebug() << "CallStatus::PAUSED";

    if (callStatus == CallStatus::PEER_PAUSED)
        qDebug() << "CallStatus::PEER_PAUSED";

    if (callStatus == CallStatus::CONNECTING)
        qDebug() << "CallStatus::CONNECTING";

    emit CallStatusChanged(callStatus_);

    emit changed(this);
}

std::string
ContactItem::getReadableCallStatus(CallStatus callStatus)
{
    switch (callStatus) {
        case CallStatus::INCOMING_RINGING :
        return "ringing";
        case CallStatus::ENDED :
        return "call ended";
        case CallStatus::OUTGOING_RINGING :
        return "ringing";
        case CallStatus::SEARCHING:
        return "searching";
        case CallStatus::IN_PROGRESS :
        return "in progress";
        default :
        return "plouf!";
    }
}

void
ContactItem::rejectIncomingCall() const
{
    CallManager::instance().hangUp(callId_.c_str());
}

void
ContactItem::acceptIncomingCall() const
{
    CallManager::instance().accept(callId_.c_str());
}

void
ContactItem::cancelOutGoingCall() const
{
    CallManager::instance().hangUp(callId_.c_str());
}

Video::Renderer*
ContactItem::getRenderer() const
{
   #ifdef ENABLE_VIDEO
   return VideoRendererManager::instance().getRenderer(callId_);
   #else
   return nullptr;
   #endif
}

void
ContactItem::hangUp() const
{
    CallManager::instance().hangUp(callId_.c_str());
}

void
ContactItem::togglePause()
{
    if (isPaused)
        CallManager::instance().unhold(callId_.c_str());
    else
        CallManager::instance().hold(callId_.c_str());

    isPaused = !isPaused; // should be done by some callback from daemon if possible
}

void
ContactItem::toggleMuteaUdio()
{
    isAudioMuted = !isAudioMuted;
    CallManager::instance().muteLocalMedia(callId_.c_str(), DRing::Media::Details::MEDIA_TYPE_AUDIO, isAudioMuted /* mute state */ );
}

void
ContactItem::toggleMuteVideo()
{
    isVideoMuted = !isVideoMuted;
    CallManager::instance().muteLocalMedia(callId_.c_str(), DRing::Media::Details::MEDIA_TYPE_VIDEO, isVideoMuted /* mute state */ );
}

void
ContactItem::toggleRecoringdAudio()
{
    isRecordingAudio = !isRecordingAudio;
    CallManager::instance().toggleRecording(callId_.c_str());
}

void
ContactItem::qualityController() const
{
    qDebug() << "qualityController, isn't yet implemented";
}

void
ContactItem::newMessageAdded(Message msg)
{
    emit newMessage(msg);
    emit lastInteractionChanged(this);
}

#include <contactitem.moc>

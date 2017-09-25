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

#include "api/newcallmodel.h"

// std
#include <chrono>

// Lrc
#include "callbackshandler.h"
#include "api/conversationmodel.h"
#include "api/newaccountmodel.h"
#include "dbus/callmanager.h"
#include "private/videorenderermanager.h"
#include "video/renderer.h"

// Ring daemon
#include <media_const.h>

// Qt
#include <QObject>
#include <QString>

namespace lrc
{

using namespace api;

class NewCallModelPimpl: public QObject
{
public:
    NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewCallModelPimpl();

    void sendMessage(const std::string& callId, const std::string& body) const;

    NewCallModel::CallInfoMap calls;
    const CallbacksHandler& callbacksHandler;
    const NewCallModel& linked;

public Q_SLOTS:
    void slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId);
    void slotCallStateChanged(const std::string& callId, const std::string &state, int code);
    void slotRemotePreviewStarted(const std::string& callId, Video::Renderer* renderer);
};

NewCallModel::NewCallModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<NewCallModelPimpl>(*this, callbacksHandler))
{

}

NewCallModel::~NewCallModel()
{
}

const call::Info&
NewCallModel::getCallFromURI(const std::string& uri) const
{
    for (const auto& call: pimpl_->calls) {
        if (call.second->peer == uri) {
            return *call.second;
        }
    }
    throw std::out_of_range("No call at URI " + uri);
}

const call::Info&
NewCallModel::getCall(const std::string& uid) const
{
    if (pimpl_->calls.find(uid) == pimpl_->calls.end()) {
        throw std::out_of_range("No call at UID " + uid);
    }
    return *pimpl_->calls[uid];
}

std::string
NewCallModel::humanReadableStatus(const call::Status& status)
{
    switch(status)
    {
    case call::Status::PAUSED:
        return tr("Hold").toStdString();
    case call::Status::IN_PROGRESS:
        return tr("Talking").toStdString();
    case call::Status::INVALID:
        return tr("ERROR").toStdString();
    case call::Status::OUTGOING_REQUESTED:
        return tr("Outgoing requested").toStdString();
    case call::Status::INCOMING_RINGING:
        return tr("Incoming").toStdString();
    case call::Status::OUTGOING_RINGING:
        return tr("Calling").toStdString();
    case call::Status::CONNECTING:
        return tr("Connecting").toStdString();
    case call::Status::SEARCHING:
        return tr("Searching").toStdString();
    case call::Status::PEER_PAUSED:
        return tr("Hold").toStdString();
    case call::Status::INACTIVE:
        return tr("Inactive").toStdString();
    case call::Status::ENDED:
        return tr("Finished").toStdString();
    case call::Status::TERMINATING:
        return tr("Finished").toStdString();
    case call::Status::CONNECTED:
        return tr("Communication established").toStdString();
    case call::Status::AUTO_ANSWERING:
        return tr("Auto answering").toStdString();
        break;
    }
}

const std::string
NewCallModel::createCall(const std::string& url)
{
    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = CallManager::instance().placeCall(owner.id.c_str(), url.c_str());

    if (callId.isEmpty())
        qDebug() << "no call placed between (account :" << owner.id.c_str() << ", contact :" << url.c_str() << ")";

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId.toStdString();
    callInfo->peer = url;
    callInfo->status =  call::Status::SEARCHING;
    pimpl_->calls[callId.toStdString()] = callInfo;

    return callId.toStdString();
}

void
NewCallModel::accept(const std::string& callId) const
{
    CallManager::instance().accept(callId.c_str());
}


void
NewCallModel::hangUp(const std::string& callId) const
{
    CallManager::instance().hangUp(callId.c_str());
}

void
NewCallModel::togglePause(const std::string& callId) const
{
    auto call = pimpl_->calls.find(callId);
    if (call == pimpl_->calls.end()) return;
    switch(pimpl_->calls[callId]->status)
    {
    case call::Status::PAUSED:
        CallManager::instance().unhold(callId.c_str());
        break;
    case call::Status::IN_PROGRESS:
        CallManager::instance().hold(callId.c_str());
        break;
    case call::Status::INVALID:
    case call::Status::OUTGOING_REQUESTED:
    case call::Status::INCOMING_RINGING:
    case call::Status::OUTGOING_RINGING:
    case call::Status::CONNECTING:
    case call::Status::SEARCHING:
    case call::Status::PEER_PAUSED:
    case call::Status::INACTIVE:
    case call::Status::ENDED:
    case call::Status::TERMINATING:
    case call::Status::CONNECTED:
    case call::Status::AUTO_ANSWERING:
        break;
    }
}

void
NewCallModel::toggleMedia(const std::string& callId, const NewCallModel::Media media, bool flag) const
{
    switch(media)
    {
    case NewCallModel::Media::AUDIO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_AUDIO,
                                               flag);
        break;

    case NewCallModel::Media::VIDEO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_VIDEO,
                                               flag);
        break;

    case NewCallModel::Media::NONE:
    default:
        break;
    }
}

void
NewCallModel::toggleRecoringdAudio(const std::string& callId) const
{
    CallManager::instance().toggleRecording(callId.c_str());
}

void
NewCallModel::setQuality(const std::string& callId, const double quality) const
{
    qDebug() << "setQuality, isn't yet implemented";
}

void
NewCallModel::transfer(const std::string& callId, const std::string& to) const
{
    qDebug() << "transfer, isn't yet implemented";
}

void
NewCallModel::addParticipant(const std::string& callId, const std::string& participant)
{

}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant)
{

}

Video::Renderer*
NewCallModel::getRenderer(const std::string& callId) const
{
   #ifdef ENABLE_VIDEO
   return VideoRendererManager::instance().getRenderer(callId);
   #else
   return nullptr;
   #endif
}

std::string
NewCallModel::getFormattedCallDuration(const std::string& callId) const
{
    if (pimpl_->calls.find(callId) == pimpl_->calls.end()) return "00:00";
    auto& startTime = pimpl_->calls[callId]->startTime;
    if (startTime == 0) return "00:00";
    auto now = std::chrono::system_clock::now();
    auto difference = std::chrono::system_clock::to_time_t(now) - startTime;
    auto formattedString = std::string("");
    if (difference / 60 != 0) {
        formattedString += std::to_string(difference/60) + ":";
        if (formattedString.length() == 2) {
            formattedString = "0" + formattedString;
        }
    } else {
        formattedString += "00:";
    }
    if (difference % 60 < 10) formattedString += "0";
    formattedString += std::to_string(difference % 60);
    return formattedString;
}


NewCallModelPimpl::NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewCallModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged, this, &NewCallModelPimpl::slotCallStateChanged);
    connect(&VideoRendererManager::instance(), &VideoRendererManager::remotePreviewStarted, this, &NewCallModelPimpl::slotRemotePreviewStarted);
}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModelPimpl::sendMessage(const std::string& callId, const std::string& body) const
{
    // cette fonction doit etre appellé depuis le sendmessage de conversation
    // la fonction sendmessage de conversation doit etre friend
    //
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    CallManager::instance().sendTextMessage(callId.c_str(), payloads, true /* not used */);

    // l'ajout du message dans la db doit se faire depuis conversatoin
}

void
NewCallModelPimpl::slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId)
{
    if (linked.owner.id != accountId) return;

    qDebug() << "NewCallModelPimpl::slotIncomingCall";

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId;
    callInfo->peer = fromId;
    callInfo->status =  call::Status::INCOMING_RINGING;
    calls[callId] = callInfo;

    emit linked.newIncomingCall(fromId, callId);
}

void
NewCallModelPimpl::slotCallStateChanged(const std::string& callId, const std::string& state, int code)
{
    if (calls.find(callId) != calls.end()) {
        if (state == "CONNECTING") {
            calls[callId]->status = call::Status::CONNECTING;
        } else if (state == "RINGING") {
            calls[callId]->status = call::Status::OUTGOING_RINGING;
        } else if (state == "HUNGUP") {
            calls[callId]->status = call::Status::TERMINATING;
        } else if (state == "OVER") {
            emit linked.callEnded(callId);
            calls[callId]->status = call::Status::ENDED;
        } else if (state == "INACTIVE") {
            calls[callId]->status = call::Status::INACTIVE;
        } else if (state == "CURRENT") {
            if (calls[callId]->startTime == 0) {
                auto now = std::chrono::system_clock::now();
                calls[callId]->startTime = std::chrono::system_clock::to_time_t(now);
                emit linked.callStarted(callId);
            }
            calls[callId]->status = call::Status::IN_PROGRESS;
        } else if (state == "HOLD") {
            calls[callId]->status = call::Status::PAUSED;
        }
        qDebug() << "slotCallStateChanged, call:" << callId.c_str() << " - state: " << state.c_str();
        emit linked.callStatusChanged(callId);
    }
}

void
NewCallModelPimpl::slotRemotePreviewStarted(const std::string& callId, Video::Renderer* renderer)
{
    emit linked.remotePreviewStarted(callId, renderer);
}

} // namespace lrc

#include "api/moc_newcallmodel.cpp"

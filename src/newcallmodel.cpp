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
#include "api/contact.h"
#include "api/contactmodel.h"
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

    NewCallModel::CallInfoMap calls;
    const CallbacksHandler& callbacksHandler;
    const NewCallModel& linked;

    /**
     * key = peer's uri
     * vector = chunks
     * @note chunks are counted from 1 to number of parts. We use 0 to store the actual number of parts stored
     */
    std::map<std::string, std::vector<std::string>> vcardsChunks;

public Q_SLOTS:
    /**
     * Listen from CallbacksHandler when a call is incoming
     * @param accountId account which receives the call
     * @param callId
     * @param fromId peer uri
     */
    void slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId);
    /**
     * Listen from CallbacksHandler when a call got a new state
     * @param callId
     * @param state the new state
     * @param code unused
     */
    void slotCallStateChanged(const std::string& callId, const std::string &state, int code);
    /**
     * Listen from VideoRendererManager when a Renderer starts
     * @param callId
     * @param renderer
     */
    void slotRemotePreviewStarted(const std::string& callId, Video::Renderer* renderer);
    /**
     * Listen from CallbacksHandler when a VCard chunk is incoming
     * @param callId
     * @param from
     * @param part
     * @param numberOfParts
     * @param payload
     */
    void slotincomingVCardChunk(const std::string& callId, const std::string& from, int part, int numberOfParts, const std::string& payload);
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
    return *pimpl_->calls.at(uid);
}

std::string
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
    pimpl_->calls.emplace(callId.toStdString(), std::move(callInfo));

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
NewCallModel::toggleAudioRecord(const std::string& callId) const
{
    CallManager::instance().toggleRecording(callId.c_str());
}

void
NewCallModel::playDTMF(const std::string& callId, const std::string& value) const
{
    auto call = pimpl_->calls.find(callId);
    if (call == pimpl_->calls.end()) return;
    if (pimpl_->calls[callId]->status != call::Status::IN_PROGRESS) return;
    CallManager::instance().playDTMF(value.c_str());
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
NewCallModel::addParticipant(const std::string& callId, const std::string& participant) const
{

}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant) const
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
    if (startTime.time_since_epoch().count() == 0) return "00:00";
    auto now = std::chrono::steady_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::seconds>(
             now.time_since_epoch() - startTime.time_since_epoch()).count();

    auto formattedString = std::string("");
    auto minutes = d / 60;
    auto seconds = d % 60;
    if (minutes > 0) {
        formattedString += std::to_string(minutes) + ":";
        if (formattedString.length() == 2) {
            formattedString = "0" + formattedString;
        }
    } else {
        formattedString += "00:";
    }
    if (seconds < 10) formattedString += "0";
    formattedString += std::to_string(seconds);
    return formattedString;
}


NewCallModelPimpl::NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    // TODO init call list and conferences if client crash but not the daemon in call.
    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewCallModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged, this, &NewCallModelPimpl::slotCallStateChanged);
    connect(&VideoRendererManager::instance(), &VideoRendererManager::remotePreviewStarted, this, &NewCallModelPimpl::slotRemotePreviewStarted);
    connect(&callbacksHandler, &CallbacksHandler::incomingVCardChunk, this, &NewCallModelPimpl::slotincomingVCardChunk);
}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModel::sendSipMessage(const std::string& callId, const std::string& body) const
{
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    CallManager::instance().sendTextMessage(callId.c_str(), payloads, true /* not used */);
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
    calls.emplace(callId, std::move(callInfo));

    emit linked.newIncomingCall(fromId, callId);
}

void
NewCallModelPimpl::slotCallStateChanged(const std::string& callId, const std::string& state, int code)
{
    Q_UNUSED(code)
    if (calls.find(callId) != calls.end()) {
        if (state == "CONNECTING") {
            calls[callId]->status = call::Status::CONNECTING;
        } else if (state == "RINGING") {
            calls[callId]->status = call::Status::OUTGOING_RINGING;
        } else if (state == "HUNGUP") {
            calls[callId]->status = call::Status::TERMINATING;
        } else if (state == "FAILURE" || state == "OVER") {
            if (calls[callId]->startTime.time_since_epoch().count() != 0) {
                emit linked.callEnded(callId);
            }
            calls[callId]->status = call::Status::ENDED;
        } else if (state == "INACTIVE") {
            calls[callId]->status = call::Status::INACTIVE;
        } else if (state == "CURRENT") {
            if (calls[callId]->startTime.time_since_epoch().count() == 0) {
                calls[callId]->startTime = std::chrono::steady_clock::now();
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

void
NewCallModelPimpl::slotincomingVCardChunk(const std::string& callId,
                                          const std::string& from,
                                          int part,
                                          int numberOfParts,
                                          const std::string& payload)
{
    auto it = calls.find(callId);

    if (it != calls.end()) {
        auto it_2 = vcardsChunks.find(from);
        if (it_2 != vcardsChunks.end()) {
            vcardsChunks[from][part-1] = payload;

            if ( not std::any_of(vcardsChunks[from].begin(), vcardsChunks[from].end(),
                [](const auto& s) { return s.empty(); }) ) {

                profile::Info profileInfo;
                profileInfo.uri = from;
                profileInfo.type = profile::Type::RING;

                std::string vcardPhoto;

                for (auto& chunk : vcardsChunks[from])
                    vcardPhoto += chunk;

                for (auto& e : QString(vcardPhoto.c_str()).split( "\n" ))
                    if (e.contains("PHOTO"))
                        profileInfo.avatar = e.split( ":" )[1].toStdString();
                    else if (e.contains("FN"))
                        profileInfo.alias = e.split( ":" )[1].toStdString();

                contact::Info contactInfo;
                contactInfo.profileInfo = profileInfo;

                linked.owner.contactModel->addContact(contactInfo);
                vcardsChunks.erase(from); // Transfer is finish, we don't want to reuse this entry.
            }
        } else {
            vcardsChunks[from] = std::vector<std::string>(numberOfParts);
            vcardsChunks[from][part-1] = payload;
        }

    }
}

bool
NewCallModel::hasCall(const std::string& callId)
{
    return pimpl_->calls.find(callId) != pimpl_->calls.end();
}

} // namespace lrc

#include "api/moc_newcallmodel.cpp"

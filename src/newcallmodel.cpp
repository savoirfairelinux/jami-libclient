/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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
#include <random>

// Lrc
#include "callbackshandler.h"
#include "api/conversationmodel.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/newaccountmodel.h"
#include "dbus/callmanager.h"
#include "mime.h"
#include "private/videorenderermanager.h"
#include "video/renderer.h"

// Ring daemon
#include <media_const.h>

// Qt
#include <QObject>
#include <QString>

static std::uniform_int_distribution<int> dis{ 0, std::numeric_limits<int>::max() };

namespace lrc
{

using namespace api;

class NewCallModelPimpl: public QObject
{
public:
    NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewCallModelPimpl();

    /**
     * Send the profile VCard into a call
     * @param callId
     */
    void sendProfile(const std::string& callId);

    NewCallModel::CallInfoMap calls;
    const CallbacksHandler& callbacksHandler;
    const NewCallModel& linked;

    /**
     * key = peer's uri
     * vector = chunks
     * @note chunks are counted from 1 to number of parts. We use 0 to store the actual number of parts stored
     */
    std::map<std::string, std::vector<std::string>> vcardsChunks;

    /**
     * Retrieve active calls from the daemon and init the model
     */
    void initCallFromDaemon();
    /**
     * Retrieve active conferences from the daemon and init the model
     */
    void initConferencesFromDaemon();
    /**
     * Translate a given daemon call state to an internal LRC state, taking previous state in account.
     */
    call::Status translateDaemonCallState(const call::Status& currentState, const std::string& state) const;
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
    /**
     * Listen from CallbacksHandler when a conference is created.
     * @param callId
     */
    void slotConferenceCreated(const std::string& callId);
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
NewCallModel::getCallFromURI(const std::string& uri, bool notOver) const
{
    // peer url = ring:uri or sip number
    auto url = (owner.profileInfo.type != profile::Type::SIP && uri.find("ring:") == std::string::npos) ? "ring:" + uri : uri;
    for (const auto& call: pimpl_->calls) {
        if (call.second->peer == url) {
            if (!notOver || call.second->status != call::Status::ENDED)
                return *call.second;
        }
    }
    throw std::out_of_range("No call at URI " + uri);
}

const call::Info&
NewCallModel::getConferenceFromURI(const std::string& uri) const
{
    for (const auto& call: pimpl_->calls) {
        if (call.second->type == call::Type::CONFERENCE) {
            QStringList callList = CallManager::instance().getParticipantList(call.first.c_str());
            foreach(const auto& callId, callList) {
                if (pimpl_->calls[callId.toStdString()]->peer == uri) {
                    return *call.second;
                }
            }
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
NewCallModel::createCall(const std::string& url, bool isAudioOnly)
{
#ifdef ENABLE_LIBWRAP
    auto callId = isAudioOnly ? CallManager::instance().placeCall(owner.id.c_str(),
                                                                  url.c_str(),
                                                                  {{"AUDIO_ONLY", "true"}})
                                  : CallManager::instance().placeCall(owner.id.c_str(), url.c_str());
#else // dbus
    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = isAudioOnly ? CallManager::instance().placeCallWithDetails(owner.id.c_str(),
                                                                                url.c_str(),
                                                                                {{"AUDIO_ONLY", "true"}})
                                 : CallManager::instance().placeCall(owner.id.c_str(), url.c_str());
#endif // ENABLE_LIBWRAP

    if (callId.isEmpty()) {
        qDebug() << "no call placed between (account :" << owner.id.c_str() << ", contact :" << url.c_str() << ")";
        return "";
    }

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId.toStdString();
    callInfo->peer = url;
    callInfo->isOutgoing = true;
    callInfo->status =  call::Status::SEARCHING;
    callInfo->type =  call::Type::DIALOG;
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
    if (!hasCall(callId)) return;
    auto& call = pimpl_->calls[callId];
    switch(call->type)
    {
    case call::Type::DIALOG:
        CallManager::instance().hangUp(callId.c_str());
        break;
    case call::Type::CONFERENCE:
        CallManager::instance().hangUpConference(callId.c_str());
        break;
    case call::Type::INVALID:
    default:
        break;
    }
}

void
NewCallModel::toggleAudioRecord(const std::string& callId) const
{
    CallManager::instance().toggleRecording(callId.c_str());
}

void
NewCallModel::playDTMF(const std::string& callId, const std::string& value) const
{
    if (!hasCall(callId)) return;
    if (pimpl_->calls[callId]->status != call::Status::IN_PROGRESS) return;
    CallManager::instance().playDTMF(value.c_str());
}

void
NewCallModel::togglePause(const std::string& callId) const
{
    if (!hasCall(callId)) return;
    auto& call = pimpl_->calls[callId];

    if (call->status == call::Status::PAUSED) {
        if (call->type == call::Type::DIALOG)
            CallManager::instance().unhold(callId.c_str());
        else {
            CallManager::instance().unholdConference(callId.c_str());
        }
    } else if (call->status == call::Status::IN_PROGRESS) {
        if (call->type == call::Type::DIALOG)
            CallManager::instance().hold(callId.c_str());
        else {
            CallManager::instance().holdConference(callId.c_str());
        }
    }
}

void
NewCallModel::toggleMedia(const std::string& callId, const NewCallModel::Media media) const
{
    if (!hasCall(callId)) return;
    auto& call = pimpl_->calls[callId];
    switch(media)
    {
    case NewCallModel::Media::AUDIO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_AUDIO,
                                               !call->audioMuted);
        call->audioMuted = !call->audioMuted;
        break;

    case NewCallModel::Media::VIDEO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_VIDEO,
                                               !call->videoMuted);
        call->videoMuted = !call->videoMuted;
        break;

    case NewCallModel::Media::NONE:
    default:
        break;
    }
}

void
NewCallModel::setQuality(const std::string& callId, const double quality) const
{
    Q_UNUSED(callId)
    Q_UNUSED(quality)
    qDebug() << "setQuality isn't implemented yet";
}

void
NewCallModel::transfer(const std::string& callId, const std::string& to) const
{
    CallManager::instance().transfer(callId.c_str(), to.c_str());
}

void
NewCallModel::transferToCall(const std::string& callId, const std::string& callIdDest) const
{
    CallManager::instance().attendedTransfer(callId.c_str(), callIdDest.c_str());
}

void
NewCallModel::joinCalls(const std::string& callIdA, const std::string& callIdB) const
{
    if (!hasCall(callIdA) || !hasCall(callIdB)) return;

    auto& call1 = pimpl_->calls[callIdA];
    auto& call2 = pimpl_->calls[callIdB];
    if (call1->type == call::Type::CONFERENCE && call2->type == call::Type::CONFERENCE)
        CallManager::instance().joinConference(callIdA.c_str(), callIdB.c_str());
    else if (call1->type == call::Type::CONFERENCE)
        CallManager::instance().addParticipant(callIdB.c_str(), callIdA.c_str());
    else if (call2->type == call::Type::CONFERENCE)
        CallManager::instance().addParticipant(callIdA.c_str(), callIdB.c_str());
    else
        CallManager::instance().joinParticipant(callIdA.c_str(), callIdB.c_str());
}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant) const
{
    Q_UNUSED(callId)
    Q_UNUSED(participant)
    qDebug() << "removeParticipant() isn't implemented yet";
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
    if (!hasCall(callId)) return "00:00";
    auto& startTime = pimpl_->calls[callId]->startTime;
    if (startTime.time_since_epoch().count() == 0) return "00:00";
    auto now = std::chrono::steady_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::seconds>(
             now.time_since_epoch() - startTime.time_since_epoch()).count();

    std::string formattedString;
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

bool
NewCallModel::isRecording(const std::string& callId) const
{
    if (!hasCall(callId)) return false;
    return CallManager::instance().getIsRecording(callId.c_str());
}


NewCallModelPimpl::NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewCallModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged, this, &NewCallModelPimpl::slotCallStateChanged);
    connect(&VideoRendererManager::instance(), &VideoRendererManager::remotePreviewStarted, this, &NewCallModelPimpl::slotRemotePreviewStarted);
    connect(&callbacksHandler, &CallbacksHandler::incomingVCardChunk, this, &NewCallModelPimpl::slotincomingVCardChunk);
    connect(&callbacksHandler, &CallbacksHandler::conferenceCreated, this , &NewCallModelPimpl::slotConferenceCreated);

#ifndef ENABLE_LIBWRAP
    // Only necessary with dbus since the daemon runs separately
    initCallFromDaemon();
    initConferencesFromDaemon();
#endif
}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModelPimpl::initCallFromDaemon()
{
    QStringList callList = CallManager::instance().getCallList();
    for (const auto& callId : callList)
    {
        MapStringString details = CallManager::instance().getCallDetails(callId);
        auto accountId = details["ACCOUNTID"].toStdString();
        if (accountId == linked.owner.id) {
            auto callInfo = std::make_shared<call::Info>();
            callInfo->id = callId.toStdString();
            auto now = std::chrono::steady_clock::now();
            auto system_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto diff = static_cast<int64_t>(system_now) - std::stol(details["TIMESTAMP_START"].toStdString());
            callInfo->startTime = now - std::chrono::seconds(diff);
            callInfo->status = translateDaemonCallState(call::Status::INVALID, details["CALL_STATE"].toStdString());
            auto endId = details["PEER_NUMBER"].indexOf("@");
            callInfo->peer = details["PEER_NUMBER"].left(endId).toStdString();
            if (linked.owner.profileInfo.type == lrc::api::profile::Type::RING) {
                callInfo->peer = "ring:" + callInfo->peer;
            }
            callInfo->videoMuted = details["VIDEO_MUTED"] == "true";
            callInfo->audioMuted = details["AUDIO_MUTED"] == "true";
            callInfo->type = call::Type::DIALOG;
            calls.emplace(callId.toStdString(), std::move(callInfo));
            // NOTE/BUG: the videorenderer can't know that the client has restarted
            // So, for now, a user will have to manually restart the medias until
            // this renderer is not redesigned.
        }
    }
}

void
NewCallModelPimpl::initConferencesFromDaemon()
{
    QStringList callList = CallManager::instance().getConferenceList();
    for (const auto& callId : callList)
    {
        QMap<QString, QString> details = CallManager::instance().getConferenceDetails(callId);
        auto callInfo = std::make_shared<call::Info>();
        callInfo->id = callId.toStdString();
        QStringList callList = CallManager::instance().getParticipantList(callId);
        auto isForThisAccount = true;
        foreach(const auto& call, callList) {
            MapStringString callDetails = CallManager::instance().getCallDetails(call);
            isForThisAccount = callDetails["ACCOUNTID"].toStdString() == linked.owner.id;
            if (!isForThisAccount) break;
            auto now = std::chrono::steady_clock::now();
            auto system_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto diff = static_cast<int64_t>(system_now) - std::stol(callDetails["TIMESTAMP_START"].toStdString());
            callInfo->status =  details["CONF_STATE"] == "ACTIVE_ATTACHED"? call::Status::IN_PROGRESS : call::Status::PAUSED;
            callInfo->startTime = now - std::chrono::seconds(diff);
            emit linked.callAddedToConference(call.toStdString(), callId.toStdString());
        }
        if (!isForThisAccount) break;
        callInfo->type = call::Type::CONFERENCE;
        calls.emplace(callId.toStdString(), std::move(callInfo));
    }
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

    // do not use auto here (QDBusPendingReply<MapStringString>)
    MapStringString callDetails = CallManager::instance().getCallDetails(callId.c_str());

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId;
    // peer url = ring:uri or sip number
    auto url = (linked.owner.profileInfo.type != profile::Type::SIP && fromId.find("ring:") == std::string::npos) ? "ring:" + fromId : fromId;
    callInfo->peer = url;
    callInfo->isOutgoing = false;
    callInfo->status =  call::Status::INCOMING_RINGING;
    callInfo->type =  call::Type::DIALOG;
    callInfo->isAudioOnly = callDetails["AUDIO_ONLY"] == "true" ? true : false;
    calls.emplace(callId, std::move(callInfo));

    emit linked.newIncomingCall(fromId, callId);
}

call::Status
NewCallModelPimpl::translateDaemonCallState(const call::Status& currentState, const std::string& status) const
{
    if (status == "INCOMING")
        return call::Status::INCOMING_RINGING;
    else if (status == "CONNECTING")
        return call::Status::CONNECTING;
    else if (status == "RINGING")
        return call::Status::OUTGOING_RINGING;
    else if (status == "HUNGUP")
        return call::Status::TERMINATING;
    else if (status == "HOLD" || status == "ACTIVE_DETACHED")
        return call::Status::PAUSED;
    else if (status == "UNHOLD" || status == "CURRENT" || status == "ACTIVE_ATTACHED")
        return call::Status::IN_PROGRESS;
    else if (status == "INACTIVE")
        return call::Status::INACTIVE;
    else if (status == "BUSY")
        return call::Status::PEER_BUSY;
    else if (status == "OVER" || status == "FAILURE") {
        if (currentState == call::Status::PEER_BUSY)
            return call::Status::ENDED_RECORDING_MESSAGE;
        else
            return call::Status::ENDED;
    }

    return call::Status::INVALID;
}

void
NewCallModelPimpl::slotCallStateChanged(const std::string& callId, const std::string& state, int code)
{
    Q_UNUSED(code)
    if (!linked.hasCall(callId)) return;

    auto& call = calls[callId];
    auto previousStatus = call->status;
    auto status = translateDaemonCallState(previousStatus, state);
    call->status = status;

    if (previousStatus == call->status) {
        // call state didn't change, simply ignore signal
        return;
    }

    if (call->status == call::Status::ENDED || call->status == call::Status::ENDED_RECORDING_MESSAGE) {
        emit linked.callEnded(callId);
    } else if (call->status == call::Status::IN_PROGRESS) {
        if (previousStatus == call::Status::INCOMING_RINGING
                || previousStatus == call::Status::OUTGOING_RINGING) {
            call->startTime = std::chrono::steady_clock::now();
            emit linked.callStarted(callId);
            sendProfile(callId);
        }
    }

    qDebug("slotCallStateChanged (call: %s), from %s to %s", callId.c_str(),
         call::to_string(previousStatus), call::to_string(status));
    emit linked.callStatusChanged(callId);
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
    if (!linked.hasCall(callId)) return;

    auto it = vcardsChunks.find(from);
    if (it != vcardsChunks.end()) {
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

bool
NewCallModel::hasCall(const std::string& callId) const
{
    return pimpl_->calls.find(callId) != pimpl_->calls.end();
}

void
NewCallModelPimpl::slotConferenceCreated(const std::string& confId)
{
    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = confId;
    callInfo->status =  call::Status::IN_PROGRESS;
    callInfo->type =  call::Type::CONFERENCE;
    callInfo->startTime = std::chrono::steady_clock::now();
    calls[confId] = callInfo;
    QStringList callList = CallManager::instance().getParticipantList(confId.c_str());
    foreach(const auto& call, callList) {
        emit linked.callAddedToConference(call.toStdString(), confId);
    }
}

void
NewCallModelPimpl::sendProfile(const std::string& callId)
{
    auto vCard = linked.owner.accountModel->accountVCard(linked.owner.id);

    std::random_device rdev;
    auto key = std::to_string(dis(rdev));

    int i = 0;
    int total = vCard.size()/1000 + (vCard.size()%1000?1:0);
    while (vCard.size()) {
        auto sizeLimit = std::min(1000, static_cast<int>(vCard.size()));
        MapStringString chunk;
        chunk[QString("%1; id=%2,part=%3,of=%4")
               .arg( RingMimes::PROFILE_VCF     )
               .arg( key.c_str()                )
               .arg( QString::number( i+1   )   )
               .arg( QString::number( total )   )
            ] = vCard.substr(0, sizeLimit).c_str();
        vCard = vCard.substr(sizeLimit);
        ++i;
        CallManager::instance().sendTextMessage(callId.c_str(), chunk, false);
    }
}

} // namespace lrc

#include "api/moc_newcallmodel.cpp"

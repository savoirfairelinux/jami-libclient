/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                             *
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
#include "authority/storagehelper.h"
#include "dbus/callmanager.h"
#include "vcard.h"
#include "video/renderer.h"

// Ring daemon
#include <media_const.h>

// Qt
#include <QObject>
#include <QString>

static std::uniform_int_distribution<int> dis{ 0, std::numeric_limits<int>::max() };
static const std::map<short, std::string> sip_call_status_code_map {
    {0, QObject::tr("Null").toStdString()},
    {100, QObject::tr("Trying").toStdString()},
    {180, QObject::tr("Ringing").toStdString()},
    {181, QObject::tr("Being Forwarded").toStdString()},
    {182, QObject::tr("Queued").toStdString()},
    {183, QObject::tr("Progress").toStdString()},
    {200, QObject::tr("OK").toStdString()},
    {202, QObject::tr("Accepted").toStdString()},
    {300, QObject::tr("Multiple Choices").toStdString()},
    {301, QObject::tr("Moved Permanently").toStdString()},
    {302, QObject::tr("Moved Temporarily").toStdString()},
    {305, QObject::tr("Use Proxy").toStdString()},
    {380, QObject::tr("Alternative Service").toStdString()},
    {400, QObject::tr("Bad Request").toStdString()},
    {401, QObject::tr("Unauthorized").toStdString()},
    {402, QObject::tr("Payment Required").toStdString()},
    {403, QObject::tr("Forbidden").toStdString()},
    {404, QObject::tr("Not Found").toStdString()},
    {405, QObject::tr("Method Not Allowed").toStdString()},
    {406, QObject::tr("Not Acceptable").toStdString()},
    {407, QObject::tr("Proxy Authentication Required").toStdString()},
    {408, QObject::tr("Request Timeout").toStdString()},
    {410, QObject::tr("Gone").toStdString()},
    {413, QObject::tr("Request Entity Too Large").toStdString()},
    {414, QObject::tr("Request URI Too Long").toStdString()},
    {415, QObject::tr("Unsupported Media Type").toStdString()},
    {416, QObject::tr("Unsupported URI Scheme").toStdString()},
    {420, QObject::tr("Bad Extension").toStdString()},
    {421, QObject::tr("Extension Required").toStdString()},
    {422, QObject::tr("Session Timer Too Small").toStdString()},
    {423, QObject::tr("Interval Too Brief").toStdString()},
    {480, QObject::tr("Temporarily Unavailable").toStdString()},
    {481, QObject::tr("Call TSX Does Not Exist").toStdString()},
    {482, QObject::tr("Loop Detected").toStdString()},
    {483, QObject::tr("Too Many Hops").toStdString()},
    {484, QObject::tr("Address Incomplete").toStdString()},
    {485, QObject::tr("Ambiguous").toStdString()},
    {486, QObject::tr("Busy").toStdString()},
    {487, QObject::tr("Request Terminated").toStdString()},
    {488, QObject::tr("Not Acceptable").toStdString()},
    {489, QObject::tr("Bad Event").toStdString()},
    {490, QObject::tr("Request Updated").toStdString()},
    {491, QObject::tr("Request Pending").toStdString()},
    {493, QObject::tr("Undecipherable").toStdString()},
    {500, QObject::tr("Internal Server Error").toStdString()},
    {501, QObject::tr("Not Implemented").toStdString()},
    {502, QObject::tr("Bad Gateway").toStdString()},
    {503, QObject::tr("Service Unavailable").toStdString()},
    {504, QObject::tr("Server Timeout").toStdString()},
    {505, QObject::tr("Version Not Supported").toStdString()},
    {513, QObject::tr("Message Too Large").toStdString()},
    {580, QObject::tr("Precondition Failure").toStdString()},
    {600, QObject::tr("Busy Everywhere").toStdString()} ,
    {603, QObject::tr("Call Refused").toStdString()},
    {604, QObject::tr("Does Not Exist Anywhere").toStdString()},
    {606, QObject::tr("Not Acceptable Anywhere").toStdString()}
};

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
        if (call.second->peerUri == url) {
            if (!notOver || !call::isTerminating(call.second->status))
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
                if (pimpl_->calls[callId.toStdString()]->peerUri == uri) {
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
NewCallModel::createCall(const std::string& uri, bool isAudioOnly)
{
#ifdef ENABLE_LIBWRAP
    auto callId = isAudioOnly ? CallManager::instance().placeCall(owner.id.c_str(),
                                                                  uri.c_str(),
                                                                  {{"AUDIO_ONLY", "true"}})
                                  : CallManager::instance().placeCall(owner.id.c_str(), uri.c_str());
#else // dbus
    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = isAudioOnly ? CallManager::instance().placeCallWithDetails(owner.id.c_str(),
                                                                                uri.c_str(),
                                                                                {{"AUDIO_ONLY", "true"}})
                                 : CallManager::instance().placeCall(owner.id.c_str(), uri.c_str());
#endif // ENABLE_LIBWRAP

    if (callId.isEmpty()) {
        qDebug() << "no call placed between (account: " << owner.id.c_str() << ", contact: " << uri.c_str() << ")";
        return "";
    }

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId.toStdString();
    callInfo->peerUri = uri;
    callInfo->isOutgoing = true;
    callInfo->status =  call::Status::SEARCHING;
    callInfo->type =  call::Type::DIALOG;
    callInfo->isAudioOnly = isAudioOnly;
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
NewCallModel::refuse(const std::string& callId) const
{
    if (!hasCall(callId)) return;
    CallManager::instance().refuse(callId.c_str());
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
    call::Info call1, call2;
    bool hasCall1 = false, hasCall2 = false;
    for (const auto &account_id : owner.accountModel->getAccountList()) {
        try {
            auto &accountInfo = owner.accountModel->getAccountInfo(account_id);
            if (accountInfo.callModel->hasCall(callIdA)) {
                call1 = accountInfo.callModel->getCall(callIdA);
                hasCall1 = true;
            }
            if (accountInfo.callModel->hasCall(callIdB)) {
                call1 = accountInfo.callModel->getCall(callIdB);
                hasCall2 = true;
            }
            if (hasCall1 && hasCall2) break;
        } catch (...) {}
    }
    if (!hasCall1 || !hasCall2) {
        qWarning() << "Can't join inexistent calls.";
        return;
    }

    if (call1.type == call::Type::CONFERENCE && call2.type == call::Type::CONFERENCE) {
        qWarning() << "JOIN CONF";
        CallManager::instance().joinConference(callIdA.c_str(), callIdB.c_str());
    }
    else if (call1.type == call::Type::CONFERENCE) {
        CallManager::instance().addParticipant(callIdB.c_str(), callIdA.c_str());
        qWarning() << "ADD TO CONF";
        
    }
    else if (call2.type == call::Type::CONFERENCE) {

        CallManager::instance().addParticipant(callIdA.c_str(), callIdB.c_str());
        qWarning() << "ADD TO CONF2 ";
    }
    else {
        qWarning() << "JOIN PART";
        CallManager::instance().joinParticipant(callIdA.c_str(), callIdB.c_str());
    }
}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant) const
{
    Q_UNUSED(callId)
    Q_UNUSED(participant)
    qDebug() << "removeParticipant() isn't implemented yet";
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
    return authority::storage::getFormattedCallDuration(d);
}

bool
NewCallModel::isRecording(const std::string& callId) const
{
    if (!hasCall(callId)) return false;
    return CallManager::instance().getIsRecording(callId.c_str());
}

std::string
NewCallModel::getSIPCallStatusString(const short& statusCode)
{
    auto element = sip_call_status_code_map.find(statusCode);
    if(element != sip_call_status_code_map.end()){
        return element->second;
    }
    return "";
}

NewCallModelPimpl::NewCallModelPimpl(const NewCallModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    connect(&callbacksHandler, &CallbacksHandler::incomingCall, this, &NewCallModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged, this, &NewCallModelPimpl::slotCallStateChanged);
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
            callInfo->status = call::to_status(details["CALL_STATE"].toStdString());
            auto endId = details["PEER_NUMBER"].indexOf("@");
            callInfo->peerUri = details["PEER_NUMBER"].left(endId).toStdString();
            if (linked.owner.profileInfo.type == lrc::api::profile::Type::RING) {
                callInfo->peerUri = "ring:" + callInfo->peerUri;
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
NewCallModel::hangupCallsAndConferences()
{
    QStringList conferences = CallManager::instance().getConferenceList();
    for (const auto& conf : conferences) {
        CallManager::instance().hangUpConference(conf);
    }
    QStringList calls = CallManager::instance().getCallList();
    for (const auto &call : calls) {
        CallManager::instance().hangUp(call);
    }
}

void
NewCallModelPimpl::slotIncomingCall(const std::string& accountId, const std::string& callId, const std::string& fromId)
{
    if (linked.owner.id != accountId) {
        return;
    }

    // do not use auto here (QDBusPendingReply<MapStringString>)
    MapStringString callDetails = CallManager::instance().getCallDetails(callId.c_str());

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId;
    // peer uri = ring:<jami_id> or sip number
    auto uri = (linked.owner.profileInfo.type != profile::Type::SIP && fromId.find("ring:") == std::string::npos) ? "ring:" + fromId : fromId;
    callInfo->peerUri = uri;
    callInfo->isOutgoing = false;
    callInfo->status =  call::Status::INCOMING_RINGING;
    callInfo->type =  call::Type::DIALOG;
    callInfo->isAudioOnly = callDetails["AUDIO_ONLY"] == "true" ? true : false;
    calls.emplace(callId, std::move(callInfo));

    emit linked.newIncomingCall(fromId, callId);

    // HACK. BECAUSE THE DAEMON DOESN'T HANDLE THIS CASE!
    if (linked.owner.confProperties.autoAnswer) {
        linked.accept(callId);
    }
}

void
NewCallModelPimpl::slotCallStateChanged(const std::string& callId, const std::string& state, int code)
{
    if (!linked.hasCall(callId)) return;

    auto status = call::to_status(state);
    auto& call = calls[callId];

    if (status == call::Status::ENDED && !call::isTerminating(call->status)) {
        call->status = call::Status::TERMINATING;
        emit linked.callStatusChanged(callId, code);
    }

    // proper state transition
    auto previousStatus = call->status;
    call->status = status;

    if (previousStatus == call->status) {
        // call state didn't change, simply ignore signal
        return;
    }

    qDebug("slotCallStateChanged (call: %s), from %s to %s", callId.c_str(),
         call::to_string(previousStatus).c_str(), call::to_string(status).c_str());

    // NOTE: signal emission order matters, always emit CallStatusChanged before CallEnded
    emit linked.callStatusChanged(callId, code);

    if (call->status == call::Status::ENDED) {
        emit linked.callEnded(callId);
    } else if (call->status == call::Status::IN_PROGRESS) {
        if (previousStatus == call::Status::INCOMING_RINGING
                || previousStatus == call::Status::OUTGOING_RINGING) {
            call->startTime = std::chrono::steady_clock::now();
            emit linked.callStarted(callId);
            sendProfile(callId);
        }
    }
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
               .arg( lrc::vCard::PROFILE_VCF     )
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

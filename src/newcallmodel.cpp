/****************************************************************************
 *    Copyright (C) 2017-2022 Savoir-faire Linux Inc.                       *
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

// Lrc
#include "callbackshandler.h"
#include "api/avmodel.h"
#include "api/behaviorcontroller.h"
#include "api/conversationmodel.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/pluginmodel.h"
#include "api/lrc.h"
#include "api/newaccountmodel.h"
#include "authority/storagehelper.h"
#include "dbus/callmanager.h"
#include "dbus/videomanager.h"
#include "vcard.h"
#include "video/renderer.h"
#include "typedefs.h"
#include "uri.h"

// Ring daemon
#include <media_const.h>
#include <account_const.h>

// Qt
#include <QObject>
#include <QString>
#include <QUrl>

// std
#include <chrono>
#include <random>
#include <map>

using namespace DRing::Media;

static std::uniform_int_distribution<int> dis {0, std::numeric_limits<int>::max()};
static const std::map<short, QString>
    sip_call_status_code_map {{0, QObject::tr("Null")},
                              {100, QObject::tr("Trying")},
                              {180, QObject::tr("Ringing")},
                              {181, QObject::tr("Being Forwarded")},
                              {182, QObject::tr("Queued")},
                              {183, QObject::tr("Progress")},
                              {200, QObject::tr("OK")},
                              {202, QObject::tr("Accepted")},
                              {300, QObject::tr("Multiple Choices")},
                              {301, QObject::tr("Moved Permanently")},
                              {302, QObject::tr("Moved Temporarily")},
                              {305, QObject::tr("Use Proxy")},
                              {380, QObject::tr("Alternative Service")},
                              {400, QObject::tr("Bad Request")},
                              {401, QObject::tr("Unauthorized")},
                              {402, QObject::tr("Payment Required")},
                              {403, QObject::tr("Forbidden")},
                              {404, QObject::tr("Not Found")},
                              {405, QObject::tr("Method Not Allowed")},
                              {406, QObject::tr("Not Acceptable")},
                              {407, QObject::tr("Proxy Authentication Required")},
                              {408, QObject::tr("Request Timeout")},
                              {410, QObject::tr("Gone")},
                              {413, QObject::tr("Request Entity Too Large")},
                              {414, QObject::tr("Request URI Too Long")},
                              {415, QObject::tr("Unsupported Media Type")},
                              {416, QObject::tr("Unsupported URI Scheme")},
                              {420, QObject::tr("Bad Extension")},
                              {421, QObject::tr("Extension Required")},
                              {422, QObject::tr("Session Timer Too Small")},
                              {423, QObject::tr("Interval Too Brief")},
                              {480, QObject::tr("Temporarily Unavailable")},
                              {481, QObject::tr("Call TSX Does Not Exist")},
                              {482, QObject::tr("Loop Detected")},
                              {483, QObject::tr("Too Many Hops")},
                              {484, QObject::tr("Address Incomplete")},
                              {485, QObject::tr("Ambiguous")},
                              {486, QObject::tr("Busy")},
                              {487, QObject::tr("Request Terminated")},
                              {488, QObject::tr("Not Acceptable")},
                              {489, QObject::tr("Bad Event")},
                              {490, QObject::tr("Request Updated")},
                              {491, QObject::tr("Request Pending")},
                              {493, QObject::tr("Undecipherable")},
                              {500, QObject::tr("Internal Server Error")},
                              {501, QObject::tr("Not Implemented")},
                              {502, QObject::tr("Bad Gateway")},
                              {503, QObject::tr("Service Unavailable")},
                              {504, QObject::tr("Server Timeout")},
                              {505, QObject::tr("Version Not Supported")},
                              {513, QObject::tr("Message Too Large")},
                              {580, QObject::tr("Precondition Failure")},
                              {600, QObject::tr("Busy Everywhere")},
                              {603, QObject::tr("Call Refused")},
                              {604, QObject::tr("Does Not Exist Anywhere")},
                              {606, QObject::tr("Not Acceptable Anywhere")}};

namespace lrc {

using namespace api;

class NewCallModelPimpl : public QObject
{
public:
    NewCallModelPimpl(const NewCallModel& linked,
                      Lrc& lrc,
                      const CallbacksHandler& callbacksHandler,
                      const BehaviorController& behaviorController);
    ~NewCallModelPimpl();

    /**
     * Send the profile VCard into a call
     * @param callId
     */
    void sendProfile(const QString& callId);

    NewCallModel::CallInfoMap calls;
    const CallbacksHandler& callbacksHandler;
    const NewCallModel& linked;
    const BehaviorController& behaviorController;

    /**
     * key = peer's uri
     * vector = chunks
     * @note chunks are counted from 1 to number of parts. We use 0 to store the actual number of
     * parts stored
     */
    std::map<QString, VectorString> vcardsChunks;

    /**
     * Retrieve active calls from the daemon and init the model
     */
    void initCallFromDaemon();

    /**
     * Retrieve active conferences from the daemon and init the model
     */
    void initConferencesFromDaemon();

    /**
     * Check if media device is muted
     */
    bool checkMediaDeviceMuted(const MapStringString& mediaAttributes);

    bool manageCurrentCall_ {true};
    QString currentCall_ {};

    Lrc& lrc;

    QList<call::PendingConferenceeInfo> pendingConferencees_;

public Q_SLOTS:
    /**
     * Listen from CallbacksHandler when a call is incoming
     * @param accountId account which receives the call
     * @param callId
     * @param fromId peer uri
     * @param displayname
     * @param mediaList media received
     */
    void slotIncomingCallWithMedia(const QString& accountId,
                                   const QString& callId,
                                   const QString& fromId,
                                   const QString& displayname,
                                   const VectorMapStringString& mediaList);
    /**
     * Connect this signal to know when a call arrives
     * @param accountId the one who receives the call
     * @param callId the call id
     * @param mediaList new media received
     */
    void slotMediaChangeRequested(const QString& accountId,
                                  const QString& callId,
                                  const VectorMapStringString& mediaList);
    /**
     * Listen from CallbacksHandler when a call got a new state
     * @param accountId
     * @param callId
     * @param state the new state
     * @param code unused
     */
    void slotCallStateChanged(const QString& accountId,
                              const QString& callId,
                              const QString& state,
                              int code);
    /**
     * Listen from CallbacksHandler when a call medias are ready
     * @param callId
     * @param event
     * @param mediaList
     */
    void slotMediaNegotiationStatus(const QString& callId,
                                    const QString& event,
                                    const VectorMapStringString& mediaList);
    /**
     * Listen from CallbacksHandler when a VCard chunk is incoming
     * @param accountId
     * @param callId
     * @param from
     * @param part
     * @param numberOfParts
     * @param payload
     */
    void slotincomingVCardChunk(const QString& accountId,
                                const QString& callId,
                                const QString& from,
                                int part,
                                int numberOfParts,
                                const QString& payload);
    /**
     * Listen from CallbacksHandler when a conference is created.
     * @param callId
     */
    void slotConferenceCreated(const QString& accountId, const QString& callId);
    /**
     * Listen from CallbacksHandler when a voice mail notice is incoming
     * @param accountId
     * @param newCount
     * @param oldCount
     * @param urgentCount
     */
    void slotVoiceMailNotify(const QString& accountId, int newCount, int oldCount, int urgentCount);
    /**
     * Listen from CallManager when a conference layout is updated
     * @param confId
     * @param infos
     */
    void slotOnConferenceInfosUpdated(const QString& confId, const VectorMapStringString& infos);
    /**
     * Listen from CallbacksHandler when the peer start recording
     * @param callId
     * @param state the new state
     */
    void remoteRecordingChanged(const QString& callId, const QString& peerNumber, bool state);
    /**
     * Listen from CallbacksHandler when a renderer starts
     * @param id
     * @param shmPath
     * @param width
     * @param height
     */
    void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
};

NewCallModel::NewCallModel(const account::Info& owner,
                           Lrc& lrc,
                           const CallbacksHandler& callbacksHandler,
                           const BehaviorController& behaviorController)
    : QObject(nullptr)
    , owner(owner)
    , pimpl_(std::make_unique<NewCallModelPimpl>(*this, lrc, callbacksHandler, behaviorController))
{}

NewCallModel::~NewCallModel() {}

const call::Info&
NewCallModel::getCallFromURI(const QString& uri, bool notOver) const
{
    // For a NON SIP account the scheme can be ring:. Sometimes it can miss, and will be certainly
    // replaced by jami://.
    // Just make the comparaison ignoring the scheme and check the rest.
    auto uriObj = URI(uri);
    for (const auto& call : pimpl_->calls) {
        auto contactUri = URI(call.second->peerUri);
        if (uriObj.userinfo() == contactUri.userinfo()
            and uriObj.hostname() == contactUri.hostname()) {
            if (!notOver || !call::isTerminating(call.second->status))
                return *call.second;
        }
    }
    throw std::out_of_range("No call at URI " + uri.toStdString());
}

const call::Info&
NewCallModel::getConferenceFromURI(const QString& uri) const
{
    for (const auto& call : pimpl_->calls) {
        if (call.second->type == call::Type::CONFERENCE) {
            QStringList callList = CallManager::instance().getParticipantList(owner.id, call.first);
            foreach (const auto& callId, callList) {
                try {
                    if (pimpl_->calls.find(callId) != pimpl_->calls.end()
                        && pimpl_->calls[callId]->peerUri == uri) {
                        return *call.second;
                    }
                } catch (...) {
                }
            }
        }
    }
    throw std::out_of_range("No call at URI " + uri.toStdString());
}

VectorString
NewCallModel::getConferenceSubcalls(const QString& confId)
{
    QStringList callList = CallManager::instance().getParticipantList(owner.id, confId);
    VectorString result;
    result.reserve(callList.size());
    foreach (const auto& callId, callList) {
        result.push_back(callId);
    }
    return result;
}

const call::Info&
NewCallModel::getCall(const QString& uid) const
{
    return *pimpl_->calls.at(uid);
}

void
NewCallModel::updateCallMediaList(const QString& callId, bool acceptVideo)
{
    try {
        auto callInfos = pimpl_->calls.find(callId);
        if (callInfos != pimpl_->calls.end()) {
            for (auto it = callInfos->second->mediaList.begin();
                 it != callInfos->second->mediaList.end();
                 it++) {
                if ((*it)[MediaAttributeKey::MEDIA_TYPE] == MediaAttributeValue::VIDEO
                    && !acceptVideo) {
                    (*it)[MediaAttributeKey::ENABLED] = "false";
                    (*it)[MediaAttributeKey::MUTED] = "true";
                    callInfos->second->videoMuted = !acceptVideo;
                }
            }
        }
    } catch (...) {
    }
}

QString
NewCallModel::createCall(const QString& uri, bool isAudioOnly, VectorMapStringString mediaList)
{
    if (mediaList.isEmpty()) {
        MapStringString mediaAttribute = {{MediaAttributeKey::MEDIA_TYPE,
                                           MediaAttributeValue::AUDIO},
                                          {MediaAttributeKey::ENABLED, "true"},
                                          {MediaAttributeKey::MUTED, "false"},
                                          {MediaAttributeKey::SOURCE, ""},
                                          {MediaAttributeKey::LABEL, "audio_0"}};
        mediaList.push_back(mediaAttribute);
        if (!isAudioOnly) {
            mediaAttribute[MediaAttributeKey::MEDIA_TYPE] = MediaAttributeValue::VIDEO;
            mediaAttribute[MediaAttributeKey::LABEL] = "video_0";
            mediaList.push_back(mediaAttribute);
        }
    }
#ifdef ENABLE_LIBWRAP
    auto callId = CallManager::instance().placeCallWithMedia(owner.id, uri, mediaList);
#else  // dbus
    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = CallManager::instance().placeCallWithMedia(owner.id, uri, mediaList);
#endif // ENABLE_LIBWRAP

    if (callId.isEmpty()) {
        qDebug() << "no call placed between (account: " << owner.id << ", contact: " << uri << ")";
        return "";
    }

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId;
    callInfo->peerUri = uri;
    callInfo->isOutgoing = true;
    callInfo->status = call::Status::SEARCHING;
    callInfo->type = call::Type::DIALOG;
    callInfo->isAudioOnly = isAudioOnly;
    callInfo->videoMuted = isAudioOnly;
    callInfo->mediaList = mediaList;
    pimpl_->calls.emplace(callId, std::move(callInfo));

    return callId;
}

void
NewCallModel::requestMediaChange(const QString& callId,
                                 const QString& mediaLabel,
                                 const QString& uri,
                                 MediaRequestType type,
                                 bool mute)
{
    // Main audio: audio_0
    // Main video: video_0

    auto& callInfo = pimpl_->calls[callId];
    if (!callInfo)
        return;

    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    // The media label must contain either "audio" or "video" substring,
    // otherwise the media will be considered as of un-supported type.
    QString mediaType = mediaLabel.contains("audio")
                            ? MediaAttributeValue::AUDIO
                            : (mediaLabel.contains("video") ? MediaAttributeValue::VIDEO : "");

    if (mediaType.isEmpty()) {
        qCritical() << "No valide media type found in media label!";
        return;
    }

    QString resource {};
    QString srctype {};
    auto proposedList = callInfo->mediaList;

    int found = 0;

    switch (type) {
    case MediaRequestType::FILESHARING: {
        // File sharing
        resource = !uri.isEmpty() ? QString("%1%2%3")
                                        .arg(DRing::Media::VideoProtocolPrefix::FILE)
                                        .arg(sep)
                                        .arg(QUrl(uri).toLocalFile())
                                  : DRing::Media::VideoProtocolPrefix::NONE;
        if (not resource.isEmpty())
            srctype = MediaAttributeValue::SRC_TYPE_FILE;
        break;
    }
    case MediaRequestType::SCREENSHARING: {
        // Screen/window sharing
        resource = uri;
        srctype = MediaAttributeValue::SRC_TYPE_DISPLAY;
        break;
    }
    case MediaRequestType::CAMERA: {
        // Camera device
        if (mediaLabel.contains("video")) {
            resource = not uri.isEmpty() ? QString("%1%2%3")
                                               .arg(DRing::Media::VideoProtocolPrefix::CAMERA)
                                               .arg(sep)
                                               .arg(uri)
                                         : DRing::Media::VideoProtocolPrefix::NONE;
        }
        srctype = MediaAttributeValue::SRC_TYPE_CAPTURE_DEVICE;
        break;
    }
    default:
        return;
    }

    if (callInfo->type == call::Type::CONFERENCE) {
        MapStringString mediaAttribute = {{MediaAttributeKey::MEDIA_TYPE, mediaType},
                                          {MediaAttributeKey::ENABLED, "true"},
                                          {MediaAttributeKey::MUTED, mute ? "true" : "false"},
                                          {MediaAttributeKey::SOURCE_TYPE, srctype},
                                          {MediaAttributeKey::SOURCE, resource},
                                          {MediaAttributeKey::LABEL, mediaLabel}};
        proposedList.push_back(mediaAttribute);
    }

    for (auto& item : proposedList) {
        if (item[MediaAttributeKey::LABEL] == mediaLabel) {
            mute = resource.isEmpty() ? item[MediaAttributeKey::MUTED] == "false" : mute;
            item[MediaAttributeKey::ENABLED] = "true";
            item[MediaAttributeKey::MUTED] = mute ? "true" : "false";
            item[MediaAttributeKey::SOURCE_TYPE] = srctype;
            // For now, only the video source can be changed by the client.
            if (item[MediaAttributeKey::MEDIA_TYPE] == MediaAttributeValue::VIDEO) {
                item[MediaAttributeKey::SOURCE] = resource.isEmpty()
                                                      ? item[MediaAttributeKey::SOURCE]
                                                      : resource;
            }
            break;
        }
        found++;
    }

    if (found == proposedList.size() && mediaLabel == "video_0") {
        mute &= !resource.isEmpty();
        MapStringString mediaAttribute = {{MediaAttributeKey::MEDIA_TYPE,
                                           MediaAttributeValue::VIDEO},
                                          {MediaAttributeKey::ENABLED, "true"},
                                          {MediaAttributeKey::MUTED, mute ? "true" : "false"},
                                          {MediaAttributeKey::SOURCE_TYPE, srctype},
                                          {MediaAttributeKey::SOURCE, resource},
                                          {MediaAttributeKey::LABEL, mediaLabel}};
        proposedList.push_back(mediaAttribute);
    }

    CallManager::instance().requestMediaChange(owner.id, callId, proposedList);

    // If media existed and its mute state was changed here, then we should
    // update the mediaList because we will not receive signal
    // mediaNegotiationStatus
    if (found < callInfo->mediaList.size()) {
        callInfo->mediaList = proposedList;
        if (mediaLabel.contains("audio_0")) {
            callInfo->audioMuted = mute;
        } else if (mediaLabel.contains("video_0")) {
            callInfo->videoMuted = mute;
        }
        if (callInfo->status == call::Status::IN_PROGRESS)
            emit callInfosChanged(owner.id, callId);
    }
}

void
NewCallModel::accept(const QString& callId) const
{
    try {
        auto& callInfo = pimpl_->calls[callId];
        if (!callInfo)
            return;
        if (callInfo->mediaList.empty())
            CallManager::instance().accept(owner.id, callId);
        else
            CallManager::instance().acceptWithMedia(owner.id, callId, callInfo->mediaList);
    } catch (...) {
    }
}

void
NewCallModel::hangUp(const QString& callId) const
{
    if (!hasCall(callId))
        return;
    auto& call = pimpl_->calls[callId];

    if (call->status == call::Status::INCOMING_RINGING) {
        CallManager::instance().refuse(owner.id, callId);
        return;
    }

    switch (call->type) {
    case call::Type::DIALOG:
        CallManager::instance().hangUp(owner.id, callId);
        break;
    case call::Type::CONFERENCE:
        CallManager::instance().hangUpConference(owner.id, callId);
        break;
    case call::Type::INVALID:
    default:
        break;
    }
}

void
NewCallModel::refuse(const QString& callId) const
{
    if (!hasCall(callId))
        return;
    CallManager::instance().refuse(owner.id, callId);
}

void
NewCallModel::toggleAudioRecord(const QString& callId) const
{
    CallManager::instance().toggleRecording(owner.id, callId);
}

void
NewCallModel::playDTMF(const QString& callId, const QString& value) const
{
    if (!hasCall(callId))
        return;
    if (pimpl_->calls[callId]->status != call::Status::IN_PROGRESS)
        return;
    CallManager::instance().playDTMF(value);
}

void
NewCallModel::togglePause(const QString& callId) const
{
    // function should now only serves for SIP accounts
    if (!hasCall(callId))
        return;
    auto& call = pimpl_->calls[callId];

    if (call->status == call::Status::PAUSED) {
        if (call->type == call::Type::DIALOG) {
            CallManager::instance().unhold(owner.id, callId);
        } else {
            CallManager::instance().unholdConference(owner.id, callId);
        }
    } else if (call->status == call::Status::IN_PROGRESS) {
        if (call->type == call::Type::DIALOG)
            CallManager::instance().hold(owner.id, callId);
        else {
            CallManager::instance().holdConference(owner.id, callId);
        }
    }
}

void
NewCallModel::toggleMedia(const QString& callId, const NewCallModel::Media media)
{
    if (!hasCall(callId))
        return;
    auto mediaLabel = media == NewCallModel::Media::VIDEO ? "video_0" : "audio_0";
    requestMediaChange(callId, mediaLabel, "", MediaRequestType::CAMERA, true);
}

void
NewCallModel::setQuality(const QString& callId, const double quality) const
{
    Q_UNUSED(callId)
    Q_UNUSED(quality)
    qDebug() << "setQuality isn't implemented yet";
}

void
NewCallModel::transfer(const QString& callId, const QString& to) const
{
    CallManager::instance().transfer(owner.id, callId, to);
}

void
NewCallModel::transferToCall(const QString& callId, const QString& callIdDest) const
{
    CallManager::instance().attendedTransfer(owner.id, callId, callIdDest);
}

void
NewCallModel::joinCalls(const QString& callIdA, const QString& callIdB) const
{
    // Get call informations
    call::Info call1, call2;
    QString accountIdCall1 = {}, accountIdCall2 = {};
    for (const auto& account_id : owner.accountModel->getAccountList()) {
        try {
            auto& accountInfo = owner.accountModel->getAccountInfo(account_id);
            if (accountInfo.callModel->hasCall(callIdA)) {
                call1 = accountInfo.callModel->getCall(callIdA);
                accountIdCall1 = account_id;
            }
            if (accountInfo.callModel->hasCall(callIdB)) {
                call2 = accountInfo.callModel->getCall(callIdB);
                accountIdCall2 = account_id;
            }
            if (!accountIdCall1.isEmpty() && !accountIdCall2.isEmpty())
                break;
        } catch (...) {
        }
    }
    if (accountIdCall1.isEmpty() || accountIdCall2.isEmpty()) {
        qWarning() << "Can't join inexistent calls.";
        return;
    }

    if (call1.type == call::Type::CONFERENCE && call2.type == call::Type::CONFERENCE) {
        bool joined = CallManager::instance().joinConference(accountIdCall1,
                                                             callIdA,
                                                             accountIdCall2,
                                                             callIdB);

        if (!joined) {
            qWarning() << "Conference: " << callIdA << " couldn't join conference " << callIdB;
            return;
        }
        if (accountIdCall1 != owner.id) {
            // If the conference is added from another account
            try {
                auto& accountInfo = owner.accountModel->getAccountInfo(accountIdCall1);
                if (accountInfo.callModel->hasCall(callIdA)) {
                    emit accountInfo.callModel->callAddedToConference(callIdA, callIdB);
                }
            } catch (...) {
            }
        } else {
            emit callAddedToConference(callIdA, callIdB);
        }
    } else if (call1.type == call::Type::CONFERENCE || call2.type == call::Type::CONFERENCE) {
        auto call = call1.type == call::Type::CONFERENCE ? callIdB : callIdA;
        auto conf = call1.type == call::Type::CONFERENCE ? callIdA : callIdB;
        // Unpause conference if conference was not active
        CallManager::instance().unholdConference(owner.id, conf);
        auto accountCall = call1.type == call::Type::CONFERENCE ? accountIdCall2 : accountIdCall1;

        bool joined = CallManager::instance().addParticipant(accountCall, call, accountCall, conf);
        if (!joined) {
            qWarning() << "Call: " << call << " couldn't join conference " << conf;
            return;
        }
        if (accountCall != owner.id) {
            // If the call is added from another account
            try {
                auto& accountInfo = owner.accountModel->getAccountInfo(accountCall);
                if (accountInfo.callModel->hasCall(call)) {
                    accountInfo.callModel->pimpl_->slotConferenceCreated(owner.id, conf);
                }
            } catch (...) {
            }
        } else
            emit callAddedToConference(call, conf);

        // Remove from pendingConferences_
        for (int i = 0; i < pimpl_->pendingConferencees_.size(); ++i) {
            if (pimpl_->pendingConferencees_.at(i).callId == call) {
                Q_EMIT beginRemovePendingConferenceesRows(i);
                pimpl_->pendingConferencees_.removeAt(i);
                Q_EMIT endRemovePendingConferenceesRows();
                break;
            }
        }
    } else {
        CallManager::instance().joinParticipant(accountIdCall1, callIdA, accountIdCall2, callIdB);
        // NOTE: This will trigger slotConferenceCreated.
    }
}

QString
NewCallModel::callAndAddParticipant(const QString uri, const QString& callId, bool audioOnly)
{
    auto newCallId = createCall(uri, audioOnly, pimpl_->calls[callId]->mediaList);
    Q_EMIT beginInsertPendingConferenceesRows(0);
    pimpl_->pendingConferencees_.prepend({uri, newCallId, callId});
    Q_EMIT endInsertPendingConferenceesRows();
    return newCallId;
}

void
NewCallModel::removeParticipant(const QString& callId, const QString& participant) const
{
    Q_UNUSED(callId)
    Q_UNUSED(participant)
    qDebug() << "removeParticipant() isn't implemented yet";
}

QString
NewCallModel::getFormattedCallDuration(const QString& callId) const
{
    if (!hasCall(callId))
        return "00:00";
    auto& startTime = pimpl_->calls[callId]->startTime;
    if (startTime.time_since_epoch().count() == 0)
        return "00:00";
    auto now = std::chrono::steady_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()
                                                              - startTime.time_since_epoch())
                 .count();
    return authority::storage::getFormattedCallDuration(d);
}

bool
NewCallModel::isRecording(const QString& callId) const
{
    if (!hasCall(callId))
        return false;
    return CallManager::instance().getIsRecording(owner.id, callId);
}

QString
NewCallModel::getSIPCallStatusString(const short& statusCode)
{
    auto element = sip_call_status_code_map.find(statusCode);
    if (element != sip_call_status_code_map.end()) {
        return element->second;
    }
    return "";
}

const QList<call::PendingConferenceeInfo>&
NewCallModel::getPendingConferencees()
{
    return pimpl_->pendingConferencees_;
}

video::RenderedDevice
NewCallModel::getCurrentRenderedDevice(const QString& call_id) const
{
    video::RenderedDevice result;
    MapStringString callDetails;
    QStringList conferences = CallManager::instance().getConferenceList(owner.id);
    if (conferences.indexOf(call_id) != -1) {
        callDetails = CallManager::instance().getConferenceDetails(owner.id, call_id);
    } else {
        callDetails = CallManager::instance().getCallDetails(owner.id, call_id);
    }
    if (!callDetails.contains("VIDEO_SOURCE")) {
        return result;
    }
    auto source = callDetails["VIDEO_SOURCE"];
    auto sourceSize = source.size();
    if (source.startsWith("camera://")) {
        result.type = video::DeviceType::CAMERA;
        result.name = source.right(sourceSize - QString("camera://").size());
    } else if (source.startsWith("file://")) {
        result.type = video::DeviceType::FILE;
        result.name = source.right(sourceSize - QString("file://").size());
    } else if (source.startsWith("display://")) {
        result.type = video::DeviceType::DISPLAY;
        result.name = source.right(sourceSize - QString("display://").size());
    }
    return result;
}

void
NewCallModel::setInputFile(const QString& uri, const QString& callId)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    auto resource = !uri.isEmpty() ? QString("%1%2%3")
                                         .arg(DRing::Media::VideoProtocolPrefix::FILE)
                                         .arg(sep)
                                         .arg(QUrl(uri).toLocalFile())
                                   : DRing::Media::VideoProtocolPrefix::NONE;
    if (callId.isEmpty()) {
        VideoManager::instance().openVideoInput(resource);
    } else {
        CallManager::instance().switchInput(owner.id, callId, resource);
    }
}

QString
NewCallModel::getDisplay(int idx, int x, int y, int w, int h)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    return QString("%1%2:%3+%4,%5 %6x%7")
        .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
        .arg(sep)
        .arg(idx)
        .arg(x)
        .arg(y)
        .arg(w)
        .arg(h);
}

QString
NewCallModel::getDisplay(const QString& windowId)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    return QString("%1%2:+0,0 window-id:%3")
        .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
        .arg(sep)
        .arg(windowId);
}

void
NewCallModel::setDisplay(int idx, int x, int y, int w, int h, const QString& callId)
{
    auto resource = getDisplay(idx, x, y, w, h);
    if (callId.isEmpty()) {
        VideoManager::instance().openVideoInput(resource);
    } else {
        CallManager::instance().switchInput(owner.id, callId, resource);
    }
}

void
NewCallModel::switchInputTo(const QString& id, const QString& callId)
{
    QString resource;
    auto devices = pimpl_->lrc.getAVModel().getDevices();
    auto deviceAvailable = std::find(std::begin(devices), std::end(devices), id);
    if (deviceAvailable != devices.end()) {
        QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
        resource = QString("%1%2%3").arg(DRing::Media::VideoProtocolPrefix::CAMERA).arg(sep).arg(id);
    } else {
        resource = QString(DRing::Media::VideoProtocolPrefix::NONE);
    }
    if (callId.isEmpty()) {
        VideoManager::instance().openVideoInput(resource);
    } else {
        CallManager::instance().switchInput(owner.id, callId, resource);
    }
}

NewCallModelPimpl::NewCallModelPimpl(const NewCallModel& linked,
                                     Lrc& lrc,
                                     const CallbacksHandler& callbacksHandler,
                                     const BehaviorController& behaviorController)
    : linked(linked)
    , lrc(lrc)
    , callbacksHandler(callbacksHandler)
    , behaviorController(behaviorController)
{
    connect(&callbacksHandler,
            &CallbacksHandler::incomingCallWithMedia,
            this,
            &NewCallModelPimpl::slotIncomingCallWithMedia);
    connect(&callbacksHandler,
            &CallbacksHandler::mediaChangeRequested,
            this,
            &NewCallModelPimpl::slotMediaChangeRequested);
    connect(&callbacksHandler,
            &CallbacksHandler::callStateChanged,
            this,
            &NewCallModelPimpl::slotCallStateChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::mediaNegotiationStatus,
            this,
            &NewCallModelPimpl::slotMediaNegotiationStatus);
    connect(&callbacksHandler,
            &CallbacksHandler::incomingVCardChunk,
            this,
            &NewCallModelPimpl::slotincomingVCardChunk);
    connect(&callbacksHandler,
            &CallbacksHandler::conferenceCreated,
            this,
            &NewCallModelPimpl::slotConferenceCreated);
    connect(&callbacksHandler,
            &CallbacksHandler::voiceMailNotify,
            this,
            &NewCallModelPimpl::slotVoiceMailNotify);
    connect(&CallManager::instance(),
            &CallManagerInterface::onConferenceInfosUpdated,
            this,
            &NewCallModelPimpl::slotOnConferenceInfosUpdated);
    connect(&callbacksHandler,
            &CallbacksHandler::remoteRecordingChanged,
            this,
            &NewCallModelPimpl::remoteRecordingChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::startedDecoding,
            this,
            &NewCallModelPimpl::startedDecoding);

#ifndef ENABLE_LIBWRAP
    // Only necessary with dbus since the daemon runs separately
    initCallFromDaemon();
    initConferencesFromDaemon();
#endif
}

NewCallModelPimpl::~NewCallModelPimpl() {}

void
NewCallModelPimpl::initCallFromDaemon()
{
    QStringList callList = CallManager::instance().getCallList(linked.owner.id);
    for (const auto& callId : callList) {
        MapStringString details = CallManager::instance().getCallDetails(linked.owner.id, callId);
        auto callInfo = std::make_shared<call::Info>();
        callInfo->id = callId;
        auto now = std::chrono::steady_clock::now();
        auto system_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto diff = static_cast<int64_t>(system_now)
                    - std::stol(details["TIMESTAMP_START"].toStdString());
        callInfo->startTime = now - std::chrono::seconds(diff);
        callInfo->status = call::to_status(details["CALL_STATE"]);
        auto endId = details["PEER_NUMBER"].indexOf("@");
        callInfo->peerUri = details["PEER_NUMBER"].left(endId);
        if (linked.owner.profileInfo.type == lrc::api::profile::Type::JAMI) {
            callInfo->peerUri = "ring:" + callInfo->peerUri;
        }
        callInfo->videoMuted = details["VIDEO_MUTED"] == "true";
        callInfo->audioMuted = details["AUDIO_MUTED"] == "true";
        callInfo->type = call::Type::DIALOG;
        VectorMapStringString infos = CallManager::instance().getConferenceInfos(linked.owner.id,
                                                                                 callId);
        callInfo->participantsInfos = infos;
        calls.emplace(callId, std::move(callInfo));
        // NOTE/BUG: the videorenderer can't know that the client has restarted
        // So, for now, a user will have to manually restart the medias until
        // this renderer is not redesigned.
    }
}

bool
NewCallModelPimpl::checkMediaDeviceMuted(const MapStringString& mediaAttributes)
{
    return mediaAttributes[MediaAttributeKey::SOURCE_TYPE]
               == MediaAttributeValue::SRC_TYPE_CAPTURE_DEVICE
           && (mediaAttributes[MediaAttributeKey::ENABLED] == "false"
               || mediaAttributes[MediaAttributeKey::MUTED] == "true");
}

void
NewCallModelPimpl::initConferencesFromDaemon()
{
    QStringList callList = CallManager::instance().getConferenceList(linked.owner.id);
    for (const auto& callId : callList) {
        QMap<QString, QString> details = CallManager::instance()
                                             .getConferenceDetails(linked.owner.id, callId);
        auto callInfo = std::make_shared<call::Info>();
        callInfo->id = callId;
        QStringList callList = CallManager::instance().getParticipantList(linked.owner.id, callId);
        foreach (const auto& call, callList) {
            MapStringString callDetails = CallManager::instance().getCallDetails(linked.owner.id,
                                                                                 call);
            auto now = std::chrono::steady_clock::now();
            auto system_now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto diff = static_cast<int64_t>(system_now)
                        - std::stol(callDetails["TIMESTAMP_START"].toStdString());
            callInfo->status = details["CONF_STATE"] == "ACTIVE_ATTACHED"
                                   ? call::Status::IN_PROGRESS
                                   : call::Status::PAUSED;
            callInfo->startTime = now - std::chrono::seconds(diff);
            emit linked.callAddedToConference(call, callId);
        }
        callInfo->type = call::Type::CONFERENCE;
        VectorMapStringString infos = CallManager::instance().getConferenceInfos(linked.owner.id,
                                                                                 callId);
        callInfo->participantsInfos = infos;
        calls.emplace(callId, std::move(callInfo));
    }
}

void
NewCallModel::setCurrentCall(const QString& callId) const
{
    if (!pimpl_->manageCurrentCall_)
        return;
    auto it = std::find_if(pimpl_->pendingConferencees_.begin(),
                           pimpl_->pendingConferencees_.end(),
                           [callId](const lrc::api::call::PendingConferenceeInfo& info) -> bool {
                               return info.callId == callId;
                           });

    // Set current call only if not adding this call
    // to a current conference
    if (it != pimpl_->pendingConferencees_.end())
        return;
    if (!hasCall(callId))
        return;

    // The client should be able to set the current call multiple times
    if (pimpl_->currentCall_ == callId)
        return;
    pimpl_->currentCall_ = callId;

    // Unhold call
    auto& call = pimpl_->calls[callId];
    if (call->status == call::Status::PAUSED) {
        auto& call = pimpl_->calls[callId];
        if (call->type == call::Type::DIALOG) {
            CallManager::instance().unhold(owner.id, callId);
        } else {
            CallManager::instance().unholdConference(owner.id, callId);
        }
    }

    VectorString filterCalls;
    QStringList conferences = CallManager::instance().getConferenceList(owner.id);
    for (const auto& confId : conferences) {
        QStringList callList = CallManager::instance().getParticipantList(owner.id, confId);
        foreach (const auto& cid, callList) {
            filterCalls.push_back(cid);
        }
    }
    for (const auto& cid : Lrc::activeCalls()) {
        auto filtered = std::find(filterCalls.begin(), filterCalls.end(), cid) != filterCalls.end();
        if (cid != callId && !filtered) {
            // Only hold calls for a non rendez-vous point
            MapStringString callDetails = CallManager::instance().getCallDetails(owner.id, callId);
            auto accountId = callDetails["ACCOUNTID"];
            CallManager::instance().hold(owner.id, cid);
        }
    }
    if (!lrc::api::Lrc::holdConferences) {
        return;
    }
    for (const auto& confId : conferences) {
        if (callId != confId) {
            MapStringString confDetails = CallManager::instance().getConferenceDetails(owner.id,
                                                                                       confId);
            // Only hold conference if attached
            if (confDetails["CALL_STATE"] == "ACTIVE_DETACHED")
                continue;
            QStringList callList = CallManager::instance().getParticipantList(owner.id, confId);
            if (callList.indexOf(callId) == -1)
                CallManager::instance().holdConference(owner.id, confId);
        }
    }
}

void
NewCallModel::setConferenceLayout(const QString& confId, const call::Layout& layout)
{
    auto call = pimpl_->calls.find(confId);
    if (call != pimpl_->calls.end()) {
        switch (layout) {
        case call::Layout::GRID:
            CallManager::instance().setConferenceLayout(owner.id, confId, 0);
            break;
        case call::Layout::ONE_WITH_SMALL:
            CallManager::instance().setConferenceLayout(owner.id, confId, 1);
            break;
        case call::Layout::ONE:
            CallManager::instance().setConferenceLayout(owner.id, confId, 2);
            break;
        }
        call->second->layout = layout;
    }
}

void
NewCallModel::setActiveParticipant(const QString& confId, const QString& participant)
{
    CallManager::instance().setActiveParticipant(owner.id, confId, participant);
}

bool
NewCallModel::isModerator(const QString& confId, const QString& uri)
{
    auto call = pimpl_->calls.find(confId);
    if (call == pimpl_->calls.end() or not call->second)
        return false;
    auto ownerUri = owner.profileInfo.uri;
    auto uriToCheck = uri;
    if (uriToCheck.isEmpty()) {
        uriToCheck = ownerUri;
    }
    auto isModerator = uriToCheck == ownerUri
                           ? call->second->type == lrc::api::call::Type::CONFERENCE
                           : false;
    if (!isModerator && call->second->participantsInfos.size() != 0) {
        for (const auto& participant : call->second->participantsInfos) {
            if (participant["uri"] == uriToCheck) {
                isModerator = participant["isModerator"] == "true";
                break;
            }
        }
    }
    return isModerator;
}

void
NewCallModel::setModerator(const QString& confId, const QString& peerId, const bool& state)
{
    CallManager::instance().setModerator(owner.id, confId, peerId, state);
}

bool
NewCallModel::isHandRaised(const QString& confId, const QString& uri) noexcept
{
    auto call = pimpl_->calls.find(confId);
    if (call == pimpl_->calls.end() or not call->second)
        return false;
    auto ownerUri = owner.profileInfo.uri;
    auto uriToCheck = uri;
    if (uriToCheck.isEmpty()) {
        uriToCheck = ownerUri;
    }
    auto handRaised = false;
    for (const auto& participant : call->second->participantsInfos) {
        auto itUri = participant.find("uri");
        auto itHand = participant.find("handRaised");
        if (itUri != participant.end() && itHand != participant.end() && *itUri == uriToCheck) {
            handRaised = participant["handRaised"] == "true";
            break;
        }
    }
    return handRaised;
}

void
NewCallModel::setHandRaised(const QString& accountId,
                            const QString& confId,
                            const QString& peerId,
                            bool state)
{
    auto ownerUri = owner.profileInfo.uri;
    auto uriToCheck = peerId;
    if (uriToCheck.isEmpty()) {
        uriToCheck = ownerUri;
    }
    CallManager::instance().raiseParticipantHand(accountId, confId, uriToCheck, state);
}

void
NewCallModel::muteParticipant(const QString& confId, const QString& peerId, const bool& state)
{
    CallManager::instance().muteParticipant(owner.id, confId, peerId, state);
}

void
NewCallModel::hangupParticipant(const QString& confId, const QString& participant)
{
    CallManager::instance().hangupParticipant(owner.id, confId, participant);
}

void
NewCallModel::sendSipMessage(const QString& callId, const QString& body) const
{
    MapStringString payloads;
    payloads["text/plain"] = body;

    CallManager::instance().sendTextMessage(owner.id, callId, payloads, true /* not used */);
}

bool
NewCallModel::isConferenceHost(const QString& callId)
{
    auto call = pimpl_->calls.find(callId);
    if (call == pimpl_->calls.end() or not call->second)
        return false;
    else
        return call->second->type == lrc::api::call::Type::CONFERENCE;
}

void
NewCallModelPimpl::slotIncomingCallWithMedia(const QString& accountId,
                                             const QString& callId,
                                             const QString& fromId,
                                             const QString& displayname,
                                             const VectorMapStringString& mediaList)
{
    if (linked.owner.id != accountId) {
        return;
    }
    // TODO: uncomment this. For now, the rendez-vous account is showing calls
    // if (linked.owner.confProperties.isRendezVous) {
    //    // Do not notify for calls if rendez vous because it's in a detached
    //    // mode and auto answer is managed by the daemon
    //    return;
    //}

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId;
    // peer uri = ring:<jami_id> or sip number
    auto uri = (linked.owner.profileInfo.type != profile::Type::SIP && !fromId.contains("ring:"))
                   ? "ring:" + fromId
                   : fromId;
    callInfo->peerUri = uri;
    callInfo->isOutgoing = false;
    callInfo->status = call::Status::INCOMING_RINGING;
    callInfo->type = call::Type::DIALOG;
    callInfo->isAudioOnly = true;
    callInfo->videoMuted = true;
    for (const auto& item : mediaList) {
        if (item[MediaAttributeKey::MEDIA_TYPE] == MediaAttributeValue::VIDEO) {
            callInfo->isAudioOnly = false;
            if (!checkMediaDeviceMuted(item)) {
                callInfo->videoMuted = false;
                break;
            }
        }
    }
    callInfo->mediaList = mediaList;
    calls.emplace(callId, std::move(callInfo));

    if (!linked.owner.confProperties.allowIncoming
        && linked.owner.profileInfo.type == profile::Type::JAMI) {
        linked.refuse(callId);
        return;
    }

    emit linked.newIncomingCall(fromId, callId, displayname);
}

void
NewCallModelPimpl::slotMediaChangeRequested(const QString& accountId,
                                            const QString& callId,
                                            const VectorMapStringString& mediaList)
{
    if (linked.owner.id != accountId) {
        return;
    }

    if (mediaList.empty())
        return;

    auto& callInfo = calls[callId];
    if (!callInfo)
        return;

    QList<QString> currentMediaLabels {};
    for (auto& currentItem : callInfo->mediaList)
        currentMediaLabels.append(currentItem[MediaAttributeKey::LABEL]);

    auto answerMedia = QList<MapStringString>::fromVector(mediaList);

    for (auto& item : answerMedia) {
        int index = currentMediaLabels.indexOf(item[MediaAttributeKey::LABEL]);
        if (index >= 0) {
            item[MediaAttributeKey::MUTED] = callInfo->mediaList[index][MediaAttributeKey::MUTED];
            item[MediaAttributeKey::ENABLED] = callInfo->mediaList[index][MediaAttributeKey::ENABLED];
        } else {
            item[MediaAttributeKey::MUTED] = "true";
            item[MediaAttributeKey::ENABLED] = "true";
        }
    }
    CallManager::instance().answerMediaChangeRequest(linked.owner.id,
                                                     callId,
                                                     QVector<MapStringString>::fromList(
                                                         answerMedia));
}

void
NewCallModelPimpl::slotCallStateChanged(const QString& accountId,
                                        const QString& callId,
                                        const QString& state,
                                        int code)
{
    if (accountId != linked.owner.id || !linked.hasCall(callId))
        return;

    auto status = call::to_status(state);
    auto& call = calls[callId];
    if (!call)
        return;

    if (status == call::Status::ENDED && !call::isTerminating(call->status)) {
        call->status = call::Status::TERMINATING;
        emit linked.callStatusChanged(callId, code);
        emit behaviorController.callStatusChanged(linked.owner.id, callId);
    }

    // proper state transition
    auto previousStatus = call->status;
    call->status = status;

    if (previousStatus == call->status) {
        // call state didn't change, simply ignore signal
        return;
    }

    qDebug() << QString("slotCallStateChanged (call: %1), from %2 to %3")
                    .arg(callId)
                    .arg(call::to_string(previousStatus))
                    .arg(call::to_string(status));

    // NOTE: signal emission order matters, always emit CallStatusChanged before CallEnded
    emit linked.callStatusChanged(callId, code);
    emit behaviorController.callStatusChanged(linked.owner.id, callId);

    if (call->status == call::Status::ENDED) {
        emit linked.callEnded(callId);

        // Remove from pendingConferences_
        for (int i = 0; i < pendingConferencees_.size(); ++i) {
            if (pendingConferencees_.at(i).callId == callId) {
                Q_EMIT linked.beginRemovePendingConferenceesRows(i);
                pendingConferencees_.removeAt(i);
                Q_EMIT linked.endRemovePendingConferenceesRows();
                break;
            }
        }
    } else if (call->status == call::Status::IN_PROGRESS) {
        if (previousStatus == call::Status::INCOMING_RINGING
            || previousStatus == call::Status::OUTGOING_RINGING) {
            if (previousStatus == call::Status::INCOMING_RINGING
                && linked.owner.profileInfo.type != profile::Type::SIP
                && !linked.owner.confProperties.isRendezVous) { // TODO remove this when we want to
                                                                // not show calls in rendez-vous
                linked.setCurrentCall(callId);
            }
            call->startTime = std::chrono::steady_clock::now();
            emit linked.callStarted(callId);
            sendProfile(callId);
        }
        // Add to calls if in pendingConferences_
        for (int i = 0; i < pendingConferencees_.size(); ++i) {
            if (pendingConferencees_.at(i).callId == callId) {
                linked.joinCalls(pendingConferencees_.at(i).callIdToJoin,
                                 pendingConferencees_.at(i).callId);
                break;
            }
        }
    } else if (call->status == call::Status::PAUSED) {
        currentCall_ = "";
    }
}

void
NewCallModelPimpl::slotMediaNegotiationStatus(const QString& callId,
                                              const QString&,
                                              const VectorMapStringString& mediaList)
{
    if (!linked.hasCall(callId)) {
        return;
    }

    auto& callInfo = calls[callId];
    if (!callInfo) {
        return;
    }

    callInfo->isAudioOnly = true;
    callInfo->videoMuted = true;
    for (const auto& item : mediaList) {
        if (item[MediaAttributeKey::MEDIA_TYPE] == MediaAttributeValue::VIDEO) {
            if (item[MediaAttributeKey::ENABLED] == "true") {
                callInfo->isAudioOnly = false;
            }
            callInfo->videoMuted = checkMediaDeviceMuted(item);
        }
        if (item[MediaAttributeKey::MEDIA_TYPE] == MediaAttributeValue::AUDIO) {
            callInfo->audioMuted = checkMediaDeviceMuted(item);
        }
    }
    callInfo->mediaList = mediaList;
    if (callInfo->status == call::Status::IN_PROGRESS)
        emit linked.callInfosChanged(linked.owner.id, callId);
}

void
NewCallModelPimpl::slotincomingVCardChunk(const QString& accountId,
                                          const QString& callId,
                                          const QString& from,
                                          int part,
                                          int numberOfParts,
                                          const QString& payload)
{
    if (accountId != linked.owner.id || !linked.hasCall(callId))
        return;

    auto it = vcardsChunks.find(from);
    if (it != vcardsChunks.end()) {
        vcardsChunks[from][part - 1] = payload;

        if (not std::any_of(vcardsChunks[from].begin(),
                            vcardsChunks[from].end(),
                            [](const auto& s) { return s.isEmpty(); })) {
            profile::Info profileInfo;
            profileInfo.uri = from;
            profileInfo.type = profile::Type::JAMI;

            QString vcardPhoto;

            for (auto& chunk : vcardsChunks[from])
                vcardPhoto += chunk;

            for (auto& e : QString(vcardPhoto).split("\n"))
                if (e.contains("PHOTO"))
                    profileInfo.avatar = e.split(":")[1];
                else if (e.contains("FN"))
                    profileInfo.alias = e.split(":")[1];

            contact::Info contactInfo;
            contactInfo.profileInfo = profileInfo;

            linked.owner.contactModel->addContact(contactInfo);
            if (!lrc::api::Lrc::cacheAvatars.load())
                contactInfo.profileInfo.avatar.clear();
            vcardsChunks.erase(from); // Transfer is finish, we don't want to reuse this entry.
        }
    } else {
        vcardsChunks[from] = VectorString(numberOfParts);
        vcardsChunks[from][part - 1] = payload;
    }
}

void
NewCallModelPimpl::slotVoiceMailNotify(const QString& accountId,
                                       int newCount,
                                       int oldCount,
                                       int urgentCount)
{
    emit linked.voiceMailNotify(accountId, newCount, oldCount, urgentCount);
}

void
NewCallModelPimpl::slotOnConferenceInfosUpdated(const QString& confId,
                                                const VectorMapStringString& infos)
{
    auto it = calls.find(confId);
    if (it == calls.end() or not it->second)
        return;

    qDebug() << "New conference layout received for call " << confId;

    // if Jami, remove @ring.dht
    it->second->participantsInfos = infos;
    for (auto& i : it->second->participantsInfos) {
        i["uri"].replace("@ring.dht", "");
        if (i["uri"].isEmpty()) {
            if (it->second->type == call::Type::CONFERENCE) {
                i["uri"] = linked.owner.profileInfo.uri;
            } else {
                i["uri"] = it->second->peerUri.replace("ring:", "");
            }
        }
    }

    emit linked.onParticipantsChanged(confId);

    for (auto& info : infos) {
        if (info["uri"].isEmpty()) {
            it->second->videoMuted = info["videoMuted"] == "true";
            it->second->audioMuted = info["audioLocalMuted"] == "true";
        }
    }

    // TODO: remove when the rendez-vous UI will be done
    // For now, the rendez-vous account can see ongoing calls
    // And must be notified when a new
    QStringList callList = CallManager::instance().getParticipantList(linked.owner.id, confId);
    foreach (const auto& call, callList) {
        emit linked.callAddedToConference(call, confId);
        calls[call]->videoMuted = it->second->videoMuted;
        calls[call]->audioMuted = it->second->audioMuted;
        Q_EMIT linked.callInfosChanged(linked.owner.id, call);
    }
}

bool
NewCallModel::hasCall(const QString& callId) const
{
    return pimpl_->calls.find(callId) != pimpl_->calls.end();
}

void
NewCallModelPimpl::slotConferenceCreated(const QString& accountId, const QString& confId)
{
    if (accountId != linked.owner.id)
        return;
    // Detect if conference is created for this account
    QStringList callList = CallManager::instance().getParticipantList(linked.owner.id, confId);
    auto hasConference = false;
    foreach (const auto& call, callList) {
        hasConference |= linked.hasCall(call);
    }
    if (!hasConference)
        return;

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = confId;
    callInfo->status = call::Status::IN_PROGRESS;
    callInfo->type = call::Type::CONFERENCE;
    callInfo->startTime = std::chrono::steady_clock::now();
    callInfo->participantsInfos = CallManager::instance().getConferenceInfos(linked.owner.id,
                                                                             confId);
    for (auto& i : callInfo->participantsInfos)
        i["uri"].replace("@ring.dht", "");
    calls[confId] = callInfo;
    foreach (const auto& call, callList) {
        emit linked.callAddedToConference(call, confId);
        // Remove call from pendingConferences_
        for (int i = 0; i < pendingConferencees_.size(); ++i) {
            if (pendingConferencees_.at(i).callId == call) {
                Q_EMIT linked.beginRemovePendingConferenceesRows(i);
                pendingConferencees_.removeAt(i);
                Q_EMIT linked.endRemovePendingConferenceesRows();
                break;
            }
        }
    }
}

void
NewCallModelPimpl::sendProfile(const QString& callId)
{
    auto vCard = linked.owner.accountModel->accountVCard(linked.owner.id);

    std::random_device rdev;
    auto key = std::to_string(dis(rdev));

    int i = 0;
    int total = vCard.size() / 1000 + (vCard.size() % 1000 ? 1 : 0);
    while (vCard.size()) {
        auto sizeLimit = std::min(1000, static_cast<int>(vCard.size()));
        MapStringString chunk;
        chunk[QString("%1; id=%2,part=%3,of=%4")
                  .arg(lrc::vCard::PROFILE_VCF)
                  .arg(key.c_str())
                  .arg(QString::number(i + 1))
                  .arg(QString::number(total))]
            = vCard.left(sizeLimit);
        vCard.remove(0, sizeLimit);
        ++i;
        CallManager::instance().sendTextMessage(linked.owner.id, callId, chunk, false);
    }
}

void
NewCallModelPimpl::remoteRecordingChanged(const QString& callId,
                                          const QString& peerNumber,
                                          bool state)
{
    auto it = calls.find(callId);
    if (it == calls.end() or not it->second)
        return;

    auto uri = peerNumber;

    if (uri.contains("ring:"))
        uri.remove("ring:");
    if (uri.contains("jami:"))
        uri.remove("jami:");
    if (uri.contains("@ring.dht"))
        uri.remove("@ring.dht");

    // Add peer to peerRec set
    if (state && not it->second->peerRec.contains(uri))
        it->second->peerRec.insert(uri);

    // remove peer from peerRec set
    if (!state && it->second->peerRec.contains(uri))
        it->second->peerRec.remove(uri);

    emit linked.remoteRecordingChanged(callId, it->second->peerRec, state);
}

void
NewCallModelPimpl::startedDecoding(const QString& id, const QString& shmPath, int width, int height)
{
    auto it = calls.find(id);
    if (it == calls.end())
        return;
    video::Settings settings;
    settings.size = toQString(width) + "x" + toQString(height);
    lrc.getAVModel().addRenderer(id, settings, shmPath);
}

} // namespace lrc

#include "api/moc_newcallmodel.cpp"

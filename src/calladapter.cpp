/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "calladapter.h"
#include "utils.h"

#include <QApplication>

CallAdapter::CallAdapter(QObject* parent)
    : QmlAdapterBase(parent)
    , oneSecondTimer_(new QTimer(this))
{
    accountId_ = LRCInstance::getCurrAccId();
    connectCallModel(accountId_);

    connect(&LRCInstance::behaviorController(),
            &BehaviorController::showIncomingCallView,
            this,
            &CallAdapter::onShowIncomingCallView);

    connect(&LRCInstance::behaviorController(),
            &BehaviorController::showCallView,
            this,
            &CallAdapter::onShowCallView);

    connect(&LRCInstance::instance(),
            &LRCInstance::currentAccountChanged,
            this,
            &CallAdapter::onAccountChanged);
}

void
CallAdapter::onAccountChanged()
{
    accountId_ = LRCInstance::getCurrAccId();
    connectCallModel(accountId_);
}

void
CallAdapter::placeAudioOnlyCall()
{
    const auto convUid = LRCInstance::getCurrentConvUid();
    if (!convUid.isEmpty()) {
        LRCInstance::getCurrentConversationModel()->placeAudioOnlyCall(convUid);
    }
}

void
CallAdapter::placeCall()
{
    const auto convUid = LRCInstance::getCurrentConvUid();
    if (!convUid.isEmpty()) {
        LRCInstance::getCurrentConversationModel()->placeCall(convUid);
    }
}

void
CallAdapter::hangUpACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->hangUp(convInfo.callId);
    }
}

void
CallAdapter::refuseACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->refuse(convInfo.callId);
    }
}

void
CallAdapter::acceptACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->accept(convInfo.callId);
        auto& accInfo = LRCInstance::getAccountInfo(convInfo.accountId);
        accInfo.callModel->setCurrentCall(convInfo.callId);

        auto contactUri = convInfo.participants.front();
        if (contactUri.isEmpty()) {
            return;
        }
        try {
            auto& contact = accInfo.contactModel->getContact(contactUri);
            if (contact.profileInfo.type == lrc::api::profile::Type::PENDING) {
                LRCInstance::getCurrentConversationModel()->makePermanent(convInfo.uid);
            }
        } catch (...) {
        }
    }
}

void
CallAdapter::onShowIncomingCallView(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (convInfo.uid.isEmpty()) {
        return;
    }
    auto selectedAccountId = LRCInstance::getCurrAccId();
    auto* callModel = LRCInstance::getCurrentCallModel();

    if (!callModel->hasCall(convInfo.callId)) {
        if (QApplication::focusObject() == nullptr || accountId != selectedAccountId) {
            showNotification(accountId, convInfo.uid);
            return;
        }

        const auto& currentConvInfo = LRCInstance::getConversationFromConvUid(
            LRCInstance::getCurrentConvUid());

        // Current call
        auto currentConvHasCall = callModel->hasCall(currentConvInfo.callId);
        if (currentConvHasCall) {
            auto currentCall = callModel->getCall(currentConvInfo.callId);
            if (currentCall.status == lrc::api::call::Status::CONNECTED
                || currentCall.status == lrc::api::call::Status::IN_PROGRESS) {
                showNotification(accountId, convInfo.uid);
                return;
            }
        }
        emit callSetupMainViewRequired(accountId, convInfo.uid);
        emit LRCInstance::instance().updateSmartList();
        return;
    }

    auto call = callModel->getCall(convInfo.callId);
    auto isCallSelected = LRCInstance::getCurrentConvUid() == convInfo.uid;

    if (call.isOutgoing) {
        if (isCallSelected) {
            emit callSetupMainViewRequired(accountId, convInfo.uid);
        }
    } else {
        auto accountProperties = LRCInstance::accountModel().getAccountConfig(selectedAccountId);
        if (!accountProperties.isRendezVous) {
            // App not focused or in different account
            if (QApplication::focusObject() == nullptr || accountId != selectedAccountId) {
                showNotification(accountId, convInfo.uid);
                return;
            }

            const auto& currentConvInfo = LRCInstance::getConversationFromConvUid(
                LRCInstance::getCurrentConvUid());

            // Call in current conversation
            auto currentConvHasCall = callModel->hasCall(currentConvInfo.callId);

            // Check INCOMING / OUTGOING call in current conversation
            if (isCallSelected) {
                if (currentConvHasCall) {
                    auto currentCall = callModel->getCall(currentConvInfo.callId);
                    if (currentCall.status == lrc::api::call::Status::OUTGOING_RINGING) {
                        showNotification(accountId, convInfo.uid);
                        return;
                    } else {
                        emit callSetupMainViewRequired(accountId, convInfo.uid);
                    }
                } else {
                    emit callSetupMainViewRequired(accountId, convInfo.uid);
                }
            } else { // Not current conversation
                if (currentConvHasCall) {
                    auto currentCall = callModel->getCall(currentConvInfo.callId);
                    if ((currentCall.status == lrc::api::call::Status::CONNECTED
                         || currentCall.status == lrc::api::call::Status::IN_PROGRESS)
                        && !accountProperties.autoAnswer) {
                        showNotification(accountId, convInfo.uid);
                        return;
                    }
                }
                emit callSetupMainViewRequired(accountId, convInfo.uid);
            }
        }
    }
    emit callStatusChanged(static_cast<int>(call.status), accountId, convInfo.uid);
    emit LRCInstance::instance().updateSmartList();
}

void
CallAdapter::onShowCallView(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (convInfo.uid.isEmpty()) {
        return;
    }
    updateCall(convInfo.uid, accountId);
}

void
CallAdapter::updateCall(const QString& convUid, const QString& accountId, bool forceCallOnly)
{
    accountId_ = accountId.isEmpty() ? accountId_ : accountId;
    convUid_ = convUid.isEmpty() ? convUid_ : convUid;

    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    if (convInfo.uid.isEmpty()) {
        return;
    }

    auto call = LRCInstance::getCallInfoForConversation(convInfo, forceCallOnly);
    if (!call) {
        return;
    }

    emit callSetupMainViewRequired(accountId_, convUid_);
    updateCallOverlay(convInfo);
    emit previewVisibilityNeedToChange(shouldShowPreview(forceCallOnly));

    if (call->status == lrc::api::call::Status::IN_PROGRESS) {
        LRCInstance::renderer()->addDistantRenderer(call->id);
        LRCInstance::getAccountInfo(accountId_).callModel->setCurrentCall(call->id);
    }
}

bool
CallAdapter::shouldShowPreview(bool force)
{
    bool shouldShowPreview {false};
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);

    if (convInfo.uid.isEmpty()) {
        return shouldShowPreview;
    }
    auto call = LRCInstance::getCallInfoForConversation(convInfo, force);
    if (call) {
        shouldShowPreview = !call->isAudioOnly && !(call->status == lrc::api::call::Status::PAUSED)
                            && !call->videoMuted && call->type != lrc::api::call::Type::CONFERENCE;
    }
    return shouldShowPreview;
}

QJsonObject
CallAdapter::fillParticipantData(QMap<QString, QString> participant)
{
    QJsonObject data;
    data["x"] = participant["x"].toInt();
    data["y"] = participant["y"].toInt();
    data["w"] = participant["w"].toInt();
    data["h"] = participant["h"].toInt();
    data["uri"] = participant["uri"];
    data["active"] = participant["active"] == "true";
    data["videoMuted"] = participant["videoMuted"] == "true";
    data["audioLocalMuted"] = participant["audioLocalMuted"] == "true";
    data["audioModeratorMuted"] = participant["audioModeratorMuted"] == "true";

    auto bestName = participant["uri"];
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
    data["isLocal"] = false;
    if (bestName == accInfo.profileInfo.uri) {
        bestName = tr("me");
        data["isLocal"] = true;
        if (participant["videoMuted"] == "true")
            data["avatar"] = accInfo.profileInfo.avatar;
    } else {
        try {
            auto& contact = LRCInstance::getCurrentAccountInfo()
                    .contactModel->getContact(participant["uri"]);
            bestName = LRCInstance::getCurrentAccountInfo()
                    .contactModel->bestNameForContact(participant["uri"]);
            if (participant["videoMuted"] == "true")
                data["avatar"] = contact.profileInfo.avatar;

        } catch (...) {
        }
    }
    data["bestName"] = bestName;

    return data;
}


QVariantList
CallAdapter::getConferencesInfos()
{
    QVariantList map;
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    if (convInfo.uid.isEmpty())
        return map;
    auto callId = convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
    if (!callId.isEmpty()) {
        try {
            auto call = LRCInstance::getCurrentCallModel()->getCall(callId);
            for (const auto& participant : call.participantsInfos) {
                QJsonObject data = fillParticipantData(participant);
                map.push_back(QVariant(data));
            }
            return map;
        } catch (...) {
        }
    }
    return map;
}

void
CallAdapter::showNotification(const QString& accountId, const QString& convUid)
{
    QString from {};
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!accountId.isEmpty() && !convInfo.uid.isEmpty()) {
        auto& accInfo = LRCInstance::getAccountInfo(accountId);
        if (!convInfo.participants.isEmpty())
            from = accInfo.contactModel->bestNameForContact(convInfo.participants[0]);
    }

    auto onClicked = [this, accountId, convUid = convInfo.uid]() {
        const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
        if (convInfo.uid.isEmpty()) {
            return;
        }
        emit LRCInstance::instance().notificationClicked();
        emit callSetupMainViewRequired(convInfo.accountId, convInfo.uid);
    };
    emit LRCInstance::instance().updateSmartList();
    Utils::showNotification(tr("is calling you"), from, accountId, convUid, onClicked);
}

void
CallAdapter::connectCallModel(const QString& accountId)
{
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);

    QObject::disconnect(callStatusChangedConnection_);
    QObject::disconnect(onParticipantsChangedConnection_);

    onParticipantsChangedConnection_ = QObject::connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::onParticipantsChanged,
        [this, accountId](const QString& confId) {
            auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);
            auto& callModel = accInfo.callModel;
            auto call = callModel->getCall(confId);
            const auto& convInfo = LRCInstance::getConversationFromCallId(confId);
            if (!convInfo.uid.isEmpty()) {
                QVariantList map;
                for (const auto& participant : call.participantsInfos) {
                    QJsonObject data = fillParticipantData(participant);
                    map.push_back(QVariant(data));
                    updateCallOverlay(convInfo);
                }
                emit updateParticipantsInfos(map, accountId, confId);
            }
        });

    callStatusChangedConnection_ = QObject::connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::callStatusChanged,
        [this, accountId](const QString& callId) {
            auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);
            auto& callModel = accInfo.callModel;
            const auto call = callModel->getCall(callId);

            /*
             * Change status label text.
             */
            const auto& convInfo = LRCInstance::getConversationFromCallId(callId);
            if (!convInfo.uid.isEmpty()) {
                emit callStatusChanged(static_cast<int>(call.status), accountId, convInfo.uid);
                updateCallOverlay(convInfo);
            }

            switch (call.status) {
            case lrc::api::call::Status::INVALID:
            case lrc::api::call::Status::INACTIVE:
            case lrc::api::call::Status::ENDED:
            case lrc::api::call::Status::PEER_BUSY:
            case lrc::api::call::Status::TIMEOUT:
            case lrc::api::call::Status::TERMINATING: {
                LRCInstance::renderer()->removeDistantRenderer(callId);
                if (convInfo.uid.isEmpty()) {
                    break;
                }
                /*
                 * If it's a conference, change the smartlist index
                 * to the next remaining participant.
                 */
                bool forceCallOnly {false};
                if (!convInfo.confId.isEmpty()) {
                    auto callList = LRCInstance::getAPI().getConferenceSubcalls(convInfo.confId);
                    if (callList.empty()) {
                        auto lastConference = LRCInstance::instance().poplastConference(
                            convInfo.confId);
                        if (!lastConference.isEmpty()) {
                            callList.append(lastConference);
                            forceCallOnly = true;
                        }
                    }
                    if (callList.isEmpty()) {
                        callList = LRCInstance::getActiveCalls();
                        forceCallOnly = true;
                    }
                    for (const auto& callId : callList) {
                        if (!callModel->hasCall(callId)) {
                            continue;
                        }
                        auto currentCall = callModel->getCall(callId);
                        if (currentCall.status == lrc::api::call::Status::IN_PROGRESS) {
                            const auto& otherConv = LRCInstance::getConversationFromCallId(callId);
                            if (!otherConv.uid.isEmpty() && otherConv.uid != convInfo.uid) {
                                /*
                                 * Reset the call view corresponding accountId, uid.
                                 */
                                LRCInstance::setSelectedConvId(otherConv.uid);
                                updateCall(otherConv.uid, otherConv.accountId, forceCallOnly);
                            }
                        }
                    }
                }
                break;
            }
            case lrc::api::call::Status::CONNECTED:
            case lrc::api::call::Status::IN_PROGRESS: {
                const auto& convInfo = LRCInstance::getConversationFromCallId(callId, accountId);
                if (!convInfo.uid.isEmpty() && convInfo.uid == LRCInstance::getCurrentConvUid()) {
                    accInfo.conversationModel->selectConversation(convInfo.uid);
                }
                updateCall(convInfo.uid, accountId);
                break;
            }
            case lrc::api::call::Status::PAUSED:
                updateCall();
            default:
                break;
            }

            emit LRCInstance::instance().updateSmartList();
        });

    remoteRecordingChangedConnection_ = QObject::connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::remoteRecordingChanged,
        [this](const QString& callId, const QSet<QString>& peerRec, bool state) {
            const auto currentCallId =
                    LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
            if (callId == currentCallId) {
                const auto& accInfo = LRCInstance::getCurrentAccountInfo();
                QStringList peers {};
                for (const auto& uri: peerRec) {
                    auto bestName = accInfo.contactModel->bestNameForContact(uri);
                    if (!bestName.isEmpty()) {
                        peers.append(bestName);
                    }
                }
                if (!peers.isEmpty()) {
                    emit remoteRecordingChanged(peers, true);
                } else if (!state) {
                    emit remoteRecordingChanged(peers, false);
                }
            }
    });
}

void
CallAdapter::sipInputPanelPlayDTMF(const QString& key)
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }

    LRCInstance::getCurrentCallModel()->playDTMF(callId, key);
}

/*
 * For Call Overlay
 */
void
CallAdapter::updateCallOverlay(const lrc::api::conversation::Info& convInfo)
{
    setTime(accountId_, convUid_);
    QObject::disconnect(oneSecondTimer_);
    QObject::connect(oneSecondTimer_, &QTimer::timeout, [this] { setTime(accountId_, convUid_); });
    oneSecondTimer_->start(20);
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);

    auto* call = LRCInstance::getCallInfoForConversation(convInfo);
    if (!call) {
        return;
    }

    bool isPaused = call->status == lrc::api::call::Status::PAUSED;
    bool isAudioOnly = call->isAudioOnly && !isPaused;
    bool isAudioMuted = call->audioMuted && (call->status != lrc::api::call::Status::PAUSED);
    bool isVideoMuted = call->videoMuted && !isPaused && !call->isAudioOnly;
    bool isRecording = isRecordingThisCall();
    auto bestName = convInfo.participants.isEmpty()
                        ? QString()
                        : accInfo.contactModel->bestNameForContact(convInfo.participants[0]);

    emit updateOverlay(isPaused,
                       isAudioOnly,
                       isAudioMuted,
                       isVideoMuted,
                       isRecording,
                       accInfo.profileInfo.type == lrc::api::profile::Type::SIP,
                       !convInfo.confId.isEmpty(),
                       bestName);
}

void
CallAdapter::hangupCall(const QString& uri)
{
    const auto& convInfo = LRCInstance::getConversationFromPeerUri(uri, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
        if (callModel->hasCall(convInfo.callId)) {
            /*
             * Store the last remaining participant of the conference,
             * so we can switch the smartlist index after termination.
             */
            if (!convInfo.confId.isEmpty()) {
                auto callList = LRCInstance::getAPI().getConferenceSubcalls(convInfo.confId);
                if (callList.size() == 2) {
                    for (const auto& cId : callList) {
                        if (cId != convInfo.callId) {
                            LRCInstance::instance().pushlastConference(convInfo.confId, cId);
                        }
                    }
                }
            }
            callModel->hangUp(convInfo.callId);
        }
    }
}

void
CallAdapter::maximizeParticipant(const QString& uri)
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(LRCInstance::getCurrentConvUid(),
                                                                   accountId_);

    auto confId = convInfo.confId;
    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        auto call = callModel->getCall(confId);
        if (call.participantsInfos.size() > 0) {
            for (const auto& participant : call.participantsInfos) {
                if (participant["uri"] == uri) {
                    if (participant["active"] == "false") {
                        callModel->setActiveParticipant(confId, uri);
                        callModel->setConferenceLayout(confId,
                                                       lrc::api::call::Layout::ONE_WITH_SMALL);
                    } else if (participant["y"].toInt() != 0) {
                        callModel->setActiveParticipant(confId, uri);
                        callModel->setConferenceLayout(confId, lrc::api::call::Layout::ONE);
                    } else {
                        callModel->setConferenceLayout(confId, lrc::api::call::Layout::GRID);
                    }
                    return;
                }
            }
        }
    } catch (...) {
    }
}

void
CallAdapter::minimizeParticipant(const QString& uri)
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(LRCInstance::getCurrentConvUid(),
                                                                   accountId_);
    auto confId = convInfo.confId;

    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        auto call = callModel->getCall(confId);
        if (call.participantsInfos.size() > 0) {
            for (const auto& participant : call.participantsInfos) {
                if (participant["uri"] == uri) {
                    if (participant["active"] == "true") {
                        if (participant["y"].toInt() == 0) {
                            callModel->setConferenceLayout(confId,
                                                           lrc::api::call::Layout::ONE_WITH_SMALL);
                        } else {
                            callModel->setConferenceLayout(confId, lrc::api::call::Layout::GRID);
                        }
                    }
                    return;
                }
            }
        }
    } catch (...) {
    }
}

void
CallAdapter::hangUpThisCall()
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
        if (!convInfo.confId.isEmpty() && callModel->hasCall(convInfo.confId)) {
            callModel->hangUp(convInfo.confId);
        } else if (callModel->hasCall(convInfo.callId)) {
            callModel->hangUp(convInfo.callId);
        }
    }
}

bool
CallAdapter::isRecordingThisCall()
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
    auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
    return accInfo.callModel->isRecording(convInfo.confId)
           || accInfo.callModel->isRecording(convInfo.callId);
}

bool
CallAdapter::isCurrentHost() const
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
        try {
            auto confId = convInfo.confId;
            if (confId.isEmpty())
                confId = convInfo.callId;
            auto call = callModel->getCall(confId);
            if (call.participantsInfos.size() == 0) {
                return true;
            } else {
                return !convInfo.confId.isEmpty() && callModel->hasCall(convInfo.confId);
            }
        } catch (...) {
        }
    }
    return true;
}

bool
CallAdapter::participantIsHost(const QString& uri) const
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    if (!convInfo.uid.isEmpty()) {
        auto& accInfo = LRCInstance::getAccountInfo(accountId_);
        auto* callModel = accInfo.callModel.get();
        try {
            if (isCurrentHost()) {
                return uri == accInfo.profileInfo.uri;
            } else {
                auto call = callModel->getCall(convInfo.callId);
                auto peer = call.peerUri.remove("ring:");
                return (uri == peer);
            }
        } catch (...) {
        }
    }
    return true;
}

bool
CallAdapter::isModerator(const QString& uri) const
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    auto confId = convInfo.confId;

    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        return callModel->isModerator(confId, uri);
    } catch (...) {
    }
    return false;
}

bool
CallAdapter::isCurrentModerator() const
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
        try {
            auto call = callModel->getCall(convInfo.callId);
            if (call.participantsInfos.size() == 0) {
                return true;
            } else {
                auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);
                for (const auto& participant : call.participantsInfos) {
                    if (participant["uri"] == accInfo.profileInfo.uri)
                        return participant["isModerator"] == "true";
                }
            }
            return false;
        } catch (...) {
        }
    }
    return true;
}

void
CallAdapter::setModerator(const QString& uri, const bool state)
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    auto confId = convInfo.confId;
    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        callModel->setModerator(confId, uri, state);
    } catch (...) {
    }
}

void
CallAdapter::muteParticipant(const QString& uri, const bool state)
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    auto confId = convInfo.confId;

    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        const auto call = callModel->getCall(confId);
        callModel->muteParticipant(confId, uri, state);
    } catch (...) {
    }
}

CallAdapter::MuteStates
CallAdapter::getMuteState(const QString& uri) const
{
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    auto confId = convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
    try {
        auto call = callModel->getCall(confId);
        if (call.participantsInfos.size() == 0) {
            return MuteStates::UNMUTED;
        } else {
            for (const auto& participant : call.participantsInfos) {
                if (participant["uri"] == uri) {
                    if (participant["audioLocalMuted"] == "true") {
                        if (participant["audioModeratorMuted"] == "true") {
                            return MuteStates::BOTH_MUTED;
                        } else {
                            return MuteStates::LOCAL_MUTED;
                        }
                    } else if (participant["audioModeratorMuted"] == "true") {
                        return MuteStates::MODERATOR_MUTED;
                    }
                    return MuteStates::UNMUTED;
                }
            }
        }
        return MuteStates::UNMUTED;
    } catch (...) {
    }
    return MuteStates::UNMUTED;
}

void
CallAdapter::hangupParticipant(const QString& uri)
{
    auto* callModel = LRCInstance::getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = LRCInstance::getConversationFromConvUid(convUid_);
    auto confId = convInfo.confId;

    if (confId.isEmpty())
        confId = convInfo.callId;
    try {
        const auto call = callModel->getCall(confId);
        callModel->hangupParticipant(confId, uri);
    } catch (...) {
    }
}

void
CallAdapter::holdThisCallToggle()
{
    const auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->togglePause(callId);
    }
    emit showOnHoldLabel(true);
}

void
CallAdapter::muteThisCallToggle()
{
    const auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::AUDIO);
    }
}

void
CallAdapter::recordThisCallToggle()
{
    const auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleAudioRecord(callId);
    }
}

void
CallAdapter::videoPauseThisCallToggle()
{
    const auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::VIDEO);
    }
    emit previewVisibilityNeedToChange(shouldShowPreview(false));
}

void
CallAdapter::setTime(const QString& accountId, const QString& convUid)
{
    const auto callId = LRCInstance::getCallIdForConversationUid(convUid, accountId);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    const auto callInfo = LRCInstance::getCurrentCallModel()->getCall(callId);
    if (callInfo.status == lrc::api::call::Status::IN_PROGRESS
        || callInfo.status == lrc::api::call::Status::PAUSED) {
        auto timeString = LRCInstance::getCurrentCallModel()->getFormattedCallDuration(callId);
        emit updateTimeText(timeString);
    }
}

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

#include "systemtray.h"
#include "utils.h"

#include <QApplication>
#include <QTimer>
#include <QJsonObject>

CallAdapter::CallAdapter(SystemTray* systemTray, LRCInstance* instance, QObject* parent)
    : QmlAdapterBase(instance, parent)
    , oneSecondTimer_(new QTimer(this))
    , systemTray_(systemTray)
{
    accountId_ = lrcInstance_->getCurrAccId();
    if (!accountId_.isEmpty())
        connectCallModel(accountId_);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::showIncomingCallView,
            this,
            &CallAdapter::onShowIncomingCallView);

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::showCallView,
            this,
            &CallAdapter::onShowCallView);

    connect(lrcInstance_, &LRCInstance::currentAccountChanged, this, &CallAdapter::onAccountChanged);

#ifdef Q_OS_LINUX
    // notification responses (gnu/linux currently)
    connect(systemTray_,
            &SystemTray::answerCallActivated,
            [this](const QString& accountId, const QString& convUid) {
                acceptACall(accountId, convUid);
                Q_EMIT lrcInstance_->notificationClicked();
                lrcInstance_->selectConversation(convUid, accountId);
                updateCall(convUid, accountId);
                Q_EMIT lrcInstance_->conversationUpdated(convUid, accountId);
            });
    connect(systemTray_,
            &SystemTray::declineCallActivated,
            [this](const QString& accountId, const QString& convUid) {
                refuseACall(accountId, convUid);
            });
#endif

    connect(&lrcInstance_->behaviorController(),
            &BehaviorController::callStatusChanged,
            this,
            &CallAdapter::onCallStatusChanged);
}

void
CallAdapter::onAccountChanged()
{
    accountId_ = lrcInstance_->getCurrAccId();
    connectCallModel(accountId_);
}

void
CallAdapter::onCallStatusChanged(const QString& accountId, const QString& callId)
{
    set_hasCall(lrcInstance_->hasActiveCall());

    // :/ one timer for all the call overlays
    if (!hasCall_)
        oneSecondTimer_->stop();

#ifdef Q_OS_LINUX
    auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
    auto& callModel = accInfo.callModel;
    const auto call = callModel->getCall(callId);

    const auto& convInfo = lrcInstance_->getConversationFromCallId(callId, accountId);
    if (convInfo.uid.isEmpty())
        return;

    // handle notifications
    if (call.status == lrc::api::call::Status::IN_PROGRESS) {
        // Call answered and in progress; close the notification
        systemTray_->hideNotification(QString("%1;%2").arg(accountId).arg(convInfo.uid));
    } else if (call.status == lrc::api::call::Status::ENDED) {
        // Call ended; close the notification
        if (systemTray_->hideNotification(QString("%1;%2").arg(accountId).arg(convInfo.uid))
            && call.startTime.time_since_epoch().count() == 0) {
            // This was a missed call; show a missed call notification
            // TODO: swarmify(avoid using convInfo.participants[0])
            auto contactPhoto = Utils::contactPhoto(lrcInstance_,
                                                    convInfo.participants[0],
                                                    QSize(50, 50),
                                                    accountId);
            auto& accInfo = lrcInstance_->getAccountInfo(accountId);
            auto from = accInfo.contactModel->bestNameForContact(convInfo.participants[0]);
            auto notifId = QString("%1;%2").arg(accountId).arg(convInfo.uid);
            systemTray_->showNotification(notifId,
                                          tr("Missed call"),
                                          tr("Missed call from %1").arg(from),
                                          NotificationType::CHAT,
                                          Utils::QImageToByteArray(contactPhoto));
        }
    }
#else
    Q_UNUSED(accountId)
    Q_UNUSED(callId)
#endif
}

void
CallAdapter::placeAudioOnlyCall()
{
    const auto convUid = lrcInstance_->get_selectedConvUid();
    if (!convUid.isEmpty()) {
        lrcInstance_->getCurrentConversationModel()->placeAudioOnlyCall(convUid);
    }
}

void
CallAdapter::placeCall()
{
    const auto convUid = lrcInstance_->get_selectedConvUid();
    if (!convUid.isEmpty()) {
        lrcInstance_->getCurrentConversationModel()->placeCall(convUid);
    }
}

void
CallAdapter::hangUpACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        lrcInstance_->getAccountInfo(accountId).callModel->hangUp(convInfo.callId);
    }
}

void
CallAdapter::refuseACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        lrcInstance_->getAccountInfo(accountId).callModel->refuse(convInfo.callId);
    }
}

void
CallAdapter::acceptACall(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        lrcInstance_->getAccountInfo(accountId).callModel->accept(convInfo.callId);
        auto& accInfo = lrcInstance_->getAccountInfo(convInfo.accountId);
        accInfo.callModel->setCurrentCall(convInfo.callId);

        auto contactUri = convInfo.participants.front();
        if (contactUri.isEmpty()) {
            return;
        }
        try {
            auto& contact = accInfo.contactModel->getContact(contactUri);
            if (contact.profileInfo.type == lrc::api::profile::Type::PENDING) {
                lrcInstance_->getCurrentConversationModel()->makePermanent(convInfo.uid);
            }
        } catch (...) {
        }
    }
}

void
CallAdapter::onShowIncomingCallView(const QString& accountId, const QString& convUid)
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (convInfo.uid.isEmpty()) {
        return;
    }
    auto selectedAccountId = lrcInstance_->getCurrAccId();
    auto* callModel = lrcInstance_->getCurrentCallModel();

    // new call
    if (!callModel->hasCall(convInfo.callId)) {
        if (QApplication::focusObject() == nullptr || accountId != selectedAccountId) {
            showNotification(accountId, convInfo.uid);
            return;
        }

        const auto& currentConvInfo = lrcInstance_->getConversationFromConvUid(
            lrcInstance_->get_selectedConvUid());

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
        // select
        lrcInstance_->selectConversation(convInfo.uid, accountId);
        return;
    }

    // this slot has been triggered as a result of either selecting a conversation
    // with an active call, placing a call, or an incoming call for the current
    // or any other conversation
    auto call = callModel->getCall(convInfo.callId);
    auto isCallSelected = lrcInstance_->get_selectedConvUid() == convInfo.uid;

    if (call.isOutgoing) {
        if (isCallSelected) {
            // don't reselect
            Q_EMIT lrcInstance_->conversationUpdated(convInfo.uid, accountId);
        }
    } else {
        auto accountProperties = lrcInstance_->accountModel().getAccountConfig(selectedAccountId);
        if (!accountProperties.isRendezVous) {
            // App not focused or in different account
            if (QApplication::focusObject() == nullptr || accountId != selectedAccountId) {
                showNotification(accountId, convInfo.uid);
                return;
            }

            const auto& currentConvInfo = lrcInstance_->getConversationFromConvUid(
                lrcInstance_->get_selectedConvUid());

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
                        // only update
                        Q_EMIT lrcInstance_->conversationUpdated(convInfo.uid, accountId);
                    }
                } else {
                    // only update
                    Q_EMIT lrcInstance_->conversationUpdated(convInfo.uid, accountId);
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
                // reselect
                lrcInstance_->selectConversation(convInfo.uid, accountId);
            }
        }
    }
    Q_EMIT callStatusChanged(static_cast<int>(call.status), accountId, convInfo.uid);
}

void
CallAdapter::onShowCallView(const QString& accountId, const QString& convUid)
{
    updateCall(convUid, accountId);
    Q_EMIT lrcInstance_->conversationUpdated(convUid, accountId);
}

void
CallAdapter::updateCall(const QString& convUid, const QString& accountId, bool forceCallOnly)
{
    accountId_ = accountId.isEmpty() ? accountId_ : accountId;
    convUid_ = convUid.isEmpty() ? convUid_ : convUid;

    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
    if (convInfo.uid.isEmpty()) {
        return;
    }

    auto call = lrcInstance_->getCallInfoForConversation(convInfo, forceCallOnly);
    if (!call) {
        return;
    }

    updateCallOverlay(convInfo);
    Q_EMIT previewVisibilityNeedToChange(shouldShowPreview(forceCallOnly));

    if (call->status == lrc::api::call::Status::IN_PROGRESS) {
        lrcInstance_->renderer()->addDistantRenderer(call->id);
        lrcInstance_->getAccountInfo(accountId_).callModel->setCurrentCall(call->id);
    }
}

bool
CallAdapter::shouldShowPreview(bool force)
{
    bool shouldShowPreview {false};
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);

    if (convInfo.uid.isEmpty()) {
        return shouldShowPreview;
    }
    auto call = lrcInstance_->getCallInfoForConversation(convInfo, force);
    if (call) {
        shouldShowPreview = !call->isAudioOnly && !(call->status == lrc::api::call::Status::PAUSED)
                            && !call->videoMuted && call->participantsInfos.isEmpty();
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
    data["isContact"] = false;

    auto bestName = participant["uri"];
    auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId_);
    data["isLocal"] = false;
    if (bestName == accInfo.profileInfo.uri) {
        bestName = tr("me");
        data["isLocal"] = true;
        if (participant["videoMuted"] == "true")
            data["avatar"] = accInfo.profileInfo.avatar;
    } else {
        try {
            auto& contact = lrcInstance_->getCurrentAccountInfo().contactModel->getContact(
                participant["uri"]);
            bestName = lrcInstance_->getCurrentAccountInfo().contactModel->bestNameForContact(
                participant["uri"]);
            if (participant["videoMuted"] == "true")
                data["avatar"] = contact.profileInfo.avatar;
            data["isContact"] = true;
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
    if (convInfo.uid.isEmpty())
        return map;
    auto callId = convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
    if (!callId.isEmpty()) {
        try {
            auto call = lrcInstance_->getCurrentCallModel()->getCall(callId);
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
    if (!accountId.isEmpty() && !convInfo.uid.isEmpty()) {
        auto& accInfo = lrcInstance_->getAccountInfo(accountId);
        if (!convInfo.participants.isEmpty())
            from = accInfo.contactModel->bestNameForContact(convInfo.participants[0]);
    }

#ifdef Q_OS_LINUX
    auto contactPhoto = Utils::contactPhoto(lrcInstance_,
                                            convInfo.participants[0],
                                            QSize(50, 50),
                                            accountId);
    auto notifId = QString("%1;%2").arg(accountId).arg(convUid);
    systemTray_->showNotification(notifId,
                                  tr("Incoming call"),
                                  tr("%1 is calling you").arg(from),
                                  NotificationType::CALL,
                                  Utils::QImageToByteArray(contactPhoto));
#else
    auto onClicked = [this, accountId, convUid = convInfo.uid]() {
        Q_EMIT lrcInstance_->notificationClicked();
        const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid, accountId);
        if (convInfo.uid.isEmpty())
            return;
        lrcInstance_->selectConversation(convInfo.uid, accountId);
    };
    systemTray_->showNotification(tr("is calling you"), from, onClicked);
#endif
}

void
CallAdapter::connectCallModel(const QString& accountId)
{
    auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);

    QObject::connect(
        oneSecondTimer_,
        &QTimer::timeout,
        this,
        [this] { setTime(accountId_, convUid_); },
        Qt::UniqueConnection);

    connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::onParticipantsChanged,
        this,
        [this, accountId](const QString& confId) {
            auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
            auto& callModel = accInfo.callModel;
            auto call = callModel->getCall(confId);
            const auto& convInfo = lrcInstance_->getConversationFromCallId(confId);
            if (!convInfo.uid.isEmpty()) {
                QVariantList map;
                for (const auto& participant : call.participantsInfos) {
                    QJsonObject data = fillParticipantData(participant);
                    map.push_back(QVariant(data));
                    updateCallOverlay(convInfo);
                }
                Q_EMIT updateParticipantsInfos(map, accountId, confId);
            }
        },
        Qt::UniqueConnection);

    connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::callStatusChanged,
        this,
        [this, accountId](const QString& callId) {
            auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId);
            auto& callModel = accInfo.callModel;
            const auto call = callModel->getCall(callId);

            /*
             * Change status label text.
             */
            const auto& convInfo = lrcInstance_->getConversationFromCallId(callId);
            if (!convInfo.uid.isEmpty()) {
                Q_EMIT callStatusChanged(static_cast<int>(call.status), accountId, convInfo.uid);
                updateCallOverlay(convInfo);
            }

            switch (call.status) {
            case lrc::api::call::Status::INVALID:
            case lrc::api::call::Status::INACTIVE:
            case lrc::api::call::Status::ENDED:
            case lrc::api::call::Status::PEER_BUSY:
            case lrc::api::call::Status::TIMEOUT:
            case lrc::api::call::Status::TERMINATING: {
                lrcInstance_->renderer()->removeDistantRenderer(callId);
                Q_EMIT lrcInstance_->conversationUpdated(convInfo.uid, accountId);
                if (convInfo.uid.isEmpty()) {
                    break;
                }
                /*
                 * If it's a conference, change the smartlist index
                 * to the next remaining participant.
                 */
                bool forceCallOnly {false};
                if (!convInfo.confId.isEmpty()) {
                    auto callList = lrcInstance_->getConferenceSubcalls(convInfo.confId);
                    if (callList.empty()) {
                        auto lastConference = lrcInstance_->poplastConference(convInfo.confId);
                        if (!lastConference.isEmpty()) {
                            callList.append(lastConference);
                            forceCallOnly = true;
                        }
                    }
                    if (callList.isEmpty()) {
                        callList = lrcInstance_->getActiveCalls();
                        forceCallOnly = true;
                    }
                    for (const auto& callId : callList) {
                        if (!callModel->hasCall(callId)) {
                            continue;
                        }
                        auto currentCall = callModel->getCall(callId);
                        if (currentCall.status == lrc::api::call::Status::IN_PROGRESS) {
                            const auto& otherConv = lrcInstance_->getConversationFromCallId(callId);
                            if (!otherConv.uid.isEmpty() && otherConv.uid != convInfo.uid) {
                                /*
                                 * Reset the call view corresponding accountId, uid.
                                 */
                                lrcInstance_->selectConversation(otherConv.uid);
                                updateCall(otherConv.uid, otherConv.accountId, forceCallOnly);
                            }
                        }
                    }

                    return;
                }
                preventScreenSaver(false);
                break;
            }
            case lrc::api::call::Status::CONNECTED:
            case lrc::api::call::Status::IN_PROGRESS: {
                const auto& convInfo = lrcInstance_->getConversationFromCallId(callId, accountId);
                if (!convInfo.uid.isEmpty() && convInfo.uid == lrcInstance_->get_selectedConvUid()) {
                    accInfo.conversationModel->selectConversation(convInfo.uid);
                }
                updateCall(convInfo.uid, accountId);
                preventScreenSaver(true);
                break;
            }
            case lrc::api::call::Status::PAUSED:
                updateCall();
            default:
                break;
            }
        },
        Qt::UniqueConnection);

    connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::remoteRecordingChanged,
        this,
        [this](const QString& callId, const QSet<QString>& peerRec, bool state) {
            const auto currentCallId = lrcInstance_->getCallIdForConversationUid(convUid_,
                                                                                 accountId_);
            if (callId == currentCallId) {
                const auto& accInfo = lrcInstance_->getCurrentAccountInfo();
                QStringList peers {};
                for (const auto& uri : peerRec) {
                    auto bestName = accInfo.contactModel->bestNameForContact(uri);
                    if (!bestName.isEmpty()) {
                        peers.append(bestName);
                    }
                }
                if (!peers.isEmpty()) {
                    Q_EMIT remoteRecordingChanged(peers, true);
                } else if (!state) {
                    Q_EMIT remoteRecordingChanged(peers, false);
                }
            }
        },
        Qt::UniqueConnection);
}

void
CallAdapter::sipInputPanelPlayDTMF(const QString& key)
{
    auto callId = lrcInstance_->getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }

    lrcInstance_->getCurrentCallModel()->playDTMF(callId, key);
}

/*
 * For Call Overlay
 */
void
CallAdapter::updateCallOverlay(const lrc::api::conversation::Info& convInfo)
{
    setTime(accountId_, convUid_);
    oneSecondTimer_->start(1000);
    auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId_);

    auto* call = lrcInstance_->getCallInfoForConversation(convInfo);
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

    Q_EMIT updateOverlay(isPaused,
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
    const auto& convInfo = lrcInstance_->getConversationFromPeerUri(uri, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
        if (callModel->hasCall(convInfo.callId)) {
            /*
             * Store the last remaining participant of the conference,
             * so we can switch the smartlist index after termination.
             */
            if (!convInfo.confId.isEmpty()) {
                auto callList = lrcInstance_->getConferenceSubcalls(convInfo.confId);
                if (callList.size() == 2) {
                    for (const auto& cId : callList) {
                        if (cId != convInfo.callId) {
                            lrcInstance_->pushlastConference(convInfo.confId, cId);
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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo
        = lrcInstance_->getConversationFromConvUid(lrcInstance_->get_selectedConvUid(), accountId_);

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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo
        = lrcInstance_->getConversationFromConvUid(lrcInstance_->get_selectedConvUid(), accountId_);
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_, accountId_);
    auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId_);
    return accInfo.callModel->isRecording(convInfo.confId)
           || accInfo.callModel->isRecording(convInfo.callId);
}

bool
CallAdapter::isCurrentHost() const
{
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_, accountId_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
    if (!convInfo.uid.isEmpty()) {
        auto& accInfo = lrcInstance_->getAccountInfo(accountId_);
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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
    if (!convInfo.uid.isEmpty()) {
        auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
        try {
            auto call = callModel->getCall(convInfo.callId);
            if (call.participantsInfos.size() == 0) {
                return true;
            } else {
                auto& accInfo = lrcInstance_->accountModel().getAccountInfo(accountId_);
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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
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
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
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
    auto* callModel = lrcInstance_->getAccountInfo(accountId_).callModel.get();
    const auto& convInfo = lrcInstance_->getConversationFromConvUid(convUid_);
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
    const auto callId = lrcInstance_->getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = lrcInstance_->getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->togglePause(callId);
    }
    Q_EMIT showOnHoldLabel(true);
}

void
CallAdapter::muteThisCallToggle()
{
    const auto callId = lrcInstance_->getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = lrcInstance_->getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::AUDIO);
    }
}

void
CallAdapter::recordThisCallToggle()
{
    const auto callId = lrcInstance_->getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = lrcInstance_->getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleAudioRecord(callId);
    }
}

void
CallAdapter::videoPauseThisCallToggle()
{
    const auto callId = lrcInstance_->getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto* callModel = lrcInstance_->getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::VIDEO);
    }
    Q_EMIT previewVisibilityNeedToChange(shouldShowPreview(false));
}

void
CallAdapter::setTime(const QString& accountId, const QString& convUid)
{
    const auto callId = lrcInstance_->getCallIdForConversationUid(convUid, accountId);
    if (callId.isEmpty() || !lrcInstance_->getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    const auto callInfo = lrcInstance_->getCurrentCallModel()->getCall(callId);
    if (callInfo.status == lrc::api::call::Status::IN_PROGRESS
        || callInfo.status == lrc::api::call::Status::PAUSED) {
        auto timeString = lrcInstance_->getCurrentCallModel()->getFormattedCallDuration(callId);
        Q_EMIT updateTimeText(timeString);
    }
}

void
CallAdapter::preventScreenSaver(bool state)
{
    if (state) {
        if (!screenSaver.isInhibited())
            screenSaver.inhibit();
    } else if (screenSaver.isInhibited()) {
        screenSaver.uninhibit();
    }
};

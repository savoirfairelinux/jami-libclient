/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Anthony Léonard <anthony.leonard@savoirfairelinux.com>
 * Author: Olivier Soldano <olivier.soldano@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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

#include "globalsystemtray.h"
#include "utils.h"

CallAdapter::CallAdapter(QObject *parent)
    : QmlAdapterBase(parent)
    , oneSecondTimer_(new QTimer(this))
{}

CallAdapter::~CallAdapter() {}

void
CallAdapter::initQmlObject()
{
    connectCallStatusChanged(LRCInstance::getCurrAccId());

    connect(&LRCInstance::behaviorController(),
            &BehaviorController::showIncomingCallView,
            this,
            &CallAdapter::slotShowIncomingCallView);
    connect(&LRCInstance::behaviorController(),
            &BehaviorController::showCallView,
            this,
            &CallAdapter::slotShowCallView);
}

void
CallAdapter::placeAudioOnlyCall()
{
    auto convInfo = LRCInstance::getCurrentConversation();
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getCurrentConversationModel()->placeAudioOnlyCall(convInfo.uid);
    }
}

void
CallAdapter::placeCall()
{
    auto convInfo = LRCInstance::getCurrentConversation();
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getCurrentConversationModel()->placeCall(convInfo.uid);
    }
}

void
CallAdapter::hangUpACall(const QString &accountId, const QString &convUid)
{
    auto convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->hangUp(convInfo.callId);
    }
}

void
CallAdapter::refuseACall(const QString &accountId, const QString &convUid)
{
    auto convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->refuse(convInfo.callId);
    }
}

void
CallAdapter::acceptACall(const QString &accountId, const QString &convUid)
{
    emit incomingCallNeedToSetupMainView(accountId, convUid);
    auto convInfo = LRCInstance::getConversationFromConvUid(convUid, accountId);
    if (!convInfo.uid.isEmpty()) {
        LRCInstance::getAccountInfo(accountId).callModel->accept(convInfo.callId);
        auto &accInfo = LRCInstance::getAccountInfo(convInfo.accountId);
        accInfo.callModel->setCurrentCall(convInfo.callId);
    }
}

void
CallAdapter::slotShowIncomingCallView(const QString &accountId, const conversation::Info &convInfo)
{
    auto callModel = LRCInstance::getCurrentCallModel();

    if (!callModel->hasCall(convInfo.callId)) {
        /*
         * Connection to close potential incoming call page when it is not current account.
         */
        auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId);

        QObject::disconnect(closeIncomingCallPageConnection_);

        closeIncomingCallPageConnection_
            = QObject::connect(accInfo.callModel.get(),
                               &lrc::api::NewCallModel::callStatusChanged,
                               [this, accountId, uid = convInfo.uid](const QString &callId) {
                                   auto &accInfo = LRCInstance::accountModel().getAccountInfo(
                                       accountId);
                                   auto &callModel = accInfo.callModel;
                                   auto call = callModel->getCall(callId);

                                   switch (call.status) {
                                   case lrc::api::call::Status::INVALID:
                                   case lrc::api::call::Status::INACTIVE:
                                   case lrc::api::call::Status::ENDED:
                                   case lrc::api::call::Status::PEER_BUSY:
                                   case lrc::api::call::Status::TIMEOUT:
                                   case lrc::api::call::Status::TERMINATING: {
                                       if (!uid.isEmpty())
                                           emit closePotentialIncomingCallPageWindow(accountId, uid);
                                       break;
                                   }
                                   default:
                                       break;
                                   }

                                   emit updateConversationSmartList();
                                   QObject::disconnect(closeIncomingCallPageConnection_);
                               });
        /*
         * Show incoming call page only.
         */
        emit showIncomingCallPage(accountId, convInfo.uid);
        return;
    }

    auto call = callModel->getCall(convInfo.callId);
    auto isCallSelected = LRCInstance::getCurrentConvUid() == convInfo.uid;

    if (call.isOutgoing) {
        if (isCallSelected) {
            emit showOutgoingCallPage(accountId, convInfo.uid);
            emit showCallStack(accountId, convInfo.uid);
        }
    } else {
        auto selectedAccountId = LRCInstance::getCurrentAccountInfo().id;
        auto accountProperties = LRCInstance::accountModel().getAccountConfig(selectedAccountId);
        if (accountProperties.autoAnswer) {
            /*
             * TODO: Auto answer
             */
        } else {
            emit showIncomingCallPage(accountId, convInfo.uid);
        }
    }

    emit callStatusChanged(lrc::api::call::to_string(call.status), accountId, convInfo.uid);

    emit updateConversationSmartList();
}

void
CallAdapter::slotShowCallView(const QString &accountId, const lrc::api::conversation::Info &convInfo)
{
    updateCall(convInfo.uid, accountId);
}

void
CallAdapter::updateCall(const QString &convUid, const QString &accountId, bool forceCallOnly)
{
    accountId_ = accountId.isEmpty() ? accountId_ : accountId;
    convUid_ = convUid.isEmpty() ? convUid_ : convUid;

    auto convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
    if (convInfo.uid.isEmpty()) {
        return;
    }

    auto call = LRCInstance::getCallInfoForConversation(convInfo, forceCallOnly);
    if (!call) {
        return;
    }

    if (call->isAudioOnly) {
        emit showAudioCallPage(accountId_, convUid_);
    } else {
        emit showVideoCallPage(accountId_, convUid_, call->id);
    }

    updateCallOverlay(convInfo);

    /*
     * Preview.
     */
    emit previewVisibilityNeedToChange(shouldShowPreview(forceCallOnly));

    emit showCallStack(accountId_, convUid_);
}

bool
CallAdapter::shouldShowPreview(bool force)
{
    bool shouldShowPreview{false};
    auto convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
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

void
CallAdapter::connectCallStatusChanged(const QString &accountId)
{
    auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId);

    QObject::disconnect(callStatusChangedConnection_);

    callStatusChangedConnection_ = QObject::connect(
        accInfo.callModel.get(),
        &lrc::api::NewCallModel::callStatusChanged,
        [this, accountId](const QString &callId) {
            auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId);
            auto &callModel = accInfo.callModel;
            auto call = callModel->getCall(callId);

            /*
             * Change status label text.
             */
            auto convInfo = LRCInstance::getConversationFromCallId(callId);
            if (!convInfo.uid.isEmpty()) {
                emit callStatusChanged(lrc::api::call::to_string(call.status),
                                       accountId,
                                       convInfo.uid);
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
                bool forceCallOnly{false};
                if (!convInfo.confId.isEmpty()) {
                    auto callList = LRCInstance::getAPI().getConferenceSubcalls(convInfo.confId);
                    if (callList.empty()) {
                        auto lastConferencee = LRCInstance::instance().popLastConferencee(
                            convInfo.confId);
                        callList.append(lastConferencee);
                        forceCallOnly = true;
                    }
                    for (const auto &callId : callList) {
                        if (!callModel->hasCall(callId)) {
                            continue;
                        }
                        auto otherConv = LRCInstance::getConversationFromCallId(callId);
                        if (!otherConv.uid.isEmpty() && otherConv.uid != convInfo.uid) {
                            /*
                             * Reset the call view corresponding accountId, uid.
                             */
                            LRCInstance::setSelectedConvId(otherConv.uid);
                            showCallStack(otherConv.accountId, otherConv.uid, true);
                            updateCall(otherConv.uid, otherConv.accountId, forceCallOnly);
                        }
                    }
                } else {
                    emit closeCallStack(accountId, convInfo.uid);
                    emit closePotentialIncomingCallPageWindow(accountId, convInfo.uid);
                }

                break;
            }
            case lrc::api::call::Status::CONNECTED:
            case lrc::api::call::Status::IN_PROGRESS: {
                auto convInfo = LRCInstance::getConversationFromCallId(callId, accountId);
                if (!convInfo.uid.isEmpty() && convInfo.uid == LRCInstance::getCurrentConvUid()) {
                    accInfo.conversationModel->selectConversation(convInfo.uid);
                }
                LRCInstance::renderer()->addDistantRenderer(callId);
                updateCall();
                LRCInstance::getAccountInfo(accountId).callModel->setCurrentCall(callId);
                break;
            }
            case lrc::api::call::Status::PAUSED:
                updateCall();
            default:
                break;
            }

            emit updateConversationSmartList();
        });
}

/*
 * For Call Overlay
 */
void
CallAdapter::updateCallOverlay(const lrc::api::conversation::Info &convInfo)
{
    setTime(accountId_, convUid_);
    QObject::disconnect(oneSecondTimer_);
    QObject::connect(oneSecondTimer_, &QTimer::timeout, [this] { setTime(accountId_, convUid_); });
    oneSecondTimer_->start(20);

    auto &accInfo = LRCInstance::accountModel().getAccountInfo(accountId_);

    auto call = LRCInstance::getCallInfoForConversation(convInfo);
    if (!call) {
        return;
    }

    bool isPaused = call->status == lrc::api::call::Status::PAUSED;
    bool isAudioOnly = call->isAudioOnly && !isPaused;
    bool isAudioMuted = call->audioMuted && (call->status != lrc::api::call::Status::PAUSED);
    bool isVideoMuted = call->videoMuted && !isPaused && !call->isAudioOnly;
    bool isRecording = accInfo.callModel->isRecording(convInfo.callId);

    emit updateOverlay(isPaused,
                       isAudioOnly,
                       isAudioMuted,
                       isVideoMuted,
                       isRecording,
                       accInfo.profileInfo.type == lrc::api::profile::Type::SIP,
                       !convInfo.confId.isEmpty(),
                       Utils::bestNameForConversation(convInfo,
                                                      *LRCInstance::getCurrentConversationModel()));
}

void
CallAdapter::hangUpThisCall()
{
    auto convInfo = LRCInstance::getConversationFromConvUid(convUid_, accountId_);
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
                    for (const auto &cId : callList) {
                        if (cId != convInfo.callId) {
                            LRCInstance::instance().pushLastConferencee(convInfo.confId, cId);
                        }
                    }
                }
            }
            callModel->hangUp(convInfo.callId);
        }
    }
}

void
CallAdapter::holdThisCallToggle()
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->togglePause(callId);
    }
    emit showOnHoldLabel(true);
}

void
CallAdapter::muteThisCallToggle()
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::AUDIO);
    }
}

void
CallAdapter::recordThisCallToggle()
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleAudioRecord(callId);
    }
}

void
CallAdapter::videoPauseThisCallToggle()
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid_, accountId_);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto callModel = LRCInstance::getCurrentCallModel();
    if (callModel->hasCall(callId)) {
        callModel->toggleMedia(callId, lrc::api::NewCallModel::Media::VIDEO);
    }
    emit previewVisibilityNeedToChange(shouldShowPreview(false));
}

void
CallAdapter::setTime(const QString &accountId, const QString &convUid)
{
    auto callId = LRCInstance::getCallIdForConversationUid(convUid, accountId);
    if (callId.isEmpty() || !LRCInstance::getCurrentCallModel()->hasCall(callId)) {
        return;
    }
    auto callInfo = LRCInstance::getCurrentCallModel()->getCall(callId);
    if (callInfo.status == lrc::api::call::Status::IN_PROGRESS
        || callInfo.status == lrc::api::call::Status::PAUSED) {
        auto timeString = LRCInstance::getCurrentCallModel()->getFormattedCallDuration(callId);
        emit updateTimeText(timeString);
    }
}
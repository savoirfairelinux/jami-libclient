/*
 * Copyright (C) 2019-2021 by Savoir-faire Linux
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

#include "lrcinstance.h"

#include <QBuffer>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QRegularExpression>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

LRCInstance::LRCInstance(migrateCallback willMigrateCb,
                         migrateCallback didMigrateCb,
                         const QString& updateUrl,
                         ConnectivityMonitor* connectivityMonitor,
                         bool muteDring)
    : lrc_(std::make_unique<Lrc>(willMigrateCb, didMigrateCb, muteDring))
    , renderer_(std::make_unique<RenderManager>(lrc_->getAVModel()))
    , updateManager_(std::make_unique<UpdateManager>(updateUrl, connectivityMonitor, this))
{
    lrc_->holdConferences = false;
};

VectorString
LRCInstance::getConferenceSubcalls(const QString& callId)
{
    return lrc_->getConferenceSubcalls(callId);
}

RenderManager*
LRCInstance::renderer()
{
    return renderer_.get();
}

UpdateManager*
LRCInstance::getUpdateManager()
{
    return updateManager_.get();
}

void
LRCInstance::connectivityChanged()
{
    lrc_->connectivityChanged();
}

NewAccountModel&
LRCInstance::accountModel()
{
    return lrc_->getAccountModel();
}

BehaviorController&
LRCInstance::behaviorController()
{
    return lrc_->getBehaviorController();
}

DataTransferModel&
LRCInstance::dataTransferModel()
{
    return lrc_->getDataTransferModel();
}

AVModel&
LRCInstance::avModel()
{
    return lrc_->getAVModel();
}

PluginModel&
LRCInstance::pluginModel()
{
    return lrc_->getPluginModel();
}

bool
LRCInstance::isConnected()
{
    return lrc_->isConnected();
}

VectorString
LRCInstance::getActiveCalls()
{
    return lrc_->activeCalls();
}

const account::Info&
LRCInstance::getAccountInfo(const QString& accountId)
{
    return accountModel().getAccountInfo(accountId);
}

const account::Info&
LRCInstance::getCurrentAccountInfo()
{
    return getAccountInfo(getCurrentAccountId());
}

profile::Type
LRCInstance::getCurrentAccountType()
{
    if (currentAccountType_ == profile::Type::INVALID) {
        try {
            currentAccountType_ = getCurrentAccountInfo().profileInfo.type;
        } catch (...) {
            qDebug() << "Cannot find current account info";
        }
    }
    return currentAccountType_;
}

bool
LRCInstance::hasActiveCall(bool withVideo)
{
    auto activeCalls = lrc_->activeCalls();
    auto accountList = accountModel().getAccountList();
    bool result = false;
    for (const auto& callId : activeCalls) {
        for (const auto& accountId : accountList) {
            auto& accountInfo = accountModel().getAccountInfo(accountId);
            if (withVideo) {
                if (accountInfo.callModel->hasCall(callId))
                    return true;
            } else {
                if (accountInfo.callModel->hasCall(callId)) {
                    auto call = accountInfo.callModel->getCall(callId);
                    result |= !(call.isAudioOnly || call.videoMuted);
                }
            }
        }
    }
    return result;
}

QString
LRCInstance::getCallIdForConversationUid(const QString& convUid, const QString& accountId)
{
    const auto& convInfo = getConversationFromConvUid(convUid, accountId);
    if (convInfo.uid.isEmpty()) {
        return {};
    }
    return convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
}

const call::Info*
LRCInstance::getCallInfo(const QString& callId, const QString& accountId)
{
    try {
        auto& accInfo = accountModel().getAccountInfo(accountId);
        if (!accInfo.callModel->hasCall(callId)) {
            return nullptr;
        }
        return &accInfo.callModel->getCall(callId);
    } catch (...) {
        return nullptr;
    }
}

const call::Info*
LRCInstance::getCallInfoForConversation(const conversation::Info& convInfo, bool forceCallOnly)
{
    try {
        auto accountId = convInfo.accountId;
        auto& accInfo = accountModel().getAccountInfo(accountId);
        auto callId = forceCallOnly
                          ? convInfo.callId
                          : (convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId);
        if (!accInfo.callModel->hasCall(callId)) {
            return nullptr;
        }
        return &accInfo.callModel->getCall(callId);
    } catch (...) {
        return nullptr;
    }
}

const conversation::Info&
LRCInstance::getConversationFromConvUid(const QString& convUid, const QString& accountId)
{
    auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                       : getCurrentAccountId());
    auto& convModel = accInfo.conversationModel;
    return convModel->getConversationForUid(convUid).value_or(invalid);
}

const conversation::Info&
LRCInstance::getConversationFromPeerUri(const QString& peerUri, const QString& accountId)
{
    auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                       : getCurrentAccountId());
    auto& convModel = accInfo.conversationModel;
    return convModel->getConversationForPeerUri(peerUri).value_or(invalid);
}

const conversation::Info&
LRCInstance::getConversationFromCallId(const QString& callId, const QString& accountId)
{
    auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                       : getCurrentAccountId());
    auto& convModel = accInfo.conversationModel;
    return convModel->getConversationForCallId(callId).value_or(invalid);
}

ConversationModel*
LRCInstance::getCurrentConversationModel()
{
    try {
        const auto& accInfo = getCurrentAccountInfo();
        return accInfo.conversationModel.get();
    } catch (...) {
        return nullptr;
    }
}

NewCallModel*
LRCInstance::getCurrentCallModel()
{
    try {
        const auto& accInfo = getCurrentAccountInfo();
        return accInfo.callModel.get();
    } catch (...) {
        return nullptr;
    }
}

ContactModel*
LRCInstance::getCurrentContactModel()
{
    try {
        const auto& accInfo = getCurrentAccountInfo();
        return accInfo.contactModel.get();
    } catch (...) {
        return nullptr;
    }
}

const QString&
LRCInstance::getCurrentAccountId()
{
    if (currentAccountId_.isEmpty()) {
        auto accountList = accountModel().getAccountList();
        if (accountList.size())
            currentAccountId_ = accountList.at(0);
    }
    return currentAccountId_;
}

void
LRCInstance::setCurrentAccountId(const QString& accountId)
{
    if (accountId == currentAccountId_)
        return; // No need to select current selected account

    auto newAccountType = getAccountInfo(accountId).profileInfo.type;
    bool accountTypeChanged = getCurrentAccountType() != newAccountType;

    currentAccountId_ = accountId;

    // Last selected account should be set as preferred.
    accountModel().setTopAccount(accountId);

    if (accountTypeChanged) {
        currentAccountType_ = newAccountType;
        Q_EMIT currentAccountTypeChanged(newAccountType);
    }
    Q_EMIT currentAccountIdChanged(accountId);
}

int
LRCInstance::getCurrentAccountIndex()
{
    for (int i = 0; i < accountModel().getAccountList().size(); i++) {
        if (accountModel().getAccountList()[i] == getCurrentAccountId()) {
            return i;
        }
    }
    return -1;
}

void
LRCInstance::setAvatarForAccount(const QPixmap& avatarPixmap, const QString& accountID)
{
    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    avatarPixmap.save(&bu, "PNG");
    auto str = QString::fromLocal8Bit(ba.toBase64());
    accountModel().setAvatar(accountID, str);
}

void
LRCInstance::setCurrAccAvatar(const QPixmap& avatarPixmap)
{
    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    avatarPixmap.save(&bu, "PNG");
    auto str = QString::fromLocal8Bit(ba.toBase64());
    accountModel().setAvatar(getCurrentAccountId(), str);
}

void
LRCInstance::setCurrAccAvatar(const QString& avatar)
{
    accountModel().setAvatar(getCurrentAccountId(), avatar);
}

void
LRCInstance::setCurrAccDisplayName(const QString& displayName)
{
    auto accountId = getCurrentAccountId();
    accountModel().setAlias(accountId, displayName);
    /*
     * Force save to .yml.
     */
    auto confProps = accountModel().getAccountConfig(accountId);
    accountModel().setAccountConfig(accountId, confProps);
}

const account::ConfProperties_t&
LRCInstance::getCurrAccConfig()
{
    return getCurrentAccountInfo().confProperties;
}

int
LRCInstance::indexOf(const QString& convId)
{
    auto& convs = getCurrentConversationModel()->getConversations();
    auto it = std::find_if(convs.begin(),
                           convs.end(),
                           [convId](const lrc::api::conversation::Info& conv) {
                               return conv.uid == convId;
                           });
    return it != convs.end() ? std::distance(convs.begin(), it) : -1;
}

void
LRCInstance::subscribeToDebugReceived()
{
    lrc_->subscribeToDebugReceived();
}

void
LRCInstance::startAudioMeter(bool async)
{
    auto f = [this] {
        if (!getActiveCalls().size()) {
            avModel().startAudioDevice();
        }
        avModel().setAudioMeterState(true);
    };
    if (async) {
        QtConcurrent::run(f);
    } else {
        f();
    }
}

void
LRCInstance::stopAudioMeter(bool async)
{
    auto f = [this] {
        if (!getActiveCalls().size()) {
            avModel().stopAudioDevice();
        }
        avModel().setAudioMeterState(false);
    };
    if (async) {
        QtConcurrent::run(f);
    } else {
        f();
    }
}

QString
LRCInstance::getContentDraft(const QString& convUid, const QString& accountId)
{
    auto draftKey = accountId + "_" + convUid;
    return contentDrafts_[draftKey];
}

void
LRCInstance::setContentDraft(const QString& convUid,
                             const QString& accountId,
                             const QString& content)
{
    auto draftKey = accountId + "_" + convUid;
    contentDrafts_[draftKey] = content;
    // this signal is only needed to update the current smartlist
    Q_EMIT draftSaved(convUid);
}

void
LRCInstance::pushlastConference(const QString& confId, const QString& callId)
{
    lastConferences_[confId] = callId;
}

QString
LRCInstance::poplastConference(const QString& confId)
{
    QString callId = {};
    auto iter = lastConferences_.find(confId);
    if (iter != lastConferences_.end()) {
        callId = iter.value();
        lastConferences_.erase(iter);
    }
    return callId;
}

void
LRCInstance::selectConversation(const QString& convId, const QString& accountId)
{
    // reselection can be used to update the conversation
    if (convId == selectedConvUid_ && accountId == currentAccountId_) {
        Q_EMIT conversationUpdated(convId, accountId);
        return;
    }
    // if the account is not currently selected, do that first, then
    // proceed to select the conversation
    if (!accountId.isEmpty() && accountId != getCurrentAccountId()) {
        Utils::oneShotConnect(this, &LRCInstance::currentAccountIdChanged, [this, convId] {
            set_selectedConvUid(convId);
        });
        setCurrentAccountId(accountId);
        return;
    }
    set_selectedConvUid(convId);
}

void
LRCInstance::deselectConversation()
{
    set_selectedConvUid();
}

void
LRCInstance::finish()
{
    renderer_.reset();
    lrc_.reset();
}

void
LRCInstance::monitor(bool continuous)
{
    lrc_->monitor(continuous);
}

/*!
 * Copyright (C) 2019-2020 by Savoir-faire Linux
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

#pragma once

#ifdef _MSC_VER
#undef ERROR
#endif

#include "updatemanager.h"
#include "rendermanager.h"
#include "utils.h"

#include "api/account.h"
#include "api/avmodel.h"
#include "api/pluginmodel.h"
#include "api/behaviorcontroller.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/conversation.h"
#include "api/conversationmodel.h"
#include "api/datatransfermodel.h"
#include "api/lrc.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"
#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"
#include "api/peerdiscoverymodel.h"

#include <QBuffer>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QRegularExpression>
#include <QSettings>
#include <QtConcurrent/QtConcurrent>

#include <memory>

class ConnectivityMonitor;

using namespace lrc::api;

using migrateCallback = std::function<void()>;
using getConvPredicate = std::function<bool(const conversation::Info& conv)>;

class LRCInstance : public QObject
{
    Q_OBJECT

public:
    LRCInstance(migrateCallback willMigrateCb = {},
                migrateCallback didMigrateCb = {},
                const QString& updateUrl = {},
                ConnectivityMonitor* connectivityMonitor = {})
    {
        lrc_ = std::make_unique<Lrc>(willMigrateCb, didMigrateCb);
        renderer_ = std::make_unique<RenderManager>(lrc_->getAVModel());
        updateManager_ = std::make_unique<UpdateManager>(updateUrl, connectivityMonitor, this);
    };

    Lrc& getAPI()
    {
        return *(lrc_);
    }

    RenderManager* renderer()
    {
        return renderer_.get();
    }

    UpdateManager* getUpdateManager()
    {
        return updateManager_.get();
    }

    void connectivityChanged()
    {
        lrc_->connectivityChanged();
    }

    NewAccountModel& accountModel()
    {
        return lrc_->getAccountModel();
    }

    BehaviorController& behaviorController()
    {
        return lrc_->getBehaviorController();
    }

    DataTransferModel& dataTransferModel()
    {
        return lrc_->getDataTransferModel();
    }

    AVModel& avModel()
    {
        return lrc_->getAVModel();
    }

    PluginModel& pluginModel()
    {
        return lrc_->getPluginModel();
    }

    bool isConnected()
    {
        return lrc_->isConnected();
    }

    VectorString getActiveCalls()
    {
        return lrc_->activeCalls();
    }

    const account::Info& getAccountInfo(const QString& accountId)
    {
        return accountModel().getAccountInfo(accountId);
    }

    const account::Info& getCurrentAccountInfo()
    {
        return getAccountInfo(getCurrAccId());
    }

    bool hasVideoCall()
    {
        auto activeCalls = lrc_->activeCalls();
        auto accountList = accountModel().getAccountList();
        bool result = false;
        for (const auto& callId : activeCalls) {
            for (const auto& accountId : accountList) {
                auto& accountInfo = accountModel().getAccountInfo(accountId);
                if (accountInfo.callModel->hasCall(callId)) {
                    auto call = accountInfo.callModel->getCall(callId);
                    result |= !(call.isAudioOnly || call.videoMuted);
                }
            }
        }
        return result;
    }

    QString getCallIdForConversationUid(const QString& convUid, const QString& accountId)
    {
        const auto& convInfo = getConversationFromConvUid(convUid, accountId);
        if (convInfo.uid.isEmpty()) {
            return {};
        }
        return convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
    }

    const call::Info* getCallInfo(const QString& callId, const QString& accountId)
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

    const call::Info* getCallInfoForConversation(const conversation::Info& convInfo,
                                                 bool forceCallOnly = {})
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

    const conversation::Info& getConversationFromConvUid(const QString& convUid,
                                                         const QString& accountId = {})
    {
        auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                           : getCurrAccId());
        auto& convModel = accInfo.conversationModel;
        return convModel->getConversationForUid(convUid).value_or(invalid);
    }

    const conversation::Info& getConversationFromPeerUri(const QString& peerUri,
                                                         const QString& accountId = {})
    {
        auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                           : getCurrAccId());
        auto& convModel = accInfo.conversationModel;
        return convModel->getConversationForPeerUri(peerUri).value_or(invalid);
    }

    const conversation::Info& getConversationFromCallId(const QString& callId,
                                                        const QString& accountId = {})
    {
        auto& accInfo = accountModel().getAccountInfo(!accountId.isEmpty() ? accountId
                                                                           : getCurrAccId());
        auto& convModel = accInfo.conversationModel;
        return convModel->getConversationForCallId(callId).value_or(invalid);
    }

    ConversationModel* getCurrentConversationModel()
    {
        return getCurrentAccountInfo().conversationModel.get();
    }

    NewCallModel* getCurrentCallModel()
    {
        return getCurrentAccountInfo().callModel.get();
    }

    const QString& getCurrAccId()
    {
        if (selectedAccountId_.isEmpty()) {
            auto accountList = accountModel().getAccountList();
            if (accountList.size())
                selectedAccountId_ = accountList.at(0);
        }
        return selectedAccountId_;
    }

    void setSelectedAccountId(const QString& accountId = {})
    {
        if (accountId == selectedAccountId_)
            return; // No need to select current selected account

        selectedAccountId_ = accountId;

        // Last selected account should be set as preferred.
        accountModel().setTopAccount(accountId);

        Q_EMIT currentAccountChanged();
    }

    const QString& getCurrentConvUid()
    {
        return selectedConvUid_;
    }

    void setSelectedConvId(const QString& convUid = {})
    {
        selectedConvUid_ = convUid;
    }

    void reset(bool newInstance = false)
    {
        if (newInstance) {
            renderer_.reset(new RenderManager(avModel()));
            lrc_.reset(new Lrc());
        } else {
            renderer_.reset();
            lrc_.reset();
        }
    }

    int getCurrentAccountIndex()
    {
        for (int i = 0; i < accountModel().getAccountList().size(); i++) {
            if (accountModel().getAccountList()[i] == getCurrAccId()) {
                return i;
            }
        }
        return -1;
    }

    void setAvatarForAccount(const QPixmap& avatarPixmap, const QString& accountID)
    {
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        avatarPixmap.save(&bu, "PNG");
        auto str = QString::fromLocal8Bit(ba.toBase64());
        accountModel().setAvatar(accountID, str);
    }

    void setCurrAccAvatar(const QPixmap& avatarPixmap)
    {
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        avatarPixmap.save(&bu, "PNG");
        auto str = QString::fromLocal8Bit(ba.toBase64());
        accountModel().setAvatar(getCurrAccId(), str);
    }

    void setCurrAccAvatar(const QString& avatar)
    {
        accountModel().setAvatar(getCurrAccId(), avatar);
    }

    void setCurrAccDisplayName(const QString& displayName)
    {
        auto accountId = getCurrAccId();
        accountModel().setAlias(accountId, displayName);
        /*
         * Force save to .yml.
         */
        auto confProps = accountModel().getAccountConfig(accountId);
        accountModel().setAccountConfig(accountId, confProps);
    }

    const account::ConfProperties_t& getCurrAccConfig()
    {
        return getCurrentAccountInfo().confProperties;
    }

    void subscribeToDebugReceived()
    {
        lrc_->subscribeToDebugReceived();
    }

    void startAudioMeter(bool async)
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

    void stopAudioMeter(bool async)
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

    QString getContentDraft(const QString& convUid, const QString& accountId)
    {
        auto draftKey = accountId + "_" + convUid;
        return contentDrafts_[draftKey];
    }

    void setContentDraft(const QString& convUid, const QString& accountId, const QString& content)
    {
        auto draftKey = accountId + "_" + convUid;
        contentDrafts_[draftKey] = content;
    }

    void pushlastConference(const QString& confId, const QString& callId)
    {
        lastConferences_[confId] = callId;
    }

    QString poplastConference(const QString& confId)
    {
        QString callId = {};
        auto iter = lastConferences_.find(confId);
        if (iter != lastConferences_.end()) {
            callId = iter.value();
            lastConferences_.erase(iter);
        }
        return callId;
    }

Q_SIGNALS:
    void accountListChanged();
    void currentAccountChanged();
    void restoreAppRequested();
    void notificationClicked();
    void updateSmartList();
    void quitEngineRequested();

private:
    std::unique_ptr<Lrc> lrc_;
    std::unique_ptr<RenderManager> renderer_;
    std::unique_ptr<UpdateManager> updateManager_;
    QString selectedAccountId_ {""};
    QString selectedConvUid_ {""};
    MapStringString contentDrafts_;
    MapStringString lastConferences_;

    conversation::Info invalid {};
};
Q_DECLARE_METATYPE(LRCInstance*)

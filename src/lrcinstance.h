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

#include "accountlistmodel.h"
#include "rendermanager.h"
#include "appsettingsmanager.h"
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

using namespace lrc::api;

using migrateCallback = std::function<void()>;
using getConvPredicate = std::function<bool(const conversation::Info& conv)>;

class LRCInstance : public QObject
{
    Q_OBJECT

public:
    static LRCInstance& instance(migrateCallback willMigrate = {}, migrateCallback didMigrate = {})
    {
        static LRCInstance instance_(willMigrate, didMigrate);
        return instance_;
    }

    static void init(migrateCallback willMigrate = {}, migrateCallback didMigrate = {})
    {
        instance(willMigrate, didMigrate);
    }

    static Lrc& getAPI()
    {
        return *(instance().lrc_);
    }

    static RenderManager* renderer()
    {
        return instance().renderer_.get();
    }

    static void connectivityChanged()
    {
        instance().lrc_->connectivityChanged();
    }

    static NewAccountModel& accountModel()
    {
        return instance().lrc_->getAccountModel();
    }

    static BehaviorController& behaviorController()
    {
        return instance().lrc_->getBehaviorController();
    }

    static DataTransferModel& dataTransferModel()
    {
        return instance().lrc_->getDataTransferModel();
    }

    static AVModel& avModel()
    {
        return instance().lrc_->getAVModel();
    }

    static PluginModel& pluginModel()
    {
        return instance().lrc_->getPluginModel();
    }

    static bool isConnected()
    {
        return instance().lrc_->isConnected();
    }

    static VectorString getActiveCalls()
    {
        return instance().lrc_->activeCalls();
    }

    static const account::Info& getAccountInfo(const QString& accountId)
    {
        return accountModel().getAccountInfo(accountId);
    }

    static const account::Info& getCurrentAccountInfo()
    {
        return getAccountInfo(getCurrAccId());
    }

    static bool hasVideoCall()
    {
        auto activeCalls = instance().lrc_->activeCalls();
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

    static QString getCallIdForConversationUid(const QString& convUid, const QString& accountId)
    {
        auto& accInfo = LRCInstance::getAccountInfo(accountId);
        auto convInfo = accInfo.conversationModel->getConversationForUID(convUid);
        if (convInfo.uid.isEmpty()) {
            return {};
        }
        return convInfo.confId.isEmpty() ? convInfo.callId : convInfo.confId;
    }

    static const call::Info* getCallInfo(const QString& callId, const QString& accountId)
    {
        try {
            auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);
            if (!accInfo.callModel->hasCall(callId)) {
                return nullptr;
            }
            return &accInfo.callModel->getCall(callId);
        } catch (...) {
            return nullptr;
        }
    }

    static const call::Info* getCallInfoForConversation(const conversation::Info& convInfo,
                                                        bool forceCallOnly = {})
    {
        try {
            auto accountId = convInfo.accountId;
            auto& accInfo = LRCInstance::accountModel().getAccountInfo(accountId);
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

    static const conversation::Info& getConversation(const QString& accountId,
                                                     getConvPredicate pred = {},
                                                     bool filtered = false)
    {
        using namespace lrc::api;
        static conversation::Info invalid = {};
        try {
            auto& accInfo = LRCInstance::getAccountInfo(accountId);
            auto& convModel = accInfo.conversationModel;
            if (filtered) {
                auto& convs = convModel->allFilteredConversations();
                auto conv = std::find_if(convs.begin(), convs.end(), pred);
                if (conv != convs.end()) {
                    return *conv;
                }
            } else {
                for (int i = static_cast<int>(profile::Type::RING);
                     i <= static_cast<int>(profile::Type::TEMPORARY);
                     ++i) {
                    auto filter = static_cast<profile::Type>(i);
                    auto& convs = convModel->getFilteredConversations(filter);
                    auto conv = std::find_if(convs.begin(), convs.end(), pred);
                    if (conv != convs.end()) {
                        return *conv;
                    }
                }
            }
        } catch (...) {
        }
        return invalid;
    }

    static const conversation::Info& getConversationFromCallId(const QString& callId,
                                                               const QString& accountId = {},
                                                               bool filtered = false)
    {
        return getConversation(
            !accountId.isEmpty() ? accountId : getCurrAccId(),
            [&](const conversation::Info& conv) -> bool {
                return callId == conv.callId or callId == conv.confId;
            },
            filtered);
    }

    static const conversation::Info& getConversationFromPeerUri(const QString& peerUri,
                                                                const QString& accountId = {},
                                                                bool filtered = false)
    {
        return getConversation(
            !accountId.isEmpty() ? accountId : getCurrAccId(),
            [&](const conversation::Info& conv) -> bool { return peerUri == conv.participants[0]; },
            filtered);
    }

    static ConversationModel* getCurrentConversationModel()
    {
        return getCurrentAccountInfo().conversationModel.get();
    }

    static NewCallModel* getCurrentCallModel()
    {
        return getCurrentAccountInfo().callModel.get();
    }

    static const QString& getCurrAccId()
    {
        if (instance().selectedAccountId_.isEmpty()) {
            auto accountList = accountModel().getAccountList();
            if (accountList.size())
                instance().selectedAccountId_ = accountList.at(0);
        }
        return instance().selectedAccountId_;
    }

    static void setSelectedAccountId(const QString& accountId = {})
    {
        if (accountId == instance().selectedAccountId_)
            return; // No need to select current selected account

        instance().selectedAccountId_ = accountId;

        // Last selected account should be set as preferred.
        accountModel().setTopAccount(accountId);

        emit instance().currentAccountChanged();
    }

    static const QString& getCurrentConvUid()
    {
        return instance().selectedConvUid_;
    }

    static void setSelectedConvId(const QString& convUid = {})
    {
        instance().selectedConvUid_ = convUid;
    }

    static void reset(bool newInstance = false)
    {
        if (newInstance) {
            instance().renderer_.reset(new RenderManager(avModel()));
            instance().lrc_.reset(new Lrc());
        } else {
            instance().renderer_.reset();
            instance().lrc_.reset();
        }
    }

    static int getCurrentAccountIndex()
    {
        for (int i = 0; i < accountModel().getAccountList().size(); i++) {
            if (accountModel().getAccountList()[i] == getCurrAccId()) {
                return i;
            }
        }
        return -1;
    }

    static const QPixmap getCurrAccPixmap()
    {
        return instance()
            .accountListModel_
            .data(instance().accountListModel_.index(getCurrentAccountIndex()),
                  AccountListModel::Role::Picture)
            .value<QPixmap>();
    }

    static void setAvatarForAccount(const QPixmap& avatarPixmap, const QString& accountID)
    {
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        avatarPixmap.save(&bu, "PNG");
        auto str = QString::fromLocal8Bit(ba.toBase64());
        accountModel().setAvatar(accountID, str);
    }

    static void setCurrAccAvatar(const QPixmap& avatarPixmap)
    {
        QByteArray ba;
        QBuffer bu(&ba);
        bu.open(QIODevice::WriteOnly);
        avatarPixmap.save(&bu, "PNG");
        auto str = QString::fromLocal8Bit(ba.toBase64());
        accountModel().setAvatar(getCurrAccId(), str);
    }

    static void setCurrAccAvatar(const QString& avatar)
    {
        accountModel().setAvatar(getCurrAccId(), avatar);
    }

    static void setCurrAccDisplayName(const QString& displayName)
    {
        auto accountId = LRCInstance::getCurrAccId();
        accountModel().setAlias(accountId, displayName);
        /*
         * Force save to .yml.
         */
        auto confProps = LRCInstance::accountModel().getAccountConfig(accountId);
        LRCInstance::accountModel().setAccountConfig(accountId, confProps);
    }

    static const account::ConfProperties_t& getCurrAccConfig()
    {
        return instance().getCurrentAccountInfo().confProperties;
    }

    static void subscribeToDebugReceived()
    {
        instance().lrc_->subscribeToDebugReceived();
    }

    static void startAudioMeter(bool async)
    {
        auto f = [] {
            if (!LRCInstance::getActiveCalls().size()) {
                LRCInstance::avModel().startAudioDevice();
            }
            LRCInstance::avModel().setAudioMeterState(true);
        };
        if (async) {
            QtConcurrent::run(f);
        } else {
            f();
        }
    }

    static void stopAudioMeter(bool async)
    {
        auto f = [] {
            if (!LRCInstance::getActiveCalls().size()) {
                LRCInstance::avModel().stopAudioDevice();
            }
            LRCInstance::avModel().setAudioMeterState(false);
        };
        if (async) {
            QtConcurrent::run(f);
        } else {
            f();
        }
    }

    static QString getContentDraft(const QString& convUid, const QString& accountId)
    {
        auto draftKey = accountId + "_" + convUid;
        return instance().contentDrafts_[draftKey];
    }

    static void setContentDraft(const QString& convUid,
                                const QString& accountId,
                                const QString& content)
    {
        auto draftKey = accountId + "_" + convUid;
        instance().contentDrafts_[draftKey] = content;
    }

    static void pushLastConferencee(const QString& confId, const QString& callId)
    {
        instance().lastConferencees_[confId] = callId;
    }

    static QString popLastConferencee(const QString& confId)
    {
        QString callId = {};
        auto iter = instance().lastConferencees_.find(confId);
        if (iter != instance().lastConferencees_.end()) {
            callId = iter.value();
            instance().lastConferencees_.erase(iter);
        }
        return callId;
    }

signals:
    void accountListChanged();
    void currentAccountChanged();
    void restoreAppRequested();

private:
    LRCInstance(migrateCallback willMigrateCb = {}, migrateCallback didMigrateCb = {})
    {
        lrc_ = std::make_unique<Lrc>(willMigrateCb, didMigrateCb);
        renderer_ = std::make_unique<RenderManager>(lrc_->getAVModel());
    };

    std::unique_ptr<Lrc> lrc_;
    std::unique_ptr<RenderManager> renderer_;
    AccountListModel accountListModel_;
    QString selectedAccountId_;
    QString selectedConvUid_;
    MapStringString contentDrafts_;
    MapStringString lastConferencees_;
};
Q_DECLARE_METATYPE(LRCInstance*)

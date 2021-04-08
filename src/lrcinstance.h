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

#pragma once

#ifdef _MSC_VER
#undef ERROR
#endif

#include "updatemanager.h"
#include "rendermanager.h"
#include "qtutils.h"
#include "utils.h"

#include "api/lrc.h"
#include "api/account.h"
#include "api/avmodel.h"
#include "api/behaviorcontroller.h"
#include "api/contact.h"
#include "api/contactmodel.h"
#include "api/conversation.h"
#include "api/conversationmodel.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"

#include <QObject>

#include <memory>

class ConnectivityMonitor;

using namespace lrc::api;

using migrateCallback = std::function<void()>;
using getConvPredicate = std::function<bool(const conversation::Info& conv)>;

class LRCInstance : public QObject
{
    Q_OBJECT
    QML_PROPERTY(QString, selectedConvUid)

public:
    explicit LRCInstance(migrateCallback willMigrateCb = {},
                         migrateCallback didMigrateCb = {},
                         const QString& updateUrl = {},
                         ConnectivityMonitor* connectivityMonitor = {});
    ~LRCInstance() = default;

    void finish();

    RenderManager* renderer();
    UpdateManager* getUpdateManager();

    NewAccountModel& accountModel();
    ConversationModel* getCurrentConversationModel();
    NewCallModel* getCurrentCallModel();
    AVModel& avModel();
    PluginModel& pluginModel();
    BehaviorController& behaviorController();
    DataTransferModel& dataTransferModel();

    void subscribeToDebugReceived();
    bool isConnected();
    void connectivityChanged();
    VectorString getActiveCalls();

    const account::Info& getAccountInfo(const QString& accountId);
    const account::Info& getCurrentAccountInfo();
    QString getCallIdForConversationUid(const QString& convUid, const QString& accountId);
    const call::Info* getCallInfo(const QString& callId, const QString& accountId);
    const call::Info* getCallInfoForConversation(const conversation::Info& convInfo,
                                                 bool forceCallOnly = {});
    const conversation::Info& getConversationFromConvUid(const QString& convUid,
                                                         const QString& accountId = {});
    const conversation::Info& getConversationFromPeerUri(const QString& peerUri,
                                                         const QString& accountId = {});
    const conversation::Info& getConversationFromCallId(const QString& callId,
                                                        const QString& accountId = {});

    const QString& getCurrAccId();
    void setSelectedAccountId(const QString& accountId = {});
    void selectConversation(const QString& accountId, const QString& convUid);
    int getCurrentAccountIndex();
    void setAvatarForAccount(const QPixmap& avatarPixmap, const QString& accountID);
    void setCurrAccAvatar(const QPixmap& avatarPixmap);
    void setCurrAccAvatar(const QString& avatar);
    void setCurrAccDisplayName(const QString& displayName);
    const account::ConfProperties_t& getCurrAccConfig();

    void startAudioMeter(bool async);
    void stopAudioMeter(bool async);

    QString getContentDraft(const QString& convUid, const QString& accountId);
    void setContentDraft(const QString& convUid, const QString& accountId, const QString& content);

    bool hasVideoCall();
    void pushlastConference(const QString& confId, const QString& callId);
    QString poplastConference(const QString& confId);
    VectorString getConferenceSubcalls(const QString& callId);

Q_SIGNALS:
    void accountListChanged();
    void currentAccountChanged();
    void restoreAppRequested();
    void notificationClicked();
    void updateSmartList();
    void quitEngineRequested();
    void conversationSelected();

private:
    std::unique_ptr<Lrc> lrc_;
    std::unique_ptr<RenderManager> renderer_;
    std::unique_ptr<UpdateManager> updateManager_;
    QString selectedAccountId_ {""};
    MapStringString contentDrafts_;
    MapStringString lastConferences_;

    conversation::Info invalid {};
};
Q_DECLARE_METATYPE(LRCInstance*)

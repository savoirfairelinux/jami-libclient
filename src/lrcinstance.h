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
#include <QThreadPool>

#include <memory>

class ConnectivityMonitor;

using namespace lrc::api;

using migrateCallback = std::function<void()>;
using getConvPredicate = std::function<bool(const conversation::Info& conv)>;

class LRCInstance : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString selectedConvUid READ get_selectedConvUid WRITE set_selectedConvUid NOTIFY
                   selectedConvUidChanged)
    QML_PROPERTY(QString, currentAccountId)
    QML_RO_PROPERTY(lrc::api::profile::Type, currentAccountType)
    QML_PROPERTY(bool, currentAccountAvatarSet)

public:
    explicit LRCInstance(migrateCallback willMigrateCb = {},
                         migrateCallback didMigrateCb = {},
                         const QString& updateUrl = {},
                         ConnectivityMonitor* connectivityMonitor = {},
                         bool muteDring = false);
    ~LRCInstance() = default;

    void finish();

    RenderManager* renderer();
    UpdateManager* getUpdateManager();

    NewAccountModel& accountModel();
    ConversationModel* getCurrentConversationModel();
    NewCallModel* getCurrentCallModel();
    ContactModel* getCurrentContactModel();
    AVModel& avModel();
    PluginModel& pluginModel();
    BehaviorController& behaviorController();

    void subscribeToDebugReceived();
    bool isConnected();
    void connectivityChanged();
    VectorString getActiveCalls();
    int notificationsCount() const;

    const account::Info& getAccountInfo(const QString& accountId);
    const account::Info& getCurrentAccountInfo();
    QString getCurrentCallId();
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

    Q_INVOKABLE void selectConversation(const QString& convId, const QString& accountId = {});
    Q_INVOKABLE void deselectConversation();
    Q_INVOKABLE void makeConversationPermanent(const QString& convId = {},
                                               const QString& accountId = {});
    Q_INVOKABLE QString getContentDraft(const QString& convUid, const QString& accountId);
    Q_INVOKABLE void setContentDraft(const QString& convUid,
                                     const QString& accountId,
                                     const QString& content);

    int getCurrentAccountIndex();
    void setCurrAccDisplayName(const QString& displayName);
    const account::ConfProperties_t& getCurrAccConfig();
    int indexOf(const QString& convId);

    void startAudioMeter();
    void stopAudioMeter();

    void monitor(bool continous);

    bool hasActiveCall(bool withVideo = false);
    VectorString getConferenceSubcalls(const QString& callId);

    QString get_selectedConvUid();

    void set_selectedConvUid(QString selectedConvUid = "");

Q_SIGNALS:
    void accountListChanged();
    void selectedConvUidChanged();
    void restoreAppRequested();
    void notificationClicked();
    void quitEngineRequested();
    void conversationUpdated(const QString& convId, const QString& accountId);
    void draftSaved(const QString& convId);

private:
    std::unique_ptr<Lrc> lrc_;
    std::unique_ptr<RenderManager> renderer_;
    std::unique_ptr<UpdateManager> updateManager_;

    QString selectedConvUid_;
    MapStringString contentDrafts_;
    MapStringString lastConferences_;

    conversation::Info invalid {};

    QThreadPool* threadPool_;
};
Q_DECLARE_METATYPE(LRCInstance*)

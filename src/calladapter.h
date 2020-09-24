/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

#pragma once

#include "lrcinstance.h"
#include "qmladapterbase.h"
#include "globalsystemtray.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSystemTrayIcon>

class CallAdapter final : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit CallAdapter(QObject* parent = nullptr);
    ~CallAdapter() = default;

protected:
    void safeInit() override {};

public:
    Q_INVOKABLE void placeAudioOnlyCall();
    Q_INVOKABLE void placeCall();
    Q_INVOKABLE void hangUpACall(const QString& accountId, const QString& convUid);
    Q_INVOKABLE void refuseACall(const QString& accountId, const QString& convUid);
    Q_INVOKABLE void acceptACall(const QString& accountId, const QString& convUid);

    Q_INVOKABLE void connectCallModel(const QString& accountId);
    Q_INVOKABLE void sipInputPanelPlayDTMF(const QString& key);

    /*
     * For Call Overlay
     */
    Q_INVOKABLE void hangupCall(const QString& uri);
    Q_INVOKABLE void maximizeParticipant(const QString& uri, bool isActive);
    Q_INVOKABLE void minimizeParticipant();
    Q_INVOKABLE void hangUpThisCall();
    Q_INVOKABLE bool isCurrentModerator() const;
    Q_INVOKABLE bool isCurrentHost() const;
    Q_INVOKABLE int getCurrentLayoutType() const;
    Q_INVOKABLE void holdThisCallToggle();
    Q_INVOKABLE void muteThisCallToggle();
    Q_INVOKABLE void recordThisCallToggle();
    Q_INVOKABLE void videoPauseThisCallToggle();
    Q_INVOKABLE bool isRecordingThisCall();
    Q_INVOKABLE QVariantList getConferencesInfos();

signals:
    void callStatusChanged(int index, const QString& accountId, const QString& convUid);
    void updateConversationSmartList();
    void updateParticipantsInfos(const QVariantList& infos,
                                 const QString& accountId,
                                 const QString& callId);
    void callSetupMainViewRequired(const QString& accountId, const QString& convUid);
    void previewVisibilityNeedToChange(bool visible);

    /*
     * For Call Overlay
     */
    void updateTimeText(const QString& time);
    void showOnHoldLabel(bool isPaused);
    void updateOverlay(bool isPaused,
                       bool isAudioOnly,
                       bool isAudioMuted,
                       bool isVideoMuted,
                       bool isRecording,
                       bool isSIP,
                       bool isConferenceCall,
                       const QString& bestName);

public slots:
    void slotShowIncomingCallView(const QString& accountId,
                                  const lrc::api::conversation::Info& convInfo);
    void slotShowCallView(const QString& accountId, const lrc::api::conversation::Info& convInfo);
    void slotAccountChanged();

private:
    void updateCall(const QString& convUid = {},
                    const QString& accountId = {},
                    bool forceCallOnly = false);
    bool shouldShowPreview(bool force);
    void showNotification(const QString& accountId, const QString& convUid);

    /*
     * Current conf/call info.
     */
    QString accountId_;
    QString convUid_;

    QMetaObject::Connection callStatusChangedConnection_;
    QMetaObject::Connection onParticipantsChangedConnection_;
    QMetaObject::Connection closeIncomingCallPageConnection_;
    QMetaObject::Connection appStateChangedConnection_;

    /*
     * For Call Overlay
     */
    void updateCallOverlay(const lrc::api::conversation::Info& convInfo);
    void setTime(const QString& accountId, const QString& convUid);
    QTimer* oneSecondTimer_;
};

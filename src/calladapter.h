/*
 * Copyright (C) 2020-2021 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
#include "screensaver.h"
#include "calloverlaymodel.h"

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSystemTrayIcon>

class SystemTray;

class CallAdapter final : public QmlAdapterBase
{
    Q_OBJECT
    QML_PROPERTY(bool, hasCall)

public:
    enum MuteStates { UNMUTED, LOCAL_MUTED, MODERATOR_MUTED, BOTH_MUTED };
    Q_ENUM(MuteStates)

    explicit CallAdapter(SystemTray* systemTray, LRCInstance* instance, QObject* parent = nullptr);
    ~CallAdapter() = default;

protected:
    void safeInit() override {};

public:
    Q_INVOKABLE void placeAudioOnlyCall();
    Q_INVOKABLE void placeCall();
    Q_INVOKABLE void hangUpACall(const QString& accountId, const QString& convUid);
    Q_INVOKABLE void setCallMedia(const QString& accountId, const QString& convUid, bool video);
    Q_INVOKABLE void acceptACall(const QString& accountId, const QString& convUid);

    Q_INVOKABLE void connectCallModel(const QString& accountId);
    Q_INVOKABLE void sipInputPanelPlayDTMF(const QString& key);

    // For Call Overlay
    Q_INVOKABLE void hangUpCall(const QString& callId);
    Q_INVOKABLE void maximizeParticipant(const QString& uri);
    Q_INVOKABLE void minimizeParticipant(const QString& uri);
    Q_INVOKABLE void showGridConferenceLayout();
    Q_INVOKABLE void hangUpThisCall();
    Q_INVOKABLE bool isCurrentHost() const;
    Q_INVOKABLE bool participantIsHost(const QString& uri) const;
    Q_INVOKABLE void setModerator(const QString& uri, const bool state);
    Q_INVOKABLE bool isModerator(const QString& uri = {}) const;
    Q_INVOKABLE bool isCurrentModerator() const;
    Q_INVOKABLE void holdThisCallToggle();
    Q_INVOKABLE void muteThisCallToggle(bool mute);
    Q_INVOKABLE void recordThisCallToggle();
    Q_INVOKABLE void videoPauseThisCallToggle(bool mute);
    Q_INVOKABLE bool isRecordingThisCall();
    Q_INVOKABLE QVariantList getConferencesInfos();
    Q_INVOKABLE void muteParticipant(const QString& uri, const bool state);
    Q_INVOKABLE MuteStates getMuteState(const QString& uri) const;
    Q_INVOKABLE void hangupParticipant(const QString& uri);
    Q_INVOKABLE void updateCall(const QString& convUid = {},
                                const QString& accountId = {},
                                bool forceCallOnly = false);
    Q_INVOKABLE QString getCallDurationTime(const QString& accountId, const QString& convUid);

Q_SIGNALS:
    void callStatusChanged(int index, const QString& accountId, const QString& convUid);
    void callInfosChanged(const QVariant& infos, const QString& accountId, const QString& convUid);
    void updateParticipantsInfos(const QVariantList& infos,
                                 const QString& accountId,
                                 const QString& callId);
    void previewVisibilityNeedToChange(bool visible);

    // For Call Overlay
    void updateTimeText(const QString& time);
    void showOnHoldLabel(bool isPaused);
    void updateOverlay(bool isPaused,
                       bool isAudioOnly,
                       bool isAudioMuted,
                       bool isVideoMuted,
                       bool isRecording,
                       bool isSIP,
                       bool isConferenceCall,
                       bool isGrid);
    void remoteRecordingChanged(const QStringList& peers, bool state);
    void eraseRemoteRecording();

public Q_SLOTS:
    void onShowIncomingCallView(const QString& accountId, const QString& convUid);
    void onShowCallView(const QString& accountId, const QString& convUid);
    void onAccountChanged();
    void onCallStatusChanged(const QString& accountId, const QString& callId);
    void onCallInfosChanged(const QString& accountId, const QString& callId);
    void onParticipantsChanged(const QString& confId);
    void onCallStatusChanged(const QString& callId, int code);
    void onRemoteRecordingChanged(const QString& callId, const QSet<QString>& peerRec, bool state);
    void onCallAddedToConference(const QString& callId, const QString& confId);

private:
    void updateRecordingPeers(bool eraseLabelOnEmpty = false);
    bool shouldShowPreview(bool force);
    void showNotification(const QString& accountId, const QString& convUid);
    QJsonObject fillParticipantData(QMap<QString, QString> participant);
    void preventScreenSaver(bool state);
    void updateCallOverlay(const lrc::api::conversation::Info& convInfo);
    void saveConferenceSubcalls();

    QString accountId_;
    QString convUid_;

    ScreenSaver screenSaver;
    SystemTray* systemTray_;
    QScopedPointer<CallOverlayModel> overlayModel_;
    VectorString currentConfSubcalls_;
};

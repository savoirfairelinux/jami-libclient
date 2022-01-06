/******************************************************************************
 *    Copyright (C) 2014-2022 Savoir-faire Linux Inc.                         *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>        *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

#include <callmanager_interface.h>
#include "typedefs.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface cx.ring.Ring.CallManager
 */
class CallManagerInterface : public QObject
{
    Q_OBJECT

public:
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> callHandlers;

    CallManagerInterface()
    {
        using DRing::exportable_callback;
        using DRing::CallSignal;

        callHandlers = {
            exportable_callback<CallSignal::StateChange>([this](const std::string& accountId,
                                                                const std::string& callId,
                                                                const std::string& state,
                                                                int code) {
                LOG_DRING_SIGNAL3("callStateChanged",
                                  QString(callId.c_str()),
                                  QString(state.c_str()),
                                  code);
                Q_EMIT callStateChanged(QString(accountId.c_str()),
                                        QString(callId.c_str()),
                                        QString(state.c_str()),
                                        code);
            }),
            exportable_callback<CallSignal::MediaNegotiationStatus>(
                [this](const std::string& callId,
                       const std::string& event,
                       const std::vector<std::map<std::string, std::string>>& mediaList) {
                    LOG_DRING_SIGNAL3("mediaNegotiationStatus",
                                      QString(callId.c_str()),
                                      QString(event.c_str()),
                                      convertVecMap(mediaList));
                    Q_EMIT mediaNegotiationStatus(QString(callId.c_str()),
                                                  QString(event.c_str()),
                                                  convertVecMap(mediaList));
                }),
            exportable_callback<CallSignal::TransferFailed>([this]() {
                LOG_DRING_SIGNAL("transferFailed", "");
                Q_EMIT transferFailed();
            }),
            exportable_callback<CallSignal::TransferSucceeded>([this]() {
                LOG_DRING_SIGNAL("transferSucceeded", "");
                Q_EMIT transferSucceeded();
            }),
            exportable_callback<CallSignal::RecordPlaybackStopped>(
                [this](const std::string& filepath) {
                    LOG_DRING_SIGNAL("recordPlaybackStopped", QString(filepath.c_str()));
                    Q_EMIT recordPlaybackStopped(QString(filepath.c_str()));
                }),
            exportable_callback<CallSignal::VoiceMailNotify>(
                [this](const std::string& accountId, int newCount, int oldCount, int urgentCount) {
                    LOG_DRING_SIGNAL4("voiceMailNotify",
                                      QString(accountId.c_str()),
                                      newCount,
                                      oldCount,
                                      urgentCount);
                    Q_EMIT voiceMailNotify(QString(accountId.c_str()),
                                           newCount,
                                           oldCount,
                                           urgentCount);
                }),
            exportable_callback<CallSignal::IncomingMessage>(
                [this](const std::string& accountId,
                       const std::string& callId,
                       const std::string& from,
                       const std::map<std::string, std::string>& message) {
                    LOG_DRING_SIGNAL4("incomingMessage",
                                      QString(accountId.c_str()),
                                      QString(callId.c_str()),
                                      QString(from.c_str()),
                                      convertMap(message));
                    Q_EMIT incomingMessage(QString(accountId.c_str()),
                                           QString(callId.c_str()),
                                           QString(from.c_str()),
                                           convertMap(message));
                }),
            exportable_callback<CallSignal::IncomingCall>([this](const std::string& accountId,
                                                                 const std::string& callId,
                                                                 const std::string& from) {
                LOG_DRING_SIGNAL3("incomingCall",
                                  QString(accountId.c_str()),
                                  QString(callId.c_str()),
                                  QString(from.c_str()));
                Q_EMIT incomingCall(QString(accountId.c_str()),
                                    QString(callId.c_str()),
                                    QString(from.c_str()));
            }),
            exportable_callback<CallSignal::IncomingCallWithMedia>(
                [this](const std::string& accountId,
                       const std::string& callId,
                       const std::string& from,
                       const std::vector<std::map<std::string, std::string>>& mediaList) {
                    LOG_DRING_SIGNAL4("incomingCallWithMedia",
                                      QString(accountId.c_str()),
                                      QString(callId.c_str()),
                                      QString(from.c_str()),
                                      convertVecMap(mediaList));
                    Q_EMIT incomingCallWithMedia(QString(accountId.c_str()),
                                                 QString(callId.c_str()),
                                                 QString(from.c_str()),
                                                 convertVecMap(mediaList));
                }),
            exportable_callback<CallSignal::MediaChangeRequested>(
                [this](const std::string& accountId,
                       const std::string& callId,
                       const std::vector<std::map<std::string, std::string>>& mediaList) {
                    LOG_DRING_SIGNAL3("mediaChangeRequested",
                                      QString(accountId.c_str()),
                                      QString(callId.c_str()),
                                      convertVecMap(mediaList));
                    Q_EMIT mediaChangeRequested(QString(accountId.c_str()),
                                                QString(callId.c_str()),
                                                convertVecMap(mediaList));
                }),
            exportable_callback<CallSignal::RecordPlaybackFilepath>(
                [this](const std::string& callId, const std::string& filepath) {
                    LOG_DRING_SIGNAL2("recordPlaybackFilepath",
                                      QString(callId.c_str()),
                                      QString(filepath.c_str()));
                    Q_EMIT recordPlaybackFilepath(QString(callId.c_str()),
                                                  QString(filepath.c_str()));
                }),
            exportable_callback<CallSignal::ConferenceCreated>(
                [this](const std::string& accountId, const std::string& confId) {
                    LOG_DRING_SIGNAL2("conferenceCreated",
                                      QString(accountId.c_str()),
                                      QString(confId.c_str()));
                    Q_EMIT conferenceCreated(QString(accountId.c_str()), QString(confId.c_str()));
                }),
            exportable_callback<CallSignal::ConferenceChanged>([this](const std::string& accountId,
                                                                      const std::string& confId,
                                                                      const std::string& state) {
                LOG_DRING_SIGNAL3("conferenceChanged",
                                  QString(accountId.c_str()),
                                  QString(confId.c_str()),
                                  QString(state.c_str()));
                Q_EMIT conferenceChanged(QString(accountId.c_str()),
                                         QString(confId.c_str()),
                                         QString(state.c_str()));
            }),
            exportable_callback<CallSignal::UpdatePlaybackScale>([this](const std::string& filepath,
                                                                        int position,
                                                                        int size) {
                LOG_DRING_SIGNAL3("updatePlaybackScale", QString(filepath.c_str()), position, size);
                Q_EMIT updatePlaybackScale(QString(filepath.c_str()), position, size);
            }),
            exportable_callback<CallSignal::ConferenceRemoved>(
                [this](const std::string& accountId, const std::string& confId) {
                    LOG_DRING_SIGNAL2("conferenceRemoved",
                                      QString(accountId.c_str()),
                                      QString(confId.c_str()));
                    Q_EMIT conferenceRemoved(QString(accountId.c_str()), QString(confId.c_str()));
                }),
            exportable_callback<CallSignal::RecordingStateChanged>([this](const std::string& callId,
                                                                          bool recordingState) {
                LOG_DRING_SIGNAL2("recordingStateChanged", QString(callId.c_str()), recordingState);
                Q_EMIT recordingStateChanged(QString(callId.c_str()), recordingState);
            }),
            exportable_callback<CallSignal::RtcpReportReceived>(
                [this](const std::string& callId, const std::map<std::string, int>& report) {
                    LOG_DRING_SIGNAL2("onRtcpReportReceived",
                                      QString(callId.c_str()),
                                      convertStringInt(report));
                    Q_EMIT onRtcpReportReceived(QString(callId.c_str()), convertStringInt(report));
                }),
            exportable_callback<CallSignal::OnConferenceInfosUpdated>(
                [this](const std::string& confId,
                       const std::vector<std::map<std::string, std::string>>& infos) {
                    LOG_DRING_SIGNAL2("onConferenceInfosUpdated",
                                      QString(confId.c_str()),
                                      convertVecMap(infos));
                    Q_EMIT onConferenceInfosUpdated(QString(confId.c_str()), convertVecMap(infos));
                }),
            exportable_callback<CallSignal::PeerHold>([this](const std::string& callId, bool state) {
                LOG_DRING_SIGNAL2("peerHold", QString(callId.c_str()), state);
                Q_EMIT peerHold(QString(callId.c_str()), state);
            }),
            exportable_callback<CallSignal::AudioMuted>(
                [this](const std::string& callId, bool state) {
                    LOG_DRING_SIGNAL2("audioMuted", QString(callId.c_str()), state);
                    Q_EMIT audioMuted(QString(callId.c_str()), state);
                }),
            exportable_callback<CallSignal::VideoMuted>(
                [this](const std::string& callId, bool state) {
                    LOG_DRING_SIGNAL2("videoMuted", QString(callId.c_str()), state);
                    Q_EMIT videoMuted(QString(callId.c_str()), state);
                }),
            exportable_callback<CallSignal::SmartInfo>(
                [this](const std::map<std::string, std::string>& info) {
                    LOG_DRING_SIGNAL("SmartInfo", "");
                    Q_EMIT SmartInfo(convertMap(info));
                }),
            exportable_callback<CallSignal::RemoteRecordingChanged>(
                [this](const std::string& callId, const std::string& contactId, bool state) {
                    LOG_DRING_SIGNAL3("remoteRecordingChanged",
                                      QString(callId.c_str()),
                                      QString(contactId.c_str()),
                                      state);
                    Q_EMIT remoteRecordingChanged(QString(callId.c_str()),
                                                  QString(contactId.c_str()),
                                                  state);
                })};
    }

    ~CallManagerInterface() {}

    bool isValid() { return true; }

public Q_SLOTS: // METHODS
    bool accept(const QString& accountId, const QString& callId)
    {
        return DRing::accept(accountId.toStdString(), callId.toStdString());
    }

    bool addMainParticipant(const QString& accountId, const QString& confId)
    {
        return DRing::addMainParticipant(accountId.toStdString(), confId.toStdString());
    }

    bool addParticipant(const QString& accountId,
                        const QString& callId,
                        const QString& account2Id,
                        const QString& confId)
    {
        return DRing::addParticipant(accountId.toStdString(),
                                     callId.toStdString(),
                                     account2Id.toStdString(),
                                     confId.toStdString());
    }

    bool attendedTransfer(const QString& accountId,
                          const QString& transferId,
                          const QString& targetId)
    {
        return DRing::attendedTransfer(accountId.toStdString(),
                                       transferId.toStdString(),
                                       targetId.toStdString());
    }

    void createConfFromParticipantList(const QString& accountId, const QStringList& participants)
    {
        DRing::createConfFromParticipantList(accountId.toStdString(),
                                             convertStringList(participants));
    }

    bool detachParticipant(const QString& accountId, const QString& callId)
    {
        return DRing::detachParticipant(accountId.toStdString(), callId.toStdString());
    }

    MapStringString getCallDetails(const QString& accountId, const QString& callId)
    {
        MapStringString temp = convertMap(
            DRing::getCallDetails(accountId.toStdString(), callId.toStdString()));
        return temp;
    }

    QStringList getCallList(const QString& accountId)
    {
        QStringList temp = convertStringList(DRing::getCallList(accountId.toStdString()));
        return temp;
    }

    MapStringString getConferenceDetails(const QString& accountId, const QString& callId)
    {
        MapStringString temp = convertMap(
            DRing::getConferenceDetails(accountId.toStdString(), callId.toStdString()));
        return temp;
    }

    VectorMapStringString getConferenceInfos(const QString& accountId, const QString& confId)
    {
        VectorMapStringString temp = convertVecMap(
            DRing::getConferenceInfos(accountId.toStdString(), confId.toStdString()));
        return temp;
    }

    QString getConferenceId(const QString& accountId, const QString& callId)
    {
        QString temp(DRing::getConferenceId(accountId.toStdString(), callId.toStdString()).c_str());
        return temp;
    }

    QStringList getConferenceList(const QString& accountId)
    {
        QStringList temp = convertStringList(DRing::getConferenceList(accountId.toStdString()));
        return temp;
    }

    bool getIsRecording(const QString& accountId, const QString& callId)
    {
        return DRing::getIsRecording(accountId.toStdString(), callId.toStdString());
    }

    QStringList getParticipantList(const QString& accountId, const QString& confId)
    {
        QStringList temp = convertStringList(
            DRing::getParticipantList(accountId.toStdString(), confId.toStdString()));
        return temp;
    }

    bool hangUp(const QString& accountId, const QString& callId)
    {
        return DRing::hangUp(accountId.toStdString(), callId.toStdString());
    }

    bool hangUpConference(const QString& accountId, const QString& confId)
    {
        return DRing::hangUpConference(accountId.toStdString(), confId.toStdString());
    }

    bool hold(const QString& accountId, const QString& callId)
    {
        return DRing::hold(accountId.toStdString(), callId.toStdString());
    }

    bool holdConference(const QString& accountId, const QString& confId)
    {
        return DRing::holdConference(accountId.toStdString(), confId.toStdString());
    }

    bool isConferenceParticipant(const QString& accountId, const QString& callId)
    {
        return DRing::isConferenceParticipant(accountId.toStdString(), callId.toStdString());
    }

    bool joinConference(const QString& accountId,
                        const QString& sel_confId,
                        const QString& account2Id,
                        const QString& drag_confId)
    {
        return DRing::joinConference(accountId.toStdString(),
                                     sel_confId.toStdString(),
                                     account2Id.toStdString(),
                                     drag_confId.toStdString());
    }

    bool joinParticipant(const QString& accountId,
                         const QString& sel_callId,
                         const QString& account2Id,
                         const QString& drag_callId)
    {
        return DRing::joinParticipant(accountId.toStdString(),
                                      sel_callId.toStdString(),
                                      account2Id.toStdString(),
                                      drag_callId.toStdString());
    }

    QString placeCall(const QString& accountId, const QString& to)
    {
        QString temp(DRing::placeCall(accountId.toStdString(), to.toStdString()).c_str());
        return temp;
    }

    // MULTISTREAM FUNCTIONS
    QString placeCallWithMedia(const QString& accountId,
                               const QString& to,
                               const VectorMapStringString& mediaList)
    {
        QString temp(DRing::placeCallWithMedia(accountId.toStdString(),
                                               to.toStdString(),
                                               convertVecMap(mediaList))
                         .c_str());
        return temp;
    }

    bool requestMediaChange(const QString& accountId,
                            const QString& callId,
                            const VectorMapStringString& mediaList)
    {
        return DRing::requestMediaChange(accountId.toStdString(),
                                         callId.toStdString(),
                                         convertVecMap(mediaList));
    }

    bool acceptWithMedia(const QString& accountId,
                         const QString& callId,
                         const VectorMapStringString& mediaList)
    {
        return DRing::acceptWithMedia(accountId.toStdString(),
                                      callId.toStdString(),
                                      convertVecMap(mediaList));
    }

    bool answerMediaChangeRequest(const QString& accountId,
                                  const QString& callId,
                                  const VectorMapStringString& mediaList)
    {
        return DRing::answerMediaChangeRequest(accountId.toStdString(),
                                               callId.toStdString(),
                                               convertVecMap(mediaList));
    }
    // END OF MULTISTREAM FUNCTIONS

    void playDTMF(const QString& key) { DRing::playDTMF(key.toStdString()); }

    void recordPlaybackSeek(double value) { DRing::recordPlaybackSeek(value); }

    bool refuse(const QString& accountId, const QString& callId)
    {
        return DRing::refuse(accountId.toStdString(), callId.toStdString());
    }

    void sendTextMessage(const QString& accountId,
                         const QString& callId,
                         const QMap<QString, QString>& message,
                         bool isMixed)
    {
        DRing::sendTextMessage(accountId.toStdString(),
                               callId.toStdString(),
                               convertMap(message),
                               QObject::tr("Me").toStdString(),
                               isMixed);
    }

    bool startRecordedFilePlayback(const QString& filepath)
    {
        // TODO: Change method name to match API
        return DRing::startRecordedFilePlayback(filepath.toStdString());
    }

    void startTone(int start, int type) { DRing::startTone(start, type); }

    void stopRecordedFilePlayback() { DRing::stopRecordedFilePlayback(); }

    bool toggleRecording(const QString& accountId, const QString& callId)
    {
        return DRing::toggleRecording(accountId.toStdString(), callId.toStdString());
    }

    bool transfer(const QString& accountId, const QString& callId, const QString& to)
    {
        return DRing::transfer(accountId.toStdString(), callId.toStdString(), to.toStdString());
    }

    bool unhold(const QString& accountId, const QString& callId)
    {
        return DRing::unhold(accountId.toStdString(), callId.toStdString());
    }

    bool unholdConference(const QString& accountId, const QString& confId)
    {
        return DRing::unholdConference(accountId.toStdString(), confId.toStdString());
    }

    bool muteLocalMedia(const QString& accountId,
                        const QString& callId,
                        const QString& mediaType,
                        bool mute)
    {
        return DRing::muteLocalMedia(accountId.toStdString(),
                                     callId.toStdString(),
                                     mediaType.toStdString(),
                                     mute);
    }

    void startSmartInfo(int refresh) { DRing::startSmartInfo(refresh); }

    void stopSmartInfo() { DRing::stopSmartInfo(); }

    void setConferenceLayout(const QString& accountId, const QString& confId, int layout)
    {
        DRing::setConferenceLayout(accountId.toStdString(), confId.toStdString(), layout);
    }

    void setActiveParticipant(const QString& accountId, const QString& confId, const QString& callId)
    {
        DRing::setActiveParticipant(accountId.toStdString(),
                                    confId.toStdString(),
                                    callId.toStdString());
    }

    bool switchInput(const QString& accountId, const QString& callId, const QString& resource)
    {
#ifdef ENABLE_VIDEO
        return DRing::switchInput(accountId.toStdString(),
                                  callId.toStdString(),
                                  resource.toStdString());
#else
        Q_UNUSED(accountId)
        Q_UNUSED(callId)
        Q_UNUSED(resource)
        return false;
#endif
    }

    void setModerator(const QString& accountId,
                      const QString& confId,
                      const QString& peerId,
                      const bool& state)
    {
        DRing::setModerator(accountId.toStdString(),
                            confId.toStdString(),
                            peerId.toStdString(),
                            state);
    }

    void muteParticipant(const QString& accountId,
                         const QString& confId,
                         const QString& peerId,
                         const bool& state)
    {
        DRing::muteParticipant(accountId.toStdString(),
                               confId.toStdString(),
                               peerId.toStdString(),
                               state);
    }

    void hangupParticipant(const QString& accountId,
                           const QString& confId,
                           const QString& participant)
    {
        DRing::hangupParticipant(accountId.toStdString(),
                                 confId.toStdString(),
                                 participant.toStdString());
    }

    void raiseParticipantHand(const QString& accountId,
                              const QString& confId,
                              const QString& peerId,
                              const bool& state)
    {
        DRing::raiseParticipantHand(accountId.toStdString(),
                                    confId.toStdString(),
                                    peerId.toStdString(),
                                    state);
    }

Q_SIGNALS: // SIGNALS
    void callStateChanged(const QString& accountId,
                          const QString& callId,
                          const QString& state,
                          int code);
    void mediaNegotiationStatus(const QString& callId,
                                const QString& event,
                                const VectorMapStringString& mediaList);
    void transferFailed();
    void transferSucceeded();
    void recordPlaybackStopped(const QString& filepath);
    void voiceMailNotify(const QString& accountId, int newCount, int oldCount, int urgentCount);
    void incomingMessage(const QString& accountId,
                         const QString& callId,
                         const QString& from,
                         const MapStringString& message);
    void incomingCall(const QString& accountId, const QString& callId, const QString& from);
    void incomingCallWithMedia(const QString& accountId,
                               const QString& callId,
                               const QString& from,
                               const VectorMapStringString& mediaList);
    void mediaChangeRequested(const QString& accountId,
                              const QString& callId,
                              const VectorMapStringString& mediaList);
    void recordPlaybackFilepath(const QString& callId, const QString& filepath);
    void conferenceCreated(const QString& accountId, const QString& confId);
    void conferenceChanged(const QString& accountId, const QString& confId, const QString& state);
    void updatePlaybackScale(const QString& filepath, int position, int size);
    void conferenceRemoved(const QString& accountId, const QString& confId);
    void recordingStateChanged(const QString& callId, bool recordingState);
    void onRtcpReportReceived(const QString& callId, MapStringInt report);
    void onConferenceInfosUpdated(const QString& confId, VectorMapStringString infos);
    void audioMuted(const QString& callId, bool state);
    void videoMuted(const QString& callId, bool state);
    void peerHold(const QString& callId, bool state);
    void SmartInfo(const MapStringString& info);
    void remoteRecordingChanged(const QString& callId,
                                const QString& peerNumber,
                                bool remoteRecordingState);
};

namespace org {
namespace ring {
namespace Ring {
typedef ::CallManagerInterface CallManager;
}
} // namespace ring
} // namespace org

/******************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                                 *
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
class CallManagerInterface: public QObject
{
    Q_OBJECT

public:

    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> callHandlers;

    CallManagerInterface()
    {
        using DRing::exportable_callback;
        using DRing::CallSignal;

         callHandlers = {
            exportable_callback<CallSignal::StateChange>(
                [this] (const std::string &callID, const std::string &state, int code) {
                    LOG_DRING_SIGNAL3("callStateChanged",QString(callID.c_str()) , QString(state.c_str()) , code);
                    Q_EMIT callStateChanged(QString(callID.c_str()), QString(state.c_str()), code);
                }),
            exportable_callback<CallSignal::TransferFailed>(
                [this] () {
                    LOG_DRING_SIGNAL("transferFailed","");
                    Q_EMIT transferFailed();
                }),
            exportable_callback<CallSignal::TransferSucceeded>(
                [this] () {
                    LOG_DRING_SIGNAL("transferSucceeded","");
                    Q_EMIT transferSucceeded();
                }),
            exportable_callback<CallSignal::RecordPlaybackStopped>(
                [this] (const std::string &filepath) {
                    LOG_DRING_SIGNAL("recordPlaybackStopped",QString(filepath.c_str()));
                    Q_EMIT recordPlaybackStopped(QString(filepath.c_str()));
                }),
            exportable_callback<CallSignal::VoiceMailNotify>(
                [this] (const std::string &accountID, int count) {
                    LOG_DRING_SIGNAL2("voiceMailNotify",QString(accountID.c_str()), count);
                    Q_EMIT voiceMailNotify(QString(accountID.c_str()), count);
                }),
            exportable_callback<CallSignal::IncomingMessage>(
                [this] (const std::string &callID, const std::string &from, const std::map<std::string,std::string> &message) {
                    LOG_DRING_SIGNAL3("incomingMessage",QString(callID.c_str()),QString(from.c_str()),convertMap(message));
                    Q_EMIT incomingMessage(QString(callID.c_str()), QString(from.c_str()), convertMap(message));
                }),
            exportable_callback<CallSignal::IncomingCall>(
                [this] (const std::string &accountID, const std::string &callID, const std::string &from) {
                    LOG_DRING_SIGNAL3("incomingCall",QString(accountID.c_str()), QString(callID.c_str()), QString(from.c_str()));
                    Q_EMIT incomingCall(QString(accountID.c_str()), QString(callID.c_str()), QString(from.c_str()));
                }),
            exportable_callback<CallSignal::RecordPlaybackFilepath>(
                [this] (const std::string &callID, const std::string &filepath) {
                    LOG_DRING_SIGNAL2("recordPlaybackFilepath",QString(callID.c_str()), QString(filepath.c_str()));
                    Q_EMIT recordPlaybackFilepath(QString(callID.c_str()), QString(filepath.c_str()));
                }),
            exportable_callback<CallSignal::ConferenceCreated>(
                [this] (const std::string &confID) {
                    LOG_DRING_SIGNAL("conferenceCreated",QString(confID.c_str()));
                    Q_EMIT conferenceCreated(QString(confID.c_str()));
                }),
            exportable_callback<CallSignal::ConferenceChanged>(
                [this] (const std::string &confID, const std::string &state) {
                    LOG_DRING_SIGNAL2("conferenceChanged",QString(confID.c_str()), QString(state.c_str()));
                    Q_EMIT conferenceChanged(QString(confID.c_str()), QString(state.c_str()));
                }),
            exportable_callback<CallSignal::UpdatePlaybackScale>(
                [this] (const std::string &filepath, int position, int size) {
                    LOG_DRING_SIGNAL3("updatePlaybackScale",QString(filepath.c_str()), position, size);
                    Q_EMIT updatePlaybackScale(QString(filepath.c_str()), position, size);
                }),
            exportable_callback<CallSignal::ConferenceRemoved>(
                [this] (const std::string &confID) {
                    LOG_DRING_SIGNAL("conferenceRemoved",QString(confID.c_str()));
                    Q_EMIT conferenceRemoved(QString(confID.c_str()));
                }),
            exportable_callback<CallSignal::NewCallCreated>(
                [this] (const std::string &accountID, const std::string &callID, const std::string &to) {
                    LOG_DRING_SIGNAL3("newCallCreated",QString(accountID.c_str()), QString(callID.c_str()), QString(to.c_str()));
                    Q_EMIT newCallCreated(QString(accountID.c_str()), QString(callID.c_str()), QString(to.c_str()));
                }),
            exportable_callback<CallSignal::RecordingStateChanged>(
                [this] (const std::string &callID, bool recordingState) {
                    LOG_DRING_SIGNAL2("recordingStateChanged",QString(callID.c_str()), recordingState);
                    Q_EMIT recordingStateChanged(QString(callID.c_str()), recordingState);
                }),
            exportable_callback<CallSignal::RtcpReportReceived>(
                [this] (const std::string &callID, const std::map<std::string, int>& report) {
                    LOG_DRING_SIGNAL2("onRtcpReportReceived",QString(callID.c_str()), convertStringInt(report));
                    Q_EMIT onRtcpReportReceived(QString(callID.c_str()), convertStringInt(report));
                }),
            exportable_callback<CallSignal::PeerHold>(
                [this] (const std::string &callID, bool state) {
                    LOG_DRING_SIGNAL2("peerHold",QString(callID.c_str()), state);
                    Q_EMIT peerHold(QString(callID.c_str()), state);
            }),
            exportable_callback<CallSignal::AudioMuted>(
                [this] (const std::string &callID, bool state) {
                    LOG_DRING_SIGNAL2("audioMuted",QString(callID.c_str()), state);
                    Q_EMIT audioMuted(QString(callID.c_str()), state);
                }),
            exportable_callback<CallSignal::VideoMuted>(
                [this] (const std::string &callID, bool state) {
                    LOG_DRING_SIGNAL2("videoMuted",QString(callID.c_str()), state);
                    Q_EMIT videoMuted(QString(callID.c_str()), state);
                }),
            exportable_callback<CallSignal::SmartInfo>(
                [this] (const std::map<std::string, std::string>& info) {
                    LOG_DRING_SIGNAL("smartInfo","");
                    Q_EMIT smartInfo(convertMap(info));
                })
         };
     }

    ~CallManagerInterface() {}

    bool isValid() { return true; }

public Q_SLOTS: // METHODS
    bool accept(const QString &callID)
    {
        return DRing::accept(callID.toStdString());
    }

    bool addMainParticipant(const QString &confID)
    {
        return DRing::addMainParticipant(confID.toStdString());
    }

    bool addParticipant(const QString &callID, const QString &confID)
    {
        return DRing::addParticipant(
                        callID.toStdString(), confID.toStdString());
    }

    bool attendedTransfer(const QString &transferID, const QString &targetID)
    {
        return DRing::attendedTransfer(
                        transferID.toStdString(), targetID.toStdString());
    }

    void createConfFromParticipantList(const QStringList &participants)
    {
        DRing::createConfFromParticipantList(
                        convertStringList(participants));
    }

    bool detachParticipant(const QString &callID)
    {
        return DRing::detachParticipant(callID.toStdString());
    }

    MapStringString getCallDetails(const QString &callID)
    {
        MapStringString temp =
            convertMap(DRing::getCallDetails(callID.toStdString()));
        return temp;
    }

    QStringList getCallList()
    {
        QStringList temp =
            convertStringList(DRing::getCallList());
        return temp;
    }

    MapStringString getConferenceDetails(const QString &callID)
    {
        MapStringString temp =
            convertMap(DRing::getConferenceDetails(
                callID.toStdString()));
        return temp;
    }

    QString getConferenceId(const QString &callID)
    {
        QString temp(DRing::getConferenceId(callID.toStdString()).c_str());
        return temp;
    }

    QStringList getConferenceList()
    {
        QStringList temp =
            convertStringList(DRing::getConferenceList());
        return temp;
    }

    QStringList getDisplayNames(const QString &confID)
    {
        QStringList temp =
            convertStringList(DRing::getDisplayNames(
                confID.toStdString()));
        return temp;
    }

    bool getIsRecording(const QString &callID)
    {
        //TODO: match API
        return DRing::getIsRecording(callID.toStdString());
    }

    QStringList getParticipantList(const QString &confID)
    {
        QStringList temp =
            convertStringList(DRing::getParticipantList(
                confID.toStdString()));
        return temp;
    }

    bool hangUp(const QString &callID)
    {
        return DRing::hangUp(callID.toStdString());
    }

    bool hangUpConference(const QString &confID)
    {
        return DRing::hangUpConference(confID.toStdString());
    }

    bool hold(const QString &callID)
    {
        return DRing::hold(callID.toStdString());
    }

    bool holdConference(const QString &confID)
    {
        return DRing::holdConference(confID.toStdString());
    }

    bool isConferenceParticipant(const QString &callID)
    {
        return DRing::isConferenceParticipant(callID.toStdString());
    }

    bool joinConference(const QString &sel_confID, const QString &drag_confID)
    {
        return DRing::joinConference(
            sel_confID.toStdString(), drag_confID.toStdString());
    }

    bool joinParticipant(const QString &sel_callID, const QString &drag_callID)
    {
        return DRing::joinParticipant(
            sel_callID.toStdString(), drag_callID.toStdString());
    }

    QString placeCall(const QString &accountID, const QString &to)
    {
        QString temp(DRing::placeCall(accountID.toStdString(), to.toStdString()).c_str());
        return temp;
    }

    void playDTMF(const QString &key)
    {
        DRing::playDTMF(key.toStdString());
    }

    void recordPlaybackSeek(double value)
    {
        DRing::recordPlaybackSeek(value);
    }

    bool refuse(const QString &callID)
    {
        return DRing::refuse(callID.toStdString());
    }

    void sendTextMessage(const QString &callID, const QMap<QString,QString> &message, bool isMixed)
    {
        DRing::sendTextMessage(
            callID.toStdString(), convertMap(message), QObject::tr("Me").toStdString(), isMixed
        );
    }

    bool startRecordedFilePlayback(const QString &filepath)
    {
        // TODO: Change method name to match API
        return DRing::startRecordedFilePlayback(filepath.toStdString());
    }

    void startTone(int start, int type)
    {
        DRing::startTone(start, type);
    }

    void stopRecordedFilePlayback(const QString &filepath)
    {
        DRing::stopRecordedFilePlayback(filepath.toStdString());
    }

    bool toggleRecording(const QString &callID)
    {
        return DRing::toggleRecording(callID.toStdString());
    }

    bool transfer(const QString &callID, const QString &to)
    {
        return DRing::transfer(
            callID.toStdString(), to.toStdString());
    }

    bool unhold(const QString &callID)
    {
        return DRing::unhold(callID.toStdString());
    }

    bool unholdConference(const QString &confID)
    {
        return DRing::unholdConference(confID.toStdString());
    }

    bool muteLocalMedia(const QString& callid, const QString& mediaType, bool mute)
    {
        return DRing::muteLocalMedia(callid.toStdString(), mediaType.toStdString(), mute);
    }

    void startSmartInfo(int refresh)
    {
        DRing::startSmartInfo(refresh);
    }

    void stopSmartInfo()
    {
        DRing::stopSmartInfo();
    }

Q_SIGNALS: // SIGNALS
    void callStateChanged(const QString &callID, const QString &state, int code);
    void transferFailed();
    void transferSucceeded();
    void recordPlaybackStopped(const QString &filepath);
    void voiceMailNotify(const QString &accountID, int count);
    void incomingMessage(const QString &callID, const QString &from, const MapStringString &message);
    void incomingCall(const QString &accountID, const QString &callID, const QString &from);
    void recordPlaybackFilepath(const QString &callID, const QString &filepath);
    void conferenceCreated(const QString &confID);
    void conferenceChanged(const QString &confID, const QString &state);
    void updatePlaybackScale(const QString &filepath, int position, int size);
    void conferenceRemoved(const QString &confID);
    void newCallCreated(const QString &accountID, const QString &callID, const QString &to);
    void recordingStateChanged(const QString &callID, bool recordingState);
    void onRtcpReportReceived(const QString &callID, MapStringInt report);
    void audioMuted(const QString &callID, bool state);
    void videoMuted(const QString &callID, bool state);
    void peerHold(const QString &callID, bool state);
    void smartInfo(const MapStringString& info);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::CallManagerInterface CallManager;
    }
  }
}

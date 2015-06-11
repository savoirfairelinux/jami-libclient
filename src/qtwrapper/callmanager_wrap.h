/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
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
#ifndef CALLMANAGERINTERFACE_H
#define CALLMANAGERINTERFACE_H

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
                    QTimer::singleShot(0, [this,callID, state, code] {
                        LOG_DRING_SIGNAL3("callStateChanged",QString(callID.c_str()) , QString(state.c_str()) , code);
                        Q_EMIT this->callStateChanged(QString(callID.c_str()), QString(state.c_str()), code);
                    });
            }),
            exportable_callback<CallSignal::TransferFailed>(
                [this] () {
                       QTimer::singleShot(0, [this] {
                             LOG_DRING_SIGNAL("transferFailed","");
                             Q_EMIT this->transferFailed();
                       });
            }),
            exportable_callback<CallSignal::TransferSucceeded>(
                [this] () {
                       QTimer::singleShot(0, [this] {
                             LOG_DRING_SIGNAL("transferSucceeded","");
                             Q_EMIT this->transferSucceeded();
                       });
            }),
            exportable_callback<CallSignal::RecordPlaybackStopped>(
                [this] (const std::string &filepath) {
                       QTimer::singleShot(0, [this,filepath] {
                             LOG_DRING_SIGNAL("recordPlaybackStopped",QString(filepath.c_str()));
                             Q_EMIT this->recordPlaybackStopped(QString(filepath.c_str()));
                       });
            }),
            exportable_callback<CallSignal::VoiceMailNotify>(
                [this] (const std::string &accountID, int count) {
                       QTimer::singleShot(0, [this,accountID, count] {
                             LOG_DRING_SIGNAL2("voiceMailNotify",QString(accountID.c_str()), count);
                             Q_EMIT this->voiceMailNotify(QString(accountID.c_str()), count);
                       });
            }),
            exportable_callback<CallSignal::IncomingMessage>(
                [this] (const std::string &callID, const std::string &from, const std::string &message) {
                       QTimer::singleShot(0, [this,callID, from, message] {
                             LOG_DRING_SIGNAL3("incomingMessage",QString(callID.c_str()),QString(from.c_str()),QString(message.c_str()));
                             Q_EMIT this->incomingMessage(QString(callID.c_str()), QString(from.c_str()), QString(message.c_str()));
                       });
            }),
            exportable_callback<CallSignal::IncomingCall>(
                [this] (const std::string &accountID, const std::string &callID, const std::string &from) {
                       QTimer::singleShot(0, [this,accountID, callID, from] {
                             LOG_DRING_SIGNAL3("incomingCall",QString(accountID.c_str()), QString(callID.c_str()), QString(from.c_str()));
                             Q_EMIT this->incomingCall(QString(accountID.c_str()), QString(callID.c_str()), QString(from.c_str()));
                       });
            }),
            exportable_callback<CallSignal::RecordPlaybackFilepath>(
                [this] (const std::string &callID, const std::string &filepath) {
                       QTimer::singleShot(0, [this,callID, filepath] {
                             LOG_DRING_SIGNAL2("recordPlaybackFilepath",QString(callID.c_str()), QString(filepath.c_str()));
                             Q_EMIT this->recordPlaybackFilepath(QString(callID.c_str()), QString(filepath.c_str()));
                       });
            }),
            exportable_callback<CallSignal::ConferenceCreated>(
                [this] (const std::string &confID) {
                       QTimer::singleShot(0, [this,confID] {
                             LOG_DRING_SIGNAL("conferenceCreated",QString(confID.c_str()));
                             Q_EMIT this->conferenceCreated(QString(confID.c_str()));
                       });
            }),
            exportable_callback<CallSignal::ConferenceChanged>(
                [this] (const std::string &confID, const std::string &state) {
                       QTimer::singleShot(0, [this,confID, state] {
                             LOG_DRING_SIGNAL2("conferenceChanged",QString(confID.c_str()), QString(state.c_str()));
                             Q_EMIT this->conferenceChanged(QString(confID.c_str()), QString(state.c_str()));
                       });
            }),
            exportable_callback<CallSignal::UpdatePlaybackScale>(
                [this] (const std::string &filepath, int position, int size) {
                       QTimer::singleShot(0, [this,filepath, position, size] {
                             LOG_DRING_SIGNAL3("updatePlaybackScale",QString(filepath.c_str()), position, size);
                             Q_EMIT this->updatePlaybackScale(QString(filepath.c_str()), position, size);
                       });
            }),
            exportable_callback<CallSignal::ConferenceRemoved>(
                [this] (const std::string &confID) {
                       QTimer::singleShot(0, [this,confID] {
                             LOG_DRING_SIGNAL("conferenceRemoved",QString(confID.c_str()));
                             Q_EMIT this->conferenceRemoved(QString(confID.c_str()));
                       });
            }),
            exportable_callback<CallSignal::NewCallCreated>(
                [this] (const std::string &accountID, const std::string &callID, const std::string &to) {
                       QTimer::singleShot(0, [this,accountID, callID, to] {
                             LOG_DRING_SIGNAL3("newCallCreated",QString(accountID.c_str()), QString(callID.c_str()), QString(to.c_str()));
                             Q_EMIT this->newCallCreated(QString(accountID.c_str()), QString(callID.c_str()), QString(to.c_str()));
                       });
            }),
            exportable_callback<CallSignal::RecordingStateChanged>(
                [this] (const std::string &callID, bool recordingState) {
                       QTimer::singleShot(0, [this,callID, recordingState] {
                             LOG_DRING_SIGNAL2("recordingStateChanged",QString(callID.c_str()), recordingState);
                             Q_EMIT this->recordingStateChanged(QString(callID.c_str()), recordingState);
                       });
            }),
            exportable_callback<CallSignal::SecureSdesOn>(
                [this] (const std::string &callID) {
                       QTimer::singleShot(0, [this,callID] {
                             LOG_DRING_SIGNAL("secureSdesOn",QString(callID.c_str()));
                             Q_EMIT this->secureSdesOn(QString(callID.c_str()));
                       });
            }),
            exportable_callback<CallSignal::SecureSdesOff>(
                [this] (const std::string &callID) {
                       QTimer::singleShot(0, [this,callID] {
                             LOG_DRING_SIGNAL("secureSdesOff",QString(callID.c_str()));
                             Q_EMIT this->secureSdesOff(QString(callID.c_str()));
                       });
            }),
            exportable_callback<CallSignal::SecureZrtpOn>(
                [this] (const std::string &callID, const std::string &cipher) {
                       QTimer::singleShot(0, [this,callID,cipher] {
                             LOG_DRING_SIGNAL2("secureZrtpOn",QString(callID.c_str()), QString(cipher.c_str()));
                             Q_EMIT this->secureZrtpOn(QString(callID.c_str()), QString(cipher.c_str()));
                       });
            }),
            exportable_callback<CallSignal::SecureZrtpOff>(
                [this] (const std::string &callID) {
                       QTimer::singleShot(0, [this,callID] {
                             Q_EMIT this->secureZrtpOff(QString(callID.c_str()));
                       });
            }),
            exportable_callback<CallSignal::ShowSAS>(
                [this] (const std::string &callID, const std::string &sas, bool verified) {
                       QTimer::singleShot(0, [this,callID, sas, verified] {
                             LOG_DRING_SIGNAL3("showSAS",QString(callID.c_str()), QString(sas.c_str()), verified);
                             Q_EMIT this->showSAS(QString(callID.c_str()), QString(sas.c_str()), verified);
                       });
            }),
            exportable_callback<CallSignal::ZrtpNotSuppOther>(
                [this] (const std::string &callID) {
                       QTimer::singleShot(0, [this,callID] {
                             LOG_DRING_SIGNAL("zrtpNotSuppOther",QString(callID.c_str()));
                             Q_EMIT this->zrtpNotSuppOther(QString(callID.c_str()));
                       });
             }),
             exportable_callback<CallSignal::ZrtpNegotiationFailed>(
                 [this] (const std::string &callID, const std::string &reason, const std::string &severity) {
                       QTimer::singleShot(0, [this,callID, reason, severity] {
                             LOG_DRING_SIGNAL3("zrtpNegotiationFailed",QString(callID.c_str()), QString(reason.c_str()), QString(severity.c_str()));
                             Q_EMIT this->zrtpNegotiationFailed(QString(callID.c_str()), QString(reason.c_str()), QString(severity.c_str()));
                       });
             }),
             exportable_callback<CallSignal::RtcpReportReceived>(
                 [this] (const std::string &callID, const std::map<std::string, int>& report) {
                       QTimer::singleShot(0, [this,callID, report] {
                             LOG_DRING_SIGNAL2("onRtcpReportReceived",QString(callID.c_str()), convertStringInt(report));
                             Q_EMIT this->onRtcpReportReceived(QString(callID.c_str()), convertStringInt(report));
                       });
             }),
             exportable_callback<CallSignal::AudioMuted>(
                 [this] (const std::string &callID, bool state) {
                       QTimer::singleShot(0, [this,callID, state] {
                             LOG_DRING_SIGNAL2("audioMuted",QString(callID.c_str()), state);
                             Q_EMIT this->audioMuted(QString(callID.c_str()), state);
                       });
             }),
             exportable_callback<CallSignal::VideoMuted>(
                 [this] (const std::string &callID, bool state) {
                       QTimer::singleShot(0, [this,callID, state] {
                             LOG_DRING_SIGNAL2("videoMuted",QString(callID.c_str()), state);
                             Q_EMIT this->videoMuted(QString(callID.c_str()), state);
                       });
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

    void acceptEnrollment(const QString &callID, bool accepted)
    {
        DRing::acceptEnrollment(callID.toStdString(), accepted);
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

    Q_DECL_DEPRECATED QString getCurrentAudioCodecName(const QString &callID)
    {
        QString temp(
            DRing::getCurrentAudioCodecName(callID.toStdString()).c_str());
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

    void requestGoClear(const QString &callID)
    {
        DRing::requestGoClear(callID.toStdString());
    }

    void resetSASVerified(const QString &callID)
    {
        DRing::resetSASVerified(callID.toStdString());
    }

    void sendTextMessage(const QString &callID, const QString &message)
    {
        DRing::sendTextMessage(
            callID.toStdString(), message.toStdString());
    }

    void setConfirmGoClear(const QString &callID)
    {
        DRing::setConfirmGoClear(callID.toStdString());
    }

    void setSASVerified(const QString &callID)
    {
        DRing::setSASVerified(callID.toStdString());
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

Q_SIGNALS: // SIGNALS
    void callStateChanged(const QString &callID, const QString &state, int code);
    void transferFailed();
    void transferSucceeded();
    void recordPlaybackStopped(const QString &filepath);
    void voiceMailNotify(const QString &accountID, int count);
    void incomingMessage(const QString &callID, const QString &from, const QString &message);
    void incomingCall(const QString &accountID, const QString &callID, const QString &from);
    void recordPlaybackFilepath(const QString &callID, const QString &filepath);
    void conferenceCreated(const QString &confID);
    void conferenceChanged(const QString &confID, const QString &state);
    void updatePlaybackScale(const QString &filepath, int position, int size);
    void conferenceRemoved(const QString &confID);
    void newCallCreated(const QString &accountID, const QString &callID, const QString &to);
    void recordingStateChanged(const QString &callID, bool recordingState);
    void secureSdesOn(const QString &callID);
    void secureSdesOff(const QString &callID);
    void secureZrtpOn(const QString &callID, const QString &cipher);
    void secureZrtpOff(const QString &callID);
    void showSAS(const QString &callID, const QString &sas, bool verified);
    void zrtpNotSuppOther(const QString &callID);
    void zrtpNegotiationFailed(const QString &callID, const QString &reason, const QString &severity);
    void onRtcpReportReceived(const QString &callID, MapStringInt report);
    void confirmGoClear(const QString &callID);
    void audioMuted(const QString &callID, bool state);
    void videoMuted(const QString &callID, bool state);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::CallManagerInterface CallManager;
    }
  }
}
#endif

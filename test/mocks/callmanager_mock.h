/******************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                                 *
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
#include "qtwrapper/conversions_wrap.hpp"

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

     }

    ~CallManagerInterface() {}

    bool isValid() { return true; }

    void emitCallStateChanged(const QString &callID, const QString &state, int code) {
        emit callStateChanged(callID, state, code);
    }

public Q_SLOTS: // METHODS
    bool accept(const QString &callID)
    {
        emit callStateChanged(callID, "CURRENT", -1);
        return true;
    }

    bool addMainParticipant(const QString &confID)
    {
        Q_UNUSED(confID)
        return false;
    }

    bool addParticipant(const QString &callID, const QString &confID)
    {
        Q_UNUSED(callID)
        Q_UNUSED(confID)
        return false;
    }

    bool attendedTransfer(const QString &transferID, const QString &targetID)
    {
        Q_UNUSED(transferID)
        Q_UNUSED(targetID)
        return false;
    }

    void createConfFromParticipantList(const QStringList &participants)
    {
        Q_UNUSED(participants)

    }

    bool detachParticipant(const QString &callID)
    {
        Q_UNUSED(callID)
        return false;
    }

    MapStringString getCallDetails(const QString &callID)
    {
        Q_UNUSED(callID)
        return MapStringString();
    }

    QStringList getCallList()
    {
        QStringList temp;
        return temp;
    }

    MapStringString getConferenceDetails(const QString &callID)
    {
        Q_UNUSED(callID)
        MapStringString temp;
        return temp;
    }

    QString getConferenceId(const QString &callID)
    {
        Q_UNUSED(callID)
        QString temp;
        return temp;
    }

    QStringList getConferenceList()
    {
        QStringList temp;
        return temp;
    }

    QStringList getDisplayNames(const QString &confID)
    {
        Q_UNUSED(confID)
        QStringList temp;
        return temp;
    }

    bool getIsRecording(const QString &callID)
    {
        Q_UNUSED(callID)
        //TODO: match API
        return false;
    }

    QStringList getParticipantList(const QString &confID)
    {
        return confID.split(",");
    }

    bool hangUp(const QString &callID)
    {
        emit callStateChanged(callID, "OVER", -1);
        return true;
    }

    bool hangUpConference(const QString &confID)
    {
        Q_UNUSED(confID)
        return false;
    }

    bool hold(const QString &callID)
    {
        emit callStateChanged(callID, "HOLD", -1);
        return true;
    }

    bool holdConference(const QString &confID)
    {
        Q_UNUSED(confID)
        return false;
    }

    bool isConferenceParticipant(const QString &callID)
    {
        Q_UNUSED(callID)
        return false;
    }

    bool joinConference(const QString &sel_confID, const QString &drag_confID)
    {
        Q_UNUSED(sel_confID)
        Q_UNUSED(drag_confID)
        return false;
    }

    bool joinParticipant(const QString &sel_callID, const QString &drag_callID)
    {
        emit conferenceCreated(sel_callID + "," + drag_callID);
        return true;
    }

    QString placeCall(const QString &accountID, const QString &to)
    {
        emit newCallCreated(accountID, to, to);
        return to;
    }

#ifdef ENABLE_LIBWRAP
    QString placeCall(const QString &accountID, const QString &to, const std::map<std::string, std::string>& VolatileCallDetails)
    {
        emit newCallCreated(accountID, to, to);
        return to;
    }
#else // dbus
    QString  placeCallWithDetails(const QString &accountID, const QString &to, const std::map<std::string, std::string>& VolatileCallDetails)
    {
       emit newCallCreated(accountID, to, to);
       return to;
    }
#endif // ENABLE_LIBWRAP


    void playDTMF(const QString &key)
    {
        Q_UNUSED(key)
    }

    void recordPlaybackSeek(double value)
    {
        Q_UNUSED(value)
    }

    bool refuse(const QString &callID)
    {
        Q_UNUSED(callID)
        return false;
    }

    void sendTextMessage(const QString &callID, const QMap<QString,QString> &message, bool isMixed)
    {
        Q_UNUSED(callID)
        Q_UNUSED(message)
        Q_UNUSED(isMixed)
    }

    bool startRecordedFilePlayback(const QString &filepath)
    {
        Q_UNUSED(filepath)
        return false;
    }

    void startTone(int start, int type)
    {
        Q_UNUSED(start)
        Q_UNUSED(type)
    }

    void stopRecordedFilePlayback()
    { }

    bool toggleRecording(const QString &callID)
    {
        Q_UNUSED(callID)
        return false;
    }

    bool transfer(const QString &callID, const QString &to)
    {
        Q_UNUSED(callID)
        Q_UNUSED(to)
        return false;
    }

    bool unhold(const QString &callID)
    {
        emit callStateChanged(callID, "CURRENT", -1);
        return true;
    }

    bool unholdConference(const QString &confID)
    {
        Q_UNUSED(confID)
        return false;
    }

    bool muteLocalMedia(const QString& callid, const QString& mediaType, bool mute)
    {
        Q_UNUSED(callid)
        Q_UNUSED(mediaType)
        Q_UNUSED(mute)
        return false;
    }

    void startSmartInfo(int refresh)
    {
        Q_UNUSED(refresh)
    }

    void stopSmartInfo()
    {
    }

    void emitIncomingCall(const QString &accountID, const QString &callID, const QString &from)
    {
        emit incomingCall(accountID, callID, from);
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

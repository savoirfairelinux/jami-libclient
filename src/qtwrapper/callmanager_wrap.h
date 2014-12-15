/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
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
#ifndef CALLMANAGER_DBUS_INTERFACE_H
#define CALLMANAGER_DBUS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include <sflphone.h>
#include "../dbus/metatypes.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.sflphone.SFLphone.CallManager
 */
class CallManagerInterface: public QObject
{
    Q_OBJECT

public:

    CallManagerInterface()
    {
        call_ev_handlers = sflph_call_ev_handlers {
            .on_state_change = [this] (const std::string &callID, const std::string &state) { printf("EMIT ONSTATECHANGE\n"); emit this->callStateChanged(QString(callID.c_str()), QString(state.c_str())); },
            .on_transfer_fail = [this] () { emit this->transferFailed(); },
            .on_transfer_success = [this] () { emit this->transferSucceeded(); },
            .on_record_playback_stopped = [this] (const std::string &filepath) { emit this->recordPlaybackStopped(QString(filepath.c_str())); },
            .on_voice_mail_notify = [this] (const std::string &accountID, int count) { emit this->voiceMailNotify(QString(accountID.c_str()), count); },
            .on_incoming_message = [this] (const std::string &callID, const std::string &from, const std::string &message) { emit this->incomingMessage(QString(callID.c_str()), QString(from.c_str()), QString(message.c_str())); },
            .on_incoming_call = [this] (const std::string &accountID, const std::string &callID, const std::string &from) { emit this->incomingCall(QString(accountID.c_str()), QString(callID.c_str()), QString(from.c_str())); },
            .on_record_playback_filepath = [this] (const std::string &callID, const std::string &filepath) { emit this->recordPlaybackFilepath(QString(callID.c_str()), QString(filepath.c_str())); },
            .on_conference_created = [this] (const std::string &confID) { emit this->conferenceCreated(QString(confID.c_str())); },
            .on_conference_changed = [this] (const std::string &confID, const std::string &state) { emit this->conferenceChanged(QString(confID.c_str()), QString(state.c_str())); },
            .on_update_playback_scale = [this] (const std::string &filepath, int position, int size) { emit this->updatePlaybackScale(QString(filepath.c_str()), position, size); },
            .on_conference_remove = [this] (const std::string &confID) { emit this->conferenceRemoved(QString(confID.c_str())); },
            .on_new_call = [this] (const std::string &accountID, const std::string &callID, const std::string &to) { printf("EMIT ONNEWCALL\n"); emit this->newCallCreated(QString(accountID.c_str()), QString(callID.c_str()), QString(to.c_str())); },
            .on_sip_call_state_change = [this] (const std::string &callID, const std::string &state, int code) { emit this->sipCallStateChanged(QString(callID.c_str()), QString(state.c_str()), code); },
            .on_record_state_change = [this] (const std::string &callID, bool recordingState) { emit this->recordingStateChanged(QString(callID.c_str()), recordingState); },
            .on_secure_sdes_on = [this] (const std::string &callID) { emit this->secureSdesOn(QString(callID.c_str())); },
            .on_secure_sdes_off = [this] (const std::string &callID) { emit this->secureSdesOff(QString(callID.c_str())); },
            .on_secure_zrtp_on = [this] (const std::string &callID, const std::string &cipher) { emit this->secureZrtpOn(QString(callID.c_str()), QString(cipher.c_str())); },
            .on_secure_zrtp_off = [this] (const std::string &callID) { emit this->secureZrtpOff(QString(callID.c_str())); },
            .on_show_sas = [this] (const std::string &callID, const std::string &sas, bool verified) { emit this->showSAS(QString(callID.c_str()), QString(sas.c_str()), verified); },
            .on_zrtp_not_supp_other = [this] (const std::string &callID) { emit this->zrtpNotSuppOther(QString(callID.c_str())); },
            .on_zrtp_negotiation_fail = [this] (const std::string &callID, const std::string &reason, const std::string &severity) { emit this->zrtpNegotiationFailed(QString(callID.c_str()), QString(reason.c_str()), QString(severity.c_str())); },
            .on_rtcp_receive_report = [this] (const std::string &callID, const std::map<std::string, int>& report) { emit this->onRtcpReportReceived(QString(callID.c_str()), convertStringInt(report)); }
            };
    }

    ~CallManagerInterface() {}

    bool isValid() { return true; }

    sflph_call_ev_handlers call_ev_handlers;

public Q_SLOTS: // METHODS
    bool accept(const QString &callID)
    {
        return sflph_call_accept(callID.toStdString());
    }

    void acceptEnrollment(const QString &callID, bool accepted)
    {
        sflph_call_accept_enrollment(callID.toStdString(), accepted);
    }

    bool addMainParticipant(const QString &confID)
    {
        return sflph_call_add_main_participant(confID.toStdString());
    }

    bool addParticipant(const QString &callID, const QString &confID)
    {
        return sflph_call_add_participant(
            callID.toStdString(), confID.toStdString());
    }

    bool attendedTransfer(const QString &transferID, const QString &targetID)
    {
        return sflph_call_attended_transfer(
            transferID.toStdString(), targetID.toStdString());
    }

    void createConfFromParticipantList(const QStringList &participants)
    {
        sflph_call_create_conf_from_participant_list(
            convertStringList(participants));
    }

    bool detachParticipant(const QString &callID)
    {
        return sflph_call_detach_participant(callID.toStdString());
    }

    MapStringString getCallDetails(const QString &callID)
    {
        MapStringString temp =
            convertMap(sflph_call_get_call_details(callID.toStdString()));
        return temp;
    }

    QStringList getCallList()
    {
        QStringList temp =
            convertStringList(sflph_call_get_call_list());
        return temp;
    }

    MapStringString getConferenceDetails(const QString &callID)
    {
        MapStringString temp =
            convertMap(sflph_call_get_conference_details(
                callID.toStdString()));
        return temp;
    }

    QString getConferenceId(const QString &callID)
    {
        QString temp(sflph_call_get_conference_id(callID.toStdString()).c_str());
        return temp;
    }

    QStringList getConferenceList()
    {
        QStringList temp =
            convertStringList(sflph_call_get_conference_list());
        return temp;
    }

    Q_DECL_DEPRECATED QString getCurrentAudioCodecName(const QString &callID)
    {
        QString temp(
            sflph_call_get_current_audio_codec_name(callID.toStdString()).c_str());
        return temp;
    }

    QStringList getDisplayNames(const QString &confID)
    {
        QStringList temp =
            convertStringList(sflph_call_get_display_names(
                confID.toStdString()));
        return temp;
    }

    bool getIsRecording(const QString &callID)
    {
        //TODO: match API
        return sflph_call_is_recording(callID.toStdString());
    }

    QStringList getParticipantList(const QString &confID)
    {
        QStringList temp =
            convertStringList(sflph_call_get_participant_list(
                confID.toStdString()));
        return temp;
    }

    bool hangUp(const QString &callID)
    {
        return sflph_call_hang_up(callID.toStdString());
    }

    bool hangUpConference(const QString &confID)
    {
        return sflph_call_hang_up_conference(confID.toStdString());
    }

    bool hold(const QString &callID)
    {
        return sflph_call_hold(callID.toStdString());
    }

    bool holdConference(const QString &confID)
    {
        return sflph_call_hold_conference(confID.toStdString());
    }

    bool isConferenceParticipant(const QString &callID)
    {
        return sflph_call_is_conference_participant(callID.toStdString());
    }

    bool joinConference(const QString &sel_confID, const QString &drag_confID)
    {
        return sflph_call_join_conference(
            sel_confID.toStdString(), drag_confID.toStdString());
    }

    bool joinParticipant(const QString &sel_callID, const QString &drag_callID)
    {
        return sflph_call_join_participant(
            sel_callID.toStdString(), drag_callID.toStdString());
    }

    bool placeCall(const QString &accountID, const QString &callID, const QString &to)
    {
        //TODO: match API
        return sflph_call_place(
            accountID.toStdString(), callID.toStdString(), to.toStdString());
    }

    void playDTMF(const QString &key)
    {
        sflph_call_play_dtmf(key.toStdString());
    }

    void recordPlaybackSeek(double value)
    {
        sflph_call_record_playback_seek(value);
    }

    bool refuse(const QString &callID)
    {
        return sflph_call_refuse(callID.toStdString());
    }

    void requestGoClear(const QString &callID)
    {
        sflph_call_request_go_clear(callID.toStdString());
    }

    void resetSASVerified(const QString &callID)
    {
        sflph_call_reset_sas_verified(callID.toStdString());
    }

    void sendTextMessage(const QString &callID, const QString &message)
    {
        sflph_call_send_text_message(
            callID.toStdString(), message.toStdString());
    }

    void setConfirmGoClear(const QString &callID)
    {
        sflph_call_set_confirm_go_clear(callID.toStdString());
    }

    Q_DECL_DEPRECATED void setRecording(const QString &callID)
    {
        sflph_call_set_recording(callID.toStdString());
    }

    void setSASVerified(const QString &callID)
    {
        sflph_call_set_sas_verified(callID.toStdString());
    }

    bool startRecordedFilePlayback(const QString &filepath)
    {
        // TODO: Change method name to match API
        return sflph_call_play_recorded_file(filepath.toStdString());
    }

    void startTone(int start, int type)
    {
        sflph_call_start_tone(start, type);
    }

    void stopRecordedFilePlayback(const QString &filepath)
    {
        sflph_call_stop_recorded_file(filepath.toStdString());
    }

    bool toggleRecording(const QString &callID)
    {
        return sflph_call_toggle_recording(callID.toStdString());
    }

    bool transfer(const QString &callID, const QString &to)
    {
        return sflph_call_transfer(
            callID.toStdString(), to.toStdString());
    }

    bool unhold(const QString &callID)
    {
        return sflph_call_unhold(callID.toStdString());
    }

    bool unholdConference(const QString &confID)
    {
        return sflph_call_unhold_conference(confID.toStdString());
    }

Q_SIGNALS: // SIGNALS
    void callStateChanged(const QString &callID, const QString &state);
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
    void sipCallStateChanged(const QString &callID, const QString &state, int code);
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

};

namespace org {
  namespace sflphone {
    namespace SFLphone {
      typedef ::CallManagerInterface CallManager;
    }
  }
}
#endif

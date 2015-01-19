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
#ifndef CONFIGURATIONMANAGER_DBUS_INTERFACE_H
#define CONFIGURATIONMANAGER_DBUS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <future>

#include <ring.h>
#include "../dbus/metatypes.h"
#include "conversions_wrap.hpp"

// TEMPORARY
#include <iostream>


/*
 * Proxy class for interface org.ring.Ring.ConfigurationManager
 */
class ConfigurationManagerInterface: public QObject
{
    Q_OBJECT

public:
    ConfigurationManagerInterface()
    {
        setObjectName("ConfigurationManagerInterface");
        config_ev_handlers = {
            .on_volume_change = [this] (const std::string &device, double value) { emit this->volumeChanged(QString(device.c_str()), value); },
            .on_accounts_change = [this] () { emit this->accountsChanged(); },
            .on_history_change = [this] () { emit this->historyChanged(); },
            .on_stun_status_fail = [this] (const std::string &reason) { emit this->stunStatusFailure(QString(reason.c_str())); },
            .on_registration_state_change = [this] (const std::string &accountID, int registration_state) { emit this->registrationStateChanged(QString(accountID.c_str()), registration_state); },
            .on_sip_registration_state_change = [this] (const std::string &accountID, const std::string &state, int code) { emit this->sipRegistrationStateChanged(QString(accountID.c_str()), QString(state.c_str()), code); },
            //TODO: .on_volatile_details_change = [this] (const QString &message.c_str()) { emit this->volatileAccountDetailsChanged(); },
            .on_error = [this] (int code) { emit this->errorAlert(code); }
        };
    }

    ~ConfigurationManagerInterface() {}

    ring_config_ev_handlers config_ev_handlers;

public Q_SLOTS: // METHODS
    QString addAccount(MapStringString details)
    {
        QString temp(
            ring_config_add_account(convertMap(details)).c_str());
        return temp;
    }

    bool checkCertificateValidity(const QString &caPath, const QString &pemPath)
    {
        return ring_config_check_certificate_validity(
            caPath.toStdString(), pemPath.toStdString());
    }

    bool checkForPrivateKey(const QString &pemPath)
    {
        return ring_config_check_for_private_key(pemPath.toStdString());
    }

    bool checkHostnameCertificate(const QString &host, const QString &port)
    {
        return ring_config_check_hostname_certificate(
            host.toStdString(), port.toStdString());
    }

    void clearHistory()
    {
        ring_config_clear_history();
    }

    MapStringString getAccountDetails(const QString &accountID)
    {
        MapStringString temp =
            convertMap(ring_config_get_account_details(accountID.toStdString()));
        return temp;
    }

    QStringList getAccountList()
    {
        std::cout << "AccountList:" << std::endl;
        for (auto x : ring_config_get_account_list())
            std::cout << x << std::endl;

        QStringList temp =
            convertStringList(ring_config_get_account_list());
        return temp;
    }

    MapStringString getAccountTemplate()
    {
        MapStringString temp =
            convertMap(ring_config_get_account_template());
        return temp;
    }

// TODO: works?
    VectorInt getActiveAudioCodecList(const QString &accountID)
    {
        return QVector<int>::fromStdVector(
            ring_config_get_active_audio_codec_list(accountID.toStdString()));
    }

    QString getAddrFromInterfaceName(const QString &interface)
    {
        QString temp(
            ring_config_get_addr_from_interface_name(interface.toStdString()).c_str());
        return temp;
    }

    QStringList getAllIpInterface()
    {
        QStringList temp =
            convertStringList(ring_config_get_all_ip_interface());
        return temp;
    }

    QStringList getAllIpInterfaceByName()
    {
        QStringList temp =
            convertStringList(ring_config_get_all_ip_interface_by_name());
        return temp;
    }

    QStringList getAudioCodecDetails(int payload)
    {
        QStringList temp =
            convertStringList(ring_config_get_audio_codec_details(payload));
        return temp;
    }

// TODO: works?
    VectorInt getAudioCodecList()
    {
        return QVector<int>::fromStdVector(ring_config_get_audio_codec_list());
    }

    int getAudioInputDeviceIndex(const QString &devname)
    {
        return ring_config_get_audio_input_device_index(devname.toStdString());
    }

    QStringList getAudioInputDeviceList()
    {
        QStringList temp =
            convertStringList(ring_config_get_audio_input_device_list());
        return temp;
    }

    QString getAudioManager()
    {
        QString temp(
            ring_config_get_audio_manager().c_str());
        return temp;
    }

    int getAudioOutputDeviceIndex(const QString &devname)
    {
        return ring_config_get_audio_output_device_index(devname.toStdString());
    }

    QStringList getAudioOutputDeviceList()
    {
        QStringList temp =
            convertStringList(ring_config_get_audio_output_device_list());
        return temp;
    }

    QStringList getAudioPluginList()
    {
        QStringList temp =
            convertStringList(ring_config_get_audio_plugin_list());
        return temp;
    }

    VectorMapStringString getCredentials(const QString &accountID)
    {
        VectorMapStringString temp;
        for(auto x : ring_config_get_credentials(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    QStringList getCurrentAudioDevicesIndex()
    {
        QStringList temp =
            convertStringList(ring_config_get_current_audio_devices_index());
        return temp;
    }

    QString getCurrentAudioOutputPlugin()
    {
        QString temp(
            ring_config_get_current_audio_output_plugin().c_str());
        return temp;
    }

    VectorMapStringString getHistory()
    {
        VectorMapStringString temp;
        for (auto x : ring_config_get_history()) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    int getHistoryLimit()
    {
        return ring_config_get_history_limit();
    }

    MapStringString getHookSettings()
    {
        MapStringString temp =
            convertMap(ring_config_get_hook_settings());
        return temp;
    }

    MapStringString getIp2IpDetails()
    {
        MapStringString temp =
            convertMap(ring_config_get_ip2ip_details());
        return temp;
    }

    bool getIsAlwaysRecording()
    {
        // TODO: update API
        return ring_config_is_always_recording();
    }

    bool getNoiseSuppressState()
    {
        return ring_config_get_noise_suppress_state();
    }

    QString getRecordPath()
    {
        QString temp(
            ring_config_get_record_path().c_str());
        return temp;
    }

    MapStringString getRingtoneList()
    {
        MapStringString temp =
            convertMap(ring_config_get_ringtone_list());
        return temp;
    }

    MapStringString getShortcuts()
    {
        MapStringString temp =
            convertMap(ring_config_get_shortcuts());
        return temp;
    }

    QStringList getSupportedAudioManagers()
    {
        QStringList temp =
            convertStringList(ring_config_get_supported_audio_managers());
        return temp;
    }

    QStringList getSupportedTlsMethod()
    {
        QStringList temp =
            convertStringList(ring_config_get_supported_tls_method());
        return temp;
    }

    MapStringString getTlsSettings()
    {
        MapStringString temp =
            convertMap(ring_config_get_tls_settings());
        return temp;
    }

    MapStringString getTlsSettingsDefault()
    {
        // TODO: update API
        MapStringString temp =
            convertMap(ring_config_get_tls_default_settings());
        return temp;
    }

    double getVolume(const QString &device)
    {
        return ring_config_get_volume(device.toStdString());
    }

    bool isAgcEnabled()
    {
        return ring_config_is_agc_enabled();
    }

    bool isCaptureMuted()
    {
        return ring_config_is_capture_muted();
    }

    bool isDtmfMuted()
    {
        return ring_config_is_dtmf_muted();
    }

    int isIax2Enabled()
    {
        return ring_config_is_iax2_enabled();
    }

    bool isPlaybackMuted()
    {
        return ring_config_is_playback_muted();
    }

    void muteCapture(bool mute)
    {
        ring_config_mute_capture(mute);
    }

    void muteDtmf(bool mute)
    {
        ring_config_mute_dtmf(mute);
    }

    void mutePlayback(bool mute)
    {
        ring_config_mute_playback(mute);
    }

    void registerAllAccounts()
    {
        ring_config_register_all_accounts();
    }

    void removeAccount(const QString &accountID)
    {
        ring_config_remove_account(accountID.toStdString());
    }

    void sendRegister(const QString &accountID, bool enable)
    {
        ring_config_send_register(accountID.toStdString(), enable);
    }

    void setAccountDetails(const QString &accountID, MapStringString details)
    {
        ring_config_set_account_details(accountID.toStdString(),
            convertMap(details));
    }

    void setAccountsOrder(const QString &order)
    {
        ring_config_set_accounts_order(order.toStdString());
    }

    void setActiveAudioCodecList(const QStringList &list, const QString &accountID)
    {
        ring_config_set_active_audio_codec_list(
            convertStringList(list), accountID.toStdString());
    }

    void setAgcState(bool enabled)
    {
        //TODO: update API
        ring_config_enable_agc(enabled);
    }

    void setAudioInputDevice(int index)
    {
        ring_config_set_audio_input_device(index);
    }

    bool setAudioManager(const QString &api)
    {
        return ring_config_set_audio_manager(api.toStdString());
    }

    void setAudioOutputDevice(int index)
    {
        ring_config_set_audio_output_device(index);
    }

    void setAudioPlugin(const QString &audioPlugin)
    {
        ring_config_set_audio_plugin(audioPlugin.toStdString());
    }

    void setAudioRingtoneDevice(int index)
    {
        ring_config_set_audio_ringtone_device(index);
    }

    void setCredentials(const QString &accountID, VectorMapStringString credentialInformation)
    {
        std::vector<std::map<std::string, std::string> > temp;
        for (auto x : credentialInformation) {
            temp.push_back(convertMap(x));
        }
        ring_config_set_credentials(accountID.toStdString(), temp);
    }

    void setHistoryLimit(int days)
    {
        ring_config_set_history_limit(days);
    }

    void setHookSettings(MapStringString settings)
    {
        ring_config_set_hook_settings(convertMap(settings));
    }

    void setIsAlwaysRecording(bool enabled)
    {
        //TODO: update API
        ring_config_set_always_recording(enabled);
    }

    void setNoiseSuppressState(bool state)
    {
        ring_config_set_noise_suppress_state(state);
    }

    void setRecordPath(const QString &rec)
    {
        ring_config_set_record_path(rec.toStdString());
    }

    void setShortcuts(MapStringString shortcutsMap)
    {
        ring_config_set_shortcuts(convertMap(shortcutsMap));
    }

    void setTlsSettings(MapStringString details)
    {
        ring_config_set_tls_settings(convertMap(details));
    }

    void setVolume(const QString &device, double value)
    {
        ring_config_set_volume(device.toStdString(), value);
    }

Q_SIGNALS: // SIGNALS
    void volumeChanged(const QString &device, double value);
    void accountsChanged();
    void historyChanged();
    void stunStatusFailure(const QString &reason);
    void registrationStateChanged(const QString &accountID, int registration_state);
    void sipRegistrationStateChanged(const QString &accountID, const QString &state, int code);
    void stunStatusSuccess(const QString &message);
    void errorAlert(int code);

};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::ConfigurationManagerInterface ConfigurationManager;
    }
  }
}
#endif

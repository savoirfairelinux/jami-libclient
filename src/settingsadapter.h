/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
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

#include <QObject>
#include <QSettings>

#include "api/account.h"
#include "api/datatransfermodel.h"
#include "lrcinstance.h"
#include "typedefs.h"
#include "utils.h"

class SettingsAdapter : public QObject
{
    Q_OBJECT
public:
    explicit SettingsAdapter(QObject* parent = nullptr);

    // Singleton
    static SettingsAdapter& instance();
    /*
     * getters of directories
     */
    Q_INVOKABLE QString getDir_Document();
    Q_INVOKABLE QString getDir_Download();

    Q_INVOKABLE QVariant getAppValue(const Settings::Key key);
    Q_INVOKABLE void setAppValue(const Settings::Key key, const QVariant& value);

    Q_INVOKABLE void setRunOnStartUp(bool state);
    Q_INVOKABLE void setDownloadPath(QString dir);

    /*
     * getters of devices' Info and options
     */
    Q_INVOKABLE lrc::api::video::Capabilities get_DeviceCapabilities(const QString& device);
    Q_INVOKABLE lrc::api::video::ResRateList get_ResRateList(lrc::api::video::Channel channel,
                                                             QString device);
    Q_INVOKABLE int get_DeviceCapabilitiesSize(const QString& device);

    /*
     * getters of resolution and frame rates of current device
     */
    Q_INVOKABLE QVector<QString> getResolutions(const QString& device);
    Q_INVOKABLE QVector<int> getFrameRates(const QString& device);

    /*
     * getters and setters: lrc video::setting
     */
    Q_INVOKABLE QString get_Video_Settings_Channel(const QString& deviceId);
    Q_INVOKABLE QString get_Video_Settings_Name(const QString& deviceId);
    Q_INVOKABLE QString get_Video_Settings_Id(const QString& deviceId);
    Q_INVOKABLE qreal get_Video_Settings_Rate(const QString& deviceId);
    Q_INVOKABLE QString get_Video_Settings_Size(const QString& deviceId);

    Q_INVOKABLE void set_Video_Settings_Rate_And_Resolution(const QString& deviceId,
                                                            qreal rate,
                                                            const QString& resolution);

    /*
     * getters and setters of current account Info
     */
    const Q_INVOKABLE lrc::api::account::Info& getCurrentAccountInfo();
    const Q_INVOKABLE lrc::api::profile::Info& getCurrentAccount_Profile_Info();

    Q_INVOKABLE lrc::api::ContactModel* getContactModel();
    Q_INVOKABLE lrc::api::NewDeviceModel* getDeviceModel();

    Q_INVOKABLE QString get_CurrentAccountInfo_RegisteredName();
    Q_INVOKABLE QString get_CurrentAccountInfo_Id();
    Q_INVOKABLE bool get_CurrentAccountInfo_Enabled();

    // profile info
    Q_INVOKABLE QString getCurrentAccount_Profile_Info_Uri();
    Q_INVOKABLE QString getCurrentAccount_Profile_Info_Alias();
    Q_INVOKABLE int getCurrentAccount_Profile_Info_Type();
    Q_INVOKABLE QString getAccountBestName();

    // getters and setters of avatar image
    Q_INVOKABLE bool getIsDefaultAvatar();
    Q_INVOKABLE void setCurrAccAvatar(QVariant avatarImg);
    Q_INVOKABLE void clearCurrentAvatar();

    /*
     * getters and setters of ConfProperties_t
     */
    // getters
    Q_INVOKABLE lrc::api::account::ConfProperties_t getAccountConfig();
    Q_INVOKABLE QString getAccountConfig_Manageruri();
    Q_INVOKABLE QString getAccountConfig_Username();
    Q_INVOKABLE QString getAccountConfig_Hostname();
    Q_INVOKABLE QString getAccountConfig_Password();

    Q_INVOKABLE bool getAccountConfig_KeepAliveEnabled();
    Q_INVOKABLE QString getAccountConfig_RouteSet();
    Q_INVOKABLE QString getAccountConfig_ProxyServer();
    Q_INVOKABLE bool getAccountConfig_ProxyEnabled();

    Q_INVOKABLE bool getAccountConfig_PeerDiscovery();
    Q_INVOKABLE bool getAccountConfig_DHT_PublicInCalls();
    Q_INVOKABLE bool getAccountConfig_RendezVous();
    Q_INVOKABLE bool getAccountConfig_AutoAnswer();

    Q_INVOKABLE QString getAccountConfig_RingNS_Uri();

    Q_INVOKABLE QString getAccountConfig_TLS_CertificateListFile();
    Q_INVOKABLE QString getAccountConfig_TLS_CertificateFile();
    Q_INVOKABLE QString getAccountConfig_TLS_PrivateKeyFile();
    Q_INVOKABLE bool getAccountConfig_TLS_Enable();
    Q_INVOKABLE QString getAccountConfig_TLS_Password();
    Q_INVOKABLE bool getAccountConfig_TLS_VerifyServer();
    Q_INVOKABLE bool getAccountConfig_TLS_VerifyClient();
    Q_INVOKABLE bool getAccountConfig_TLS_RequireClientCertificate();
    Q_INVOKABLE int getAccountConfig_TLS_Method_inInt();
    Q_INVOKABLE QString getAccountConfig_TLS_Servername();
    Q_INVOKABLE int getAccountConfig_TLS_NegotiationTimeoutSec();

    Q_INVOKABLE bool getAccountConfig_SRTP_Enabled();
    Q_INVOKABLE int getAccountConfig_SRTP_KeyExchange();
    Q_INVOKABLE bool getAccountConfig_SRTP_RtpFallback();

    Q_INVOKABLE bool getAccountConfig_UpnpEnabled();
    Q_INVOKABLE bool getAccountConfig_TURN_Enabled();
    Q_INVOKABLE QString getAccountConfig_TURN_Server();
    Q_INVOKABLE QString getAccountConfig_TURN_Username();
    Q_INVOKABLE QString getAccountConfig_TURN_Password();
    Q_INVOKABLE QString getAccountConfig_TURN_Realm();

    Q_INVOKABLE bool getAccountConfig_STUN_Enabled();
    Q_INVOKABLE QString getAccountConfig_STUN_Server();

    Q_INVOKABLE bool getAccountConfig_Video_Enabled();
    Q_INVOKABLE int getAccountConfig_Video_VideoPortMin();
    Q_INVOKABLE int getAccountConfig_Video_VideoPortMax();

    Q_INVOKABLE int getAccountConfig_Audio_AudioPortMin();
    Q_INVOKABLE int getAccountConfig_Audio_AudioPortMax();

    Q_INVOKABLE bool getAccountConfig_Ringtone_RingtoneEnabled();
    Q_INVOKABLE QString getAccountConfig_Ringtone_RingtonePath();

    Q_INVOKABLE int getAccountConfig_Registration_Expire();
    Q_INVOKABLE int getAccountConfig_Localport();
    Q_INVOKABLE bool getAccountConfig_PublishedSameAsLocal();
    Q_INVOKABLE QString getAccountConfig_PublishedAddress();
    Q_INVOKABLE int getAccountConfig_PublishedPort();

    Q_INVOKABLE QString getAccountConfig_Mailbox();

    // setters
    Q_INVOKABLE void setAccountConfig_Username(QString input);
    Q_INVOKABLE void setAccountConfig_Hostname(QString input);
    Q_INVOKABLE void setAccountConfig_Password(QString input);
    Q_INVOKABLE void setAccountConfig_RouteSet(QString input);

    Q_INVOKABLE void setAutoConnectOnLocalNetwork(bool state);
    Q_INVOKABLE void setCallsUntrusted(bool state);
    Q_INVOKABLE void setIsRendezVous(bool state);
    Q_INVOKABLE void setAutoAnswerCalls(bool state);
    Q_INVOKABLE void setEnableRingtone(bool state);
    Q_INVOKABLE void setEnableProxy(bool state);
    Q_INVOKABLE void setKeepAliveEnabled(bool state);
    Q_INVOKABLE void setUseUPnP(bool state);
    Q_INVOKABLE void setUseTURN(bool state);
    Q_INVOKABLE void setUseSTUN(bool state);
    Q_INVOKABLE void setVideoState(bool state);
    Q_INVOKABLE void setUseSRTP(bool state);
    Q_INVOKABLE void setUseSDES(bool state);
    Q_INVOKABLE void setUseRTPFallback(bool state);
    Q_INVOKABLE void setUseTLS(bool state);
    Q_INVOKABLE void setVerifyCertificatesServer(bool state);
    Q_INVOKABLE void setVerifyCertificatesClient(bool state);
    Q_INVOKABLE void setRequireCertificatesIncomingTLS(bool state);
    Q_INVOKABLE void setUseCustomAddressAndPort(bool state);

    Q_INVOKABLE void setNameServer(QString text);
    Q_INVOKABLE void setProxyAddress(QString text);
    Q_INVOKABLE void setBootstrapAddress(QString text);
    Q_INVOKABLE void setTURNAddress(QString text);
    Q_INVOKABLE void setTURNUsername(QString text);
    Q_INVOKABLE void setTURNPassword(QString text);
    Q_INVOKABLE void setTURNRealm(QString text);
    Q_INVOKABLE void setSTUNAddress(QString text);

    Q_INVOKABLE void lineEditVoiceMailDialCodeEditFinished(QString text);
    Q_INVOKABLE void outgoingTLSServerNameLineEditTextChanged(QString text);
    Q_INVOKABLE void lineEditSIPCertPasswordLineEditTextChanged(QString text);
    Q_INVOKABLE void lineEditSIPCustomAddressLineEditTextChanged(QString text);

    Q_INVOKABLE void customPortSIPSpinBoxValueChanged(int value);
    Q_INVOKABLE void negotiationTimeoutSpinBoxValueChanged(int value);
    Q_INVOKABLE void registrationTimeoutSpinBoxValueChanged(int value);
    Q_INVOKABLE void networkInterfaceSpinBoxValueChanged(int value);
    Q_INVOKABLE void audioRTPMinPortSpinBoxEditFinished(int value);
    Q_INVOKABLE void audioRTPMaxPortSpinBoxEditFinished(int value);
    Q_INVOKABLE void videoRTPMinPortSpinBoxEditFinished(int value);
    Q_INVOKABLE void videoRTPMaxPortSpinBoxEditFinished(int value);

    Q_INVOKABLE void tlsProtocolComboBoxIndexChanged(const int& index);

    Q_INVOKABLE void setDeviceName(QString text);

    Q_INVOKABLE void unbanContact(int index);

    Q_INVOKABLE void audioCodecsStateChange(unsigned int id, bool isToEnable);
    Q_INVOKABLE void videoCodecsStateChange(unsigned int id, bool isToEnable);

    Q_INVOKABLE void decreaseAudioCodecPriority(unsigned int id);
    Q_INVOKABLE void increaseAudioCodecPriority(unsigned int id);

    Q_INVOKABLE void decreaseVideoCodecPriority(unsigned int id);
    Q_INVOKABLE void increaseVideoCodecPriority(unsigned int id);

    Q_INVOKABLE void set_RingtonePath(QString text);
    Q_INVOKABLE void set_FileCACert(QString text);
    Q_INVOKABLE void set_FileUserCert(QString text);
    Q_INVOKABLE void set_FilePrivateKey(QString text);
};
Q_DECLARE_METATYPE(SettingsAdapter*)

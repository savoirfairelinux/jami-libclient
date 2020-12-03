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

#include "settingsadapter.h"

#include "api/newdevicemodel.h"

SettingsAdapter::SettingsAdapter(QObject* parent)
    : QObject(parent)
{}

/// Singleton
SettingsAdapter&
SettingsAdapter::instance()
{
    static auto instance = new SettingsAdapter;
    return *instance;
}

QString
SettingsAdapter::getDir_Document()
{
    return QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

QString
SettingsAdapter::getDir_Download()
{
    QString downloadPath = QDir::toNativeSeparators(
        LRCInstance::dataTransferModel().downloadDirectory);
    if (downloadPath.isEmpty()) {
        downloadPath = lrc::api::DataTransferModel::createDefaultDirectory();
        setDownloadPath(downloadPath);
        LRCInstance::dataTransferModel().downloadDirectory = downloadPath;
    }
#ifdef Q_OS_WIN
    int pos = downloadPath.lastIndexOf(QChar('\\'));
#else
    int pos = downloadPath.lastIndexOf(QChar('/'));
#endif
    if (pos == downloadPath.length() - 1)
        downloadPath.truncate(pos);
    return downloadPath;
}

QVariant
SettingsAdapter::getAppValue(const Settings::Key key)
{
    return AppSettingsManager::getValue(key);
}

void
SettingsAdapter::setAppValue(const Settings::Key key, const QVariant& value)
{
    AppSettingsManager::setValue(key, value);
}

void
SettingsAdapter::setRunOnStartUp(bool state)
{
    if (Utils::CheckStartupLink(L"Jami")) {
        if (!state) {
            Utils::DeleteStartupLink(L"Jami");
        }
    } else if (state) {
        Utils::CreateStartupLink(L"Jami");
    }
}

void
SettingsAdapter::setDownloadPath(QString dir)
{
    setAppValue(Settings::Key::DownloadPath, dir);
    LRCInstance::dataTransferModel().downloadDirectory = dir + "/";
}

lrc::api::video::ResRateList
SettingsAdapter::get_ResRateList(lrc::api::video::Channel channel, QString device)
{
    auto deviceCapabilities = get_DeviceCapabilities(device);

    return deviceCapabilities[channel];
}

int
SettingsAdapter::get_DeviceCapabilitiesSize(const QString& device)
{
    return get_DeviceCapabilities(device).size();
}

QVector<QString>
SettingsAdapter::getResolutions(const QString& device)
{
    QVector<QString> resolutions;

    auto currentSettings = LRCInstance::avModel().getDeviceSettings(device);

    auto currentChannel = currentSettings.channel.isEmpty() ? "default" : currentSettings.channel;
    auto channelCaps = get_ResRateList(currentChannel, device);
    for (auto [resolution, frameRateList] : channelCaps) {
        for (auto rate : frameRateList) {
            resolutions.append(resolution);
        }
    }

    return resolutions;
}

QVector<int>
SettingsAdapter::getFrameRates(const QString& device)
{
    QVector<int> rates;

    auto currentSettings = LRCInstance::avModel().getDeviceSettings(device);

    auto currentChannel = currentSettings.channel.isEmpty() ? "default" : currentSettings.channel;
    auto channelCaps = get_ResRateList(currentChannel, device);
    for (auto [resolution, frameRateList] : channelCaps) {
        for (auto rate : frameRateList) {
            rates.append((int) rate);
        }
    }

    return rates;
}

lrc::api::video::Capabilities
SettingsAdapter::get_DeviceCapabilities(const QString& device)
{
    return LRCInstance::avModel().getDeviceCapabilities(device);
}

QString
SettingsAdapter::get_Video_Settings_Channel(const QString& deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.channel;
}

QString
SettingsAdapter::get_Video_Settings_Name(const QString& deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.name;
}

QString
SettingsAdapter::get_Video_Settings_Id(const QString& deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.id;
}

qreal
SettingsAdapter::get_Video_Settings_Rate(const QString& deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (qreal) settings.rate;
}

QString
SettingsAdapter::get_Video_Settings_Size(const QString& deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.size;
}

void
SettingsAdapter::set_Video_Settings_Rate_And_Resolution(const QString& deviceId,
                                                        qreal rate,
                                                        const QString& resolution)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);
    settings.rate = rate;
    settings.size = resolution;
    LRCInstance::avModel().setDeviceSettings(settings);
}

const lrc::api::account::Info&
SettingsAdapter::getCurrentAccountInfo()
{
    return LRCInstance::getCurrentAccountInfo();
}

const Q_INVOKABLE lrc::api::profile::Info&
SettingsAdapter::getCurrentAccount_Profile_Info()
{
    return LRCInstance::getCurrentAccountInfo().profileInfo;
}

lrc::api::ContactModel*
SettingsAdapter::getContactModel()
{
    return getCurrentAccountInfo().contactModel.get();
}

lrc::api::NewDeviceModel*
SettingsAdapter::getDeviceModel()
{
    return getCurrentAccountInfo().deviceModel.get();
}

QString
SettingsAdapter::get_CurrentAccountInfo_RegisteredName()
{
    return LRCInstance::getCurrentAccountInfo().registeredName;
}

QString
SettingsAdapter::get_CurrentAccountInfo_Id()
{
    return LRCInstance::getCurrentAccountInfo().id;
}

bool
SettingsAdapter::get_CurrentAccountInfo_Enabled()
{
    return LRCInstance::getCurrentAccountInfo().enabled;
}

QString
SettingsAdapter::getCurrentAccount_Profile_Info_Uri()
{
    return getCurrentAccount_Profile_Info().uri;
}

QString
SettingsAdapter::getCurrentAccount_Profile_Info_Alias()
{
    return getCurrentAccount_Profile_Info().alias;
}

int
SettingsAdapter::getCurrentAccount_Profile_Info_Type()
{
    return (int) (getCurrentAccount_Profile_Info().type);
}

QString
SettingsAdapter::getAccountBestName()
{
    return LRCInstance::accountModel().bestNameForAccount(LRCInstance::getCurrAccId());
}

lrc::api::account::ConfProperties_t
SettingsAdapter::getAccountConfig()
{
    lrc::api::account::ConfProperties_t res;
    try {
        res = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    } catch (...) {
    }
    return res;
}

QString
SettingsAdapter::getAccountConfig_Manageruri()
{
    return getAccountConfig().managerUri;
}

QString
SettingsAdapter::getAccountConfig_Username()
{
    return getAccountConfig().username;
}

QString
SettingsAdapter::getAccountConfig_Hostname()
{
    return getAccountConfig().hostname;
}

QString
SettingsAdapter::getAccountConfig_Password()
{
    return getAccountConfig().password;
}

bool
SettingsAdapter::getAccountConfig_KeepAliveEnabled()
{
    return getAccountConfig().keepAliveEnabled;
}

QString
SettingsAdapter::getAccountConfig_RouteSet()
{
    return getAccountConfig().routeset;
}

QString
SettingsAdapter::getAccountConfig_ProxyServer()
{
    return getAccountConfig().proxyServer;
}

bool
SettingsAdapter::getAccountConfig_PeerDiscovery()
{
    return getAccountConfig().peerDiscovery;
}

bool
SettingsAdapter::getAccountConfig_DHT_PublicInCalls()
{
    return getAccountConfig().DHT.PublicInCalls;
}

bool
SettingsAdapter::getAccountConfig_RendezVous()
{
    return getAccountConfig().isRendezVous;
}

bool
SettingsAdapter::getAccountConfig_AutoAnswer()
{
    return getAccountConfig().autoAnswer;
}

QString
SettingsAdapter::getAccountConfig_RingNS_Uri()
{
    return getAccountConfig().RingNS.uri;
}

bool
SettingsAdapter::getAccountConfig_ProxyEnabled()
{
    return getAccountConfig().proxyEnabled;
}

QString
SettingsAdapter::getAccountConfig_TLS_CertificateListFile()
{
    return getAccountConfig().TLS.certificateListFile;
}

QString
SettingsAdapter::getAccountConfig_TLS_CertificateFile()
{
    return getAccountConfig().TLS.certificateFile;
}

QString
SettingsAdapter::getAccountConfig_TLS_PrivateKeyFile()
{
    return getAccountConfig().TLS.privateKeyFile;
}

bool
SettingsAdapter::getAccountConfig_TLS_Enable()
{
    return getAccountConfig().TLS.enable;
}

QString
SettingsAdapter::getAccountConfig_TLS_Password()
{
    return getAccountConfig().TLS.password;
}

bool
SettingsAdapter::getAccountConfig_TLS_VerifyServer()
{
    return getAccountConfig().TLS.verifyServer;
}

bool
SettingsAdapter::getAccountConfig_TLS_VerifyClient()
{
    return getAccountConfig().TLS.verifyClient;
}

bool
SettingsAdapter::getAccountConfig_TLS_RequireClientCertificate()
{
    return getAccountConfig().TLS.requireClientCertificate;
}

int
SettingsAdapter::getAccountConfig_TLS_Method_inInt()
{
    return (int) getAccountConfig().TLS.method;
}

QString
SettingsAdapter::getAccountConfig_TLS_Servername()
{
    return getAccountConfig().TLS.serverName;
}

int
SettingsAdapter::getAccountConfig_TLS_NegotiationTimeoutSec()
{
    return getAccountConfig().TLS.negotiationTimeoutSec;
}

bool
SettingsAdapter::getAccountConfig_SRTP_Enabled()
{
    return getAccountConfig().SRTP.enable;
}

int
SettingsAdapter::getAccountConfig_SRTP_KeyExchange()
{
    return (int) getAccountConfig().SRTP.keyExchange;
}

bool
SettingsAdapter::getAccountConfig_SRTP_RtpFallback()
{
    return getAccountConfig().SRTP.rtpFallback;
}

bool
SettingsAdapter::getAccountConfig_UpnpEnabled()
{
    return getAccountConfig().upnpEnabled;
}

bool
SettingsAdapter::getAccountConfig_TURN_Enabled()
{
    return getAccountConfig().TURN.enable;
}

QString
SettingsAdapter::getAccountConfig_TURN_Server()
{
    return getAccountConfig().TURN.server;
}

QString
SettingsAdapter::getAccountConfig_TURN_Username()
{
    return getAccountConfig().TURN.username;
}

QString
SettingsAdapter::getAccountConfig_TURN_Password()
{
    return getAccountConfig().TURN.password;
}

QString
SettingsAdapter::getAccountConfig_TURN_Realm()
{
    return getAccountConfig().TURN.realm;
}

bool
SettingsAdapter::getAccountConfig_STUN_Enabled()
{
    return getAccountConfig().STUN.enable;
}

QString
SettingsAdapter::getAccountConfig_STUN_Server()
{
    return getAccountConfig().STUN.server;
}

bool
SettingsAdapter::getAccountConfig_Video_Enabled()
{
    return getAccountConfig().Video.videoEnabled;
}

int
SettingsAdapter::getAccountConfig_Video_VideoPortMin()
{
    return getAccountConfig().Video.videoPortMin;
}

int
SettingsAdapter::getAccountConfig_Video_VideoPortMax()
{
    return getAccountConfig().Video.videoPortMax;
}

int
SettingsAdapter::getAccountConfig_Audio_AudioPortMin()
{
    return getAccountConfig().Audio.audioPortMin;
}

int
SettingsAdapter::getAccountConfig_Audio_AudioPortMax()
{
    return getAccountConfig().Audio.audioPortMax;
}

bool
SettingsAdapter::getAccountConfig_Ringtone_RingtoneEnabled()
{
    return getAccountConfig().Ringtone.ringtoneEnabled;
}

QString
SettingsAdapter::getAccountConfig_Ringtone_RingtonePath()
{
    return getAccountConfig().Ringtone.ringtonePath;
}

int
SettingsAdapter::getAccountConfig_Registration_Expire()
{
    return getAccountConfig().Registration.expire;
}

int
SettingsAdapter::getAccountConfig_Localport()
{
    return getAccountConfig().localPort;
}

bool
SettingsAdapter::getAccountConfig_PublishedSameAsLocal()
{
    return getAccountConfig().publishedSameAsLocal;
}

QString
SettingsAdapter::getAccountConfig_PublishedAddress()
{
    return getAccountConfig().publishedAddress;
}

int
SettingsAdapter::getAccountConfig_PublishedPort()
{
    return getAccountConfig().publishedPort;
}

QString
SettingsAdapter::getAccountConfig_Mailbox()
{
    return getAccountConfig().mailbox;
}

void
SettingsAdapter::setAccountConfig_Username(QString input)
{
    auto confProps = getAccountConfig();
    confProps.username = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setAccountConfig_Hostname(QString input)
{
    auto confProps = getAccountConfig();
    confProps.hostname = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setAccountConfig_Password(QString input)
{
    auto confProps = getAccountConfig();
    confProps.password = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setAccountConfig_RouteSet(QString input)
{
    auto confProps = getAccountConfig();
    confProps.routeset = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setAutoConnectOnLocalNetwork(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.peerDiscovery = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setCallsUntrusted(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.DHT.PublicInCalls = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setIsRendezVous(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.isRendezVous = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setAutoAnswerCalls(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.autoAnswer = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setEnableRingtone(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Ringtone.ringtoneEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setEnableProxy(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.proxyEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setKeepAliveEnabled(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.keepAliveEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseUPnP(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.upnpEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseTURN(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseSTUN(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.STUN.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setVideoState(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseSRTP(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseSDES(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.keyExchange = state ? lrc::api::account::KeyExchangeProtocol::SDES
                                       : lrc::api::account::KeyExchangeProtocol::NONE;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseRTPFallback(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.rtpFallback = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseTLS(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setVerifyCertificatesServer(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.verifyServer = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setVerifyCertificatesClient(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.verifyClient = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setRequireCertificatesIncomingTLS(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.requireClientCertificate = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setUseCustomAddressAndPort(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedSameAsLocal = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setNameServer(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.RingNS.uri = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setProxyAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.proxyServer = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setBootstrapAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.hostname = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setTURNAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.server = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setTURNUsername(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.username = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setTURNPassword(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.password = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setTURNRealm(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.realm = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::setSTUNAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.STUN.server = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::lineEditVoiceMailDialCodeEditFinished(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.mailbox = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::outgoingTLSServerNameLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.serverName = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::lineEditSIPCertPasswordLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.password = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::lineEditSIPCustomAddressLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedAddress = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::customPortSIPSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedPort = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::negotiationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.negotiationTimeoutSec = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::registrationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Registration.expire = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::networkInterfaceSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.localPort = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::audioRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Audio.audioPortMin = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::audioRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Audio.audioPortMax = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::videoRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoPortMin = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::videoRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoPortMax = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::tlsProtocolComboBoxIndexChanged(const int& index)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());

    if (static_cast<int>(confProps.TLS.method) != index) {
        if (index == 0) {
            confProps.TLS.method = lrc::api::account::TlsMethod::DEFAULT;
        } else if (index == 1) {
            confProps.TLS.method = lrc::api::account::TlsMethod::TLSv1;
        } else if (index == 2) {
            confProps.TLS.method = lrc::api::account::TlsMethod::TLSv1_1;
        } else {
            confProps.TLS.method = lrc::api::account::TlsMethod::TLSv1_2;
        }
        LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
    }
}

void
SettingsAdapter::setDeviceName(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.deviceName = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::unbanContact(int index)
{
    auto& accountInfo = LRCInstance::getCurrentAccountInfo();
    auto bannedContactList = accountInfo.contactModel->getBannedContacts();
    auto it = bannedContactList.begin();
    std::advance(it, index);

    auto contactInfo = accountInfo.contactModel->getContact(*it);
    accountInfo.contactModel->addContact(contactInfo);
}

void
SettingsAdapter::audioCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto audioCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getAudioCodecs();
    LRCInstance::getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdapter::videoCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto videoCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getVideoCodecs();
    LRCInstance::getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdapter::decreaseAudioCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->decreasePriority(id, false);
}

void
SettingsAdapter::increaseAudioCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->increasePriority(id, false);
}

void
SettingsAdapter::decreaseVideoCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->decreasePriority(id, true);
}

void
SettingsAdapter::increaseVideoCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->increasePriority(id, true);
}

void
SettingsAdapter::set_RingtonePath(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Ringtone.ringtonePath = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::set_FileCACert(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.certificateListFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::set_FileUserCert(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.certificateFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdapter::set_FilePrivateKey(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.privateKeyFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

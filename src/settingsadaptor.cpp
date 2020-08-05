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

#include "settingsadaptor.h"

#include "api/newdevicemodel.h"

SettingsAdaptor::SettingsAdaptor(QObject *parent)
    : QObject(parent)
{}

///Singleton
SettingsAdaptor &
SettingsAdaptor::instance()
{
    static auto instance = new SettingsAdaptor;
    return *instance;
}

QString
SettingsAdaptor::getDir_Document()
{
    return QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

QString
SettingsAdaptor::getDir_Download()
{
    QString downloadPath = QDir::toNativeSeparators(LRCInstance::dataTransferModel().downloadDirectory);
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

bool
SettingsAdaptor::getSettingsValue_CloseOrMinimized()
{
    QSettings settings("jami.net", "Jami");
    return settings.value(SettingsKey::closeOrMinimized).toBool();
}

bool
SettingsAdaptor::getSettingsValue_EnableNotifications()
{
    QSettings settings("jami.net", "Jami");
    return settings.value(SettingsKey::enableNotifications).toBool();
}

bool
SettingsAdaptor::getSettingsValue_AutoUpdate()
{
    QSettings settings("jami.net", "Jami");
    return settings.value(SettingsKey::autoUpdate).toBool();
}

void
SettingsAdaptor::setClosedOrMin(bool state)
{
    QSettings settings("jami.net", "Jami");
    settings.setValue(SettingsKey::closeOrMinimized, state);
}

void
SettingsAdaptor::setNotifications(bool state)
{
    QSettings settings("jami.net", "Jami");
    settings.setValue(SettingsKey::enableNotifications, state);
}

void
SettingsAdaptor::setUpdateAutomatic(bool state)
{
#ifdef Q_OS_WIN
    QSettings settings("jami.net", "Jami");
    settings.setValue(SettingsKey::autoUpdate, state);
#endif
}

void
SettingsAdaptor::setRunOnStartUp(bool state)
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
SettingsAdaptor::setDownloadPath(QString dir)
{
    QSettings settings("jami.net", "Jami");
    settings.setValue(SettingsKey::downloadPath, dir);
    LRCInstance::dataTransferModel().downloadDirectory = dir + "/";
}

lrc::api::video::ResRateList
SettingsAdaptor::get_ResRateList(lrc::api::video::Channel channel, QString device)
{
    auto deviceCapabilities = get_DeviceCapabilities(device);

    return deviceCapabilities[channel];
}

int
SettingsAdaptor::get_DeviceCapabilitiesSize(const QString &device)
{
    return get_DeviceCapabilities(device).size();
}

QVector<QString>
SettingsAdaptor::getResolutions(const QString &device)
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
SettingsAdaptor::getFrameRates(const QString &device)
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
SettingsAdaptor::get_DeviceCapabilities(const QString &device)
{
    return LRCInstance::avModel().getDeviceCapabilities(device);
}

QString
SettingsAdaptor::get_Video_Settings_Channel(const QString &deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.channel;
}

QString
SettingsAdaptor::get_Video_Settings_Name(const QString &deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.name;
}

QString
SettingsAdaptor::get_Video_Settings_Id(const QString &deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.id;
}

qreal
SettingsAdaptor::get_Video_Settings_Rate(const QString &deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (qreal) settings.rate;
}

QString
SettingsAdaptor::get_Video_Settings_Size(const QString &deviceId)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);

    return (QString) settings.size;
}

void
SettingsAdaptor::set_Video_Settings_Rate_And_Resolution(const QString &deviceId,
                                                        qreal rate,
                                                        const QString &resolution)
{
    auto settings = LRCInstance::avModel().getDeviceSettings(deviceId);
    settings.rate = rate;
    settings.size = resolution;
    LRCInstance::avModel().setDeviceSettings(settings);
}

const lrc::api::account::Info &
SettingsAdaptor::getCurrentAccountInfo()
{
    return LRCInstance::getCurrentAccountInfo();
}

const Q_INVOKABLE lrc::api::profile::Info &
SettingsAdaptor::getCurrentAccount_Profile_Info()
{
    return LRCInstance::getCurrentAccountInfo().profileInfo;
}

lrc::api::ContactModel *
SettingsAdaptor::getContactModel()
{
    return getCurrentAccountInfo().contactModel.get();
}

lrc::api::NewDeviceModel *
SettingsAdaptor::getDeviceModel()
{
    return getCurrentAccountInfo().deviceModel.get();
}

QString
SettingsAdaptor::get_CurrentAccountInfo_RegisteredName()
{
    return LRCInstance::getCurrentAccountInfo().registeredName;
}

QString
SettingsAdaptor::get_CurrentAccountInfo_Id()
{
    return LRCInstance::getCurrentAccountInfo().id;
}

bool
SettingsAdaptor::get_CurrentAccountInfo_Enabled()
{
    return LRCInstance::getCurrentAccountInfo().enabled;
}

QString
SettingsAdaptor::getCurrentAccount_Profile_Info_Uri()
{
    return getCurrentAccount_Profile_Info().uri;
}

QString
SettingsAdaptor::getCurrentAccount_Profile_Info_Alias()
{
    return getCurrentAccount_Profile_Info().alias;
}

int
SettingsAdaptor::getCurrentAccount_Profile_Info_Type()
{
    return (int) (getCurrentAccount_Profile_Info().type);
}

QString
SettingsAdaptor::getAccountBestName()
{
    return Utils::bestNameForAccount(LRCInstance::getCurrentAccountInfo());
}

QString
SettingsAdaptor::getAvatarImage_Base64(int avatarSize)
{
    auto &accountInfo = LRCInstance::getCurrentAccountInfo();
    auto avatar = Utils::accountPhoto(accountInfo, {avatarSize, avatarSize});

    return QString::fromLatin1(Utils::QImageToByteArray(avatar).toBase64().data());
}

bool
SettingsAdaptor::getIsDefaultAvatar()
{
    auto &accountInfo = LRCInstance::getCurrentAccountInfo();

    return accountInfo.profileInfo.avatar.isEmpty();
}

bool
SettingsAdaptor::setCurrAccAvatar(QString avatarImgBase64)
{
    QImage avatarImg;
    const bool ret = avatarImg.loadFromData(QByteArray::fromBase64(avatarImgBase64.toLatin1()));
    if (!ret) {
        qDebug() << "Current avatar loading from base64 fail";
        return false;
    } else {
        LRCInstance::setCurrAccAvatar(QPixmap::fromImage(avatarImg));
    }
    return true;
}

void
SettingsAdaptor::clearCurrentAvatar()
{
    LRCInstance::setCurrAccAvatar(QPixmap());
}

lrc::api::account::ConfProperties_t
SettingsAdaptor::getAccountConfig()
{
    lrc::api::account::ConfProperties_t res;
    try {
        res = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    } catch (...) {}
    return res;
}

QString
SettingsAdaptor::getAccountConfig_Manageruri()
{
    return getAccountConfig().managerUri;
}

QString
SettingsAdaptor::getAccountConfig_Username()
{
    return getAccountConfig().username;
}

QString
SettingsAdaptor::getAccountConfig_Hostname()
{
    return getAccountConfig().hostname;
}

QString
SettingsAdaptor::getAccountConfig_Password()
{
    return getAccountConfig().password;
}

QString
SettingsAdaptor::getAccountConfig_ProxyServer()
{
    return getAccountConfig().proxyServer;
}

bool
SettingsAdaptor::getAccountConfig_PeerDiscovery()
{
    return getAccountConfig().peerDiscovery;
}

bool
SettingsAdaptor::getAccountConfig_DHT_PublicInCalls()
{
    return getAccountConfig().DHT.PublicInCalls;
}

bool
SettingsAdaptor::getAccountConfig_RendezVous()
{
    return getAccountConfig().isRendezVous;
}

bool
SettingsAdaptor::getAccountConfig_AutoAnswer()
{
    return getAccountConfig().autoAnswer;
}

QString
SettingsAdaptor::getAccountConfig_RingNS_Uri()
{
    return getAccountConfig().RingNS.uri;
}

bool
SettingsAdaptor::getAccountConfig_ProxyEnabled()
{
    return getAccountConfig().proxyEnabled;
}

QString
SettingsAdaptor::getAccountConfig_TLS_CertificateListFile()
{
    return getAccountConfig().TLS.certificateListFile;
}

QString
SettingsAdaptor::getAccountConfig_TLS_CertificateFile()
{
    return getAccountConfig().TLS.certificateFile;
}

QString
SettingsAdaptor::getAccountConfig_TLS_PrivateKeyFile()
{
    return getAccountConfig().TLS.privateKeyFile;
}

bool
SettingsAdaptor::getAccountConfig_TLS_Enable()
{
    return getAccountConfig().TLS.enable;
}

QString
SettingsAdaptor::getAccountConfig_TLS_Password()
{
    return getAccountConfig().TLS.password;
}

bool
SettingsAdaptor::getAccountConfig_TLS_VerifyServer()
{
    return getAccountConfig().TLS.verifyServer;
}

bool
SettingsAdaptor::getAccountConfig_TLS_VerifyClient()
{
    return getAccountConfig().TLS.verifyClient;
}

bool
SettingsAdaptor::getAccountConfig_TLS_RequireClientCertificate()
{
    return getAccountConfig().TLS.requireClientCertificate;
}

int
SettingsAdaptor::getAccountConfig_TLS_Method_inInt()
{
    return (int) getAccountConfig().TLS.method;
}

QString
SettingsAdaptor::getAccountConfig_TLS_Servername()
{
    return getAccountConfig().TLS.serverName;
}

int
SettingsAdaptor::getAccountConfig_TLS_NegotiationTimeoutSec()
{
    return getAccountConfig().TLS.negotiationTimeoutSec;
}

bool
SettingsAdaptor::getAccountConfig_SRTP_Enabled()
{
    return getAccountConfig().SRTP.enable;
}

int
SettingsAdaptor::getAccountConfig_SRTP_KeyExchange()
{
    return (int) getAccountConfig().SRTP.keyExchange;
}

bool
SettingsAdaptor::getAccountConfig_SRTP_RtpFallback()
{
    return getAccountConfig().SRTP.rtpFallback;
}

bool
SettingsAdaptor::getAccountConfig_UpnpEnabled()
{
    return getAccountConfig().upnpEnabled;
}

bool
SettingsAdaptor::getAccountConfig_TURN_Enabled()
{
    return getAccountConfig().TURN.enable;
}

QString
SettingsAdaptor::getAccountConfig_TURN_Server()
{
    return getAccountConfig().TURN.server;
}

QString
SettingsAdaptor::getAccountConfig_TURN_Username()
{
    return getAccountConfig().TURN.username;
}

QString
SettingsAdaptor::getAccountConfig_TURN_Password()
{
    return getAccountConfig().TURN.password;
}

QString
SettingsAdaptor::getAccountConfig_TURN_Realm()
{
    return getAccountConfig().TURN.realm;
}

bool
SettingsAdaptor::getAccountConfig_STUN_Enabled()
{
    return getAccountConfig().STUN.enable;
}

QString
SettingsAdaptor::getAccountConfig_STUN_Server()
{
    return getAccountConfig().STUN.server;
}

bool
SettingsAdaptor::getAccountConfig_Video_Enabled()
{
    return getAccountConfig().Video.videoEnabled;
}

int
SettingsAdaptor::getAccountConfig_Video_VideoPortMin()
{
    return getAccountConfig().Video.videoPortMin;
}

int
SettingsAdaptor::getAccountConfig_Video_VideoPortMax()
{
    return getAccountConfig().Video.videoPortMax;
}

int
SettingsAdaptor::getAccountConfig_Audio_AudioPortMin()
{
    return getAccountConfig().Audio.audioPortMin;
}

int
SettingsAdaptor::getAccountConfig_Audio_AudioPortMax()
{
    return getAccountConfig().Audio.audioPortMax;
}

bool
SettingsAdaptor::getAccountConfig_Ringtone_RingtoneEnabled()
{
    return getAccountConfig().Ringtone.ringtoneEnabled;
}

QString
SettingsAdaptor::getAccountConfig_Ringtone_RingtonePath()
{
    return getAccountConfig().Ringtone.ringtonePath;
}

int
SettingsAdaptor::getAccountConfig_Registration_Expire()
{
    return getAccountConfig().Registration.expire;
}

int
SettingsAdaptor::getAccountConfig_Localport()
{
    return getAccountConfig().localPort;
}

bool
SettingsAdaptor::getAccountConfig_PublishedSameAsLocal()
{
    return getAccountConfig().publishedSameAsLocal;
}

QString
SettingsAdaptor::getAccountConfig_PublishedAddress()
{
    return getAccountConfig().publishedAddress;
}

int
SettingsAdaptor::getAccountConfig_PublishedPort()
{
    return getAccountConfig().publishedPort;
}

QString
SettingsAdaptor::getAccountConfig_Mailbox()
{
    return getAccountConfig().mailbox;
}

void
SettingsAdaptor::setAccountConfig_Username(QString input)
{
    auto confProps = getAccountConfig();
    confProps.username = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setAccountConfig_Hostname(QString input)
{
    auto confProps = getAccountConfig();
    confProps.hostname = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setAccountConfig_Password(QString input)
{
    auto confProps = getAccountConfig();
    confProps.password = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setAccountConfig_ProxyServer(QString input)
{
    auto confProps = getAccountConfig();
    confProps.proxyServer = input;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setAutoConnectOnLocalNetwork(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.peerDiscovery = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setCallsUntrusted(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.DHT.PublicInCalls = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setIsRendezVous(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.isRendezVous = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}


void
SettingsAdaptor::setAutoAnswerCalls(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.autoAnswer = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setEnableRingtone(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Ringtone.ringtoneEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setEnableProxy(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.proxyEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseUPnP(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.upnpEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseTURN(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseSTUN(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.STUN.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setVideoState(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoEnabled = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseSRTP(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseSDES(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.keyExchange = state ? lrc::api::account::KeyExchangeProtocol::SDES
                                       : lrc::api::account::KeyExchangeProtocol::NONE;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseRTPFallback(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.SRTP.rtpFallback = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseTLS(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.enable = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setVerifyCertificatesServer(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.verifyServer = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setVerifyCertificatesClient(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.verifyClient = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setRequireCertificatesIncomingTLS(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.requireClientCertificate = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setUseCustomAddressAndPort(bool state)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedSameAsLocal = state;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setNameServer(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.RingNS.uri = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setProxyAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.proxyServer = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setBootstrapAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.hostname = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setTURNAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.server = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setTURNUsername(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.username = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setTURNPassword(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.password = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setTURNRealm(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TURN.realm = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::setSTUNAddress(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.STUN.server = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::lineEditVoiceMailDialCodeEditFinished(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.mailbox = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::outgoingTLSServerNameLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.serverName = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::lineEditSIPCertPasswordLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.password = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::lineEditSIPCustomAddressLineEditTextChanged(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedAddress = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::customPortSIPSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.publishedPort = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::negotiationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.negotiationTimeoutSec = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::registrationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Registration.expire = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::networkInterfaceSpinBoxValueChanged(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.localPort = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::audioRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Audio.audioPortMin = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::audioRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Audio.audioPortMax = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::videoRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoPortMin = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::videoRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Video.videoPortMax = value;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::tlsProtocolComboBoxIndexChanged(const int &index)
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
SettingsAdaptor::setDeviceName(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.deviceName = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::unbanContact(int index)
{
    auto bannedContactList = LRCInstance::getCurrentAccountInfo().contactModel->getBannedContacts();
    auto it = bannedContactList.begin();
    std::advance(it, index);

    auto contactInfo = LRCInstance::getCurrentAccountInfo().contactModel->getContact(*it);

    LRCInstance::getCurrentAccountInfo().contactModel->addContact(contactInfo);
}

void
SettingsAdaptor::audioCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto audioCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getAudioCodecs();
    LRCInstance::getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdaptor::videoCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto videoCodecList = LRCInstance::getCurrentAccountInfo().codecModel->getVideoCodecs();
    LRCInstance::getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdaptor::decreaseAudioCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->decreasePriority(id, false);
}

void
SettingsAdaptor::increaseAudioCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->increasePriority(id, false);
}

void
SettingsAdaptor::decreaseVideoCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->decreasePriority(id, true);
}

void
SettingsAdaptor::increaseVideoCodecPriority(unsigned int id)
{
    LRCInstance::getCurrentAccountInfo().codecModel->increasePriority(id, true);
}

void
SettingsAdaptor::set_RingtonePath(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.Ringtone.ringtonePath = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::set_FileCACert(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.certificateListFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::set_FileUserCert(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.certificateFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

void
SettingsAdaptor::set_FilePrivateKey(QString text)
{
    auto confProps = LRCInstance::accountModel().getAccountConfig(LRCInstance::getCurrAccId());
    confProps.TLS.privateKeyFile = text;
    LRCInstance::accountModel().setAccountConfig(LRCInstance::getCurrAccId(), confProps);
}

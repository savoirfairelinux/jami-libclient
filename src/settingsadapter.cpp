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

#include "api/newcodecmodel.h"
#include "api/newdevicemodel.h"

SettingsAdapter::SettingsAdapter(AppSettingsManager* settingsManager,
                                 LRCInstance* instance,
                                 QObject* parent)
    : QmlAdapterBase(instance, parent)
    , settingsManager_(settingsManager)
{}

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
        lrcInstance_->dataTransferModel().downloadDirectory);
    if (downloadPath.isEmpty()) {
        downloadPath = lrc::api::DataTransferModel::createDefaultDirectory();
        setDownloadPath(downloadPath);
        lrcInstance_->dataTransferModel().downloadDirectory = downloadPath;
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
    return settingsManager_->getValue(key);
}

void
SettingsAdapter::setAppValue(const Settings::Key key, const QVariant& value)
{
    settingsManager_->setValue(key, value);
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
    lrcInstance_->dataTransferModel().downloadDirectory = dir + "/";
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

    auto currentSettings = lrcInstance_->avModel().getDeviceSettings(device);

    auto currentChannel = currentSettings.channel.isEmpty() ? "default" : currentSettings.channel;
    auto channelCaps = get_ResRateList(currentChannel, device);
    for (auto [resolution, frameRateList] : channelCaps) {
        for (auto rate : frameRateList) {
            (void) rate;
            resolutions.append(resolution);
        }
    }

    return resolutions;
}

QVector<int>
SettingsAdapter::getFrameRates(const QString& device)
{
    QVector<int> rates;

    auto currentSettings = lrcInstance_->avModel().getDeviceSettings(device);

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
    return lrcInstance_->avModel().getDeviceCapabilities(device);
}

QString
SettingsAdapter::get_Video_Settings_Channel(const QString& deviceId)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);

    return (QString) settings.channel;
}

QString
SettingsAdapter::get_Video_Settings_Name(const QString& deviceId)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);

    return (QString) settings.name;
}

QString
SettingsAdapter::get_Video_Settings_Id(const QString& deviceId)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);

    return (QString) settings.id;
}

qreal
SettingsAdapter::get_Video_Settings_Rate(const QString& deviceId)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);

    return (qreal) settings.rate;
}

QString
SettingsAdapter::get_Video_Settings_Size(const QString& deviceId)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);

    return (QString) settings.size;
}

void
SettingsAdapter::set_Video_Settings_Rate_And_Resolution(const QString& deviceId,
                                                        qreal rate,
                                                        const QString& resolution)
{
    auto settings = lrcInstance_->avModel().getDeviceSettings(deviceId);
    settings.rate = rate;
    settings.size = resolution;
    lrcInstance_->avModel().setDeviceSettings(settings);
}

const lrc::api::account::Info&
SettingsAdapter::getCurrentAccountInfo()
{
    return lrcInstance_->getCurrentAccountInfo();
}

const Q_INVOKABLE lrc::api::profile::Info&
SettingsAdapter::getCurrentAccount_Profile_Info()
{
    return lrcInstance_->getCurrentAccountInfo().profileInfo;
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
    return lrcInstance_->getCurrentAccountInfo().registeredName;
}

QString
SettingsAdapter::get_CurrentAccountInfo_Id()
{
    return lrcInstance_->getCurrentAccountInfo().id;
}

bool
SettingsAdapter::get_CurrentAccountInfo_Enabled()
{
    return lrcInstance_->getCurrentAccountInfo().enabled;
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
    return lrcInstance_->accountModel().bestNameForAccount(lrcInstance_->getCurrentAccountId());
}

lrc::api::account::ConfProperties_t
SettingsAdapter::getAccountConfig()
{
    lrc::api::account::ConfProperties_t res;
    try {
        res = lrcInstance_->accountModel().getAccountConfig(lrcInstance_->getCurrentAccountId());
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
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setAccountConfig_Hostname(QString input)
{
    auto confProps = getAccountConfig();
    confProps.hostname = input;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setAccountConfig_Password(QString input)
{
    auto confProps = getAccountConfig();
    confProps.password = input;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setAccountConfig_RouteSet(QString input)
{
    auto confProps = getAccountConfig();
    confProps.routeset = input;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setAutoConnectOnLocalNetwork(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.peerDiscovery = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setCallsUntrusted(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.DHT.PublicInCalls = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setIsRendezVous(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.isRendezVous = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setAutoAnswerCalls(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.autoAnswer = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setEnableRingtone(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Ringtone.ringtoneEnabled = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setEnableProxy(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.proxyEnabled = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setKeepAliveEnabled(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.keepAliveEnabled = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseUPnP(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.upnpEnabled = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseTURN(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TURN.enable = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseSTUN(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.STUN.enable = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setVideoState(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Video.videoEnabled = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseSRTP(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.SRTP.enable = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseSDES(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.SRTP.keyExchange = state ? lrc::api::account::KeyExchangeProtocol::SDES
                                       : lrc::api::account::KeyExchangeProtocol::NONE;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseRTPFallback(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.SRTP.rtpFallback = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseTLS(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.enable = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setVerifyCertificatesServer(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.verifyServer = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setVerifyCertificatesClient(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.verifyClient = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setRequireCertificatesIncomingTLS(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.requireClientCertificate = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setUseCustomAddressAndPort(bool state)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.publishedSameAsLocal = state;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setNameServer(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.RingNS.uri = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setProxyAddress(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.proxyServer = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setBootstrapAddress(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.hostname = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setTURNAddress(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TURN.server = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setTURNUsername(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TURN.username = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setTURNPassword(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TURN.password = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setTURNRealm(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TURN.realm = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setSTUNAddress(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.STUN.server = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::lineEditVoiceMailDialCodeEditFinished(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.mailbox = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::outgoingTLSServerNameLineEditTextChanged(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.serverName = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::lineEditSIPCertPasswordLineEditTextChanged(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.password = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::lineEditSIPCustomAddressLineEditTextChanged(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.publishedAddress = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::customPortSIPSpinBoxValueChanged(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.publishedPort = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::negotiationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.negotiationTimeoutSec = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::registrationTimeoutSpinBoxValueChanged(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Registration.expire = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::networkInterfaceSpinBoxValueChanged(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.localPort = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::audioRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Audio.audioPortMin = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::audioRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Audio.audioPortMax = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::videoRTPMinPortSpinBoxEditFinished(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Video.videoPortMin = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::videoRTPMaxPortSpinBoxEditFinished(int value)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Video.videoPortMax = value;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::tlsProtocolComboBoxIndexChanged(const int& index)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());

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
        lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(),
                                                      confProps);
    }
}

void
SettingsAdapter::setDeviceName(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.deviceName = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::unbanContact(int index)
{
    auto& accountInfo = lrcInstance_->getCurrentAccountInfo();
    auto bannedContactList = accountInfo.contactModel->getBannedContacts();
    auto it = bannedContactList.begin();
    std::advance(it, index);

    try {
        auto contactInfo = accountInfo.contactModel->getContact(*it);
        accountInfo.contactModel->addContact(contactInfo);
    } catch (const std::out_of_range& e) {
        qDebug() << e.what();
    }
}

void
SettingsAdapter::audioCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto audioCodecList = lrcInstance_->getCurrentAccountInfo().codecModel->getAudioCodecs();
    lrcInstance_->getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdapter::videoCodecsStateChange(unsigned int id, bool isToEnable)
{
    auto videoCodecList = lrcInstance_->getCurrentAccountInfo().codecModel->getVideoCodecs();
    lrcInstance_->getCurrentAccountInfo().codecModel->enable(id, isToEnable);
}

void
SettingsAdapter::decreaseAudioCodecPriority(unsigned int id)
{
    lrcInstance_->getCurrentAccountInfo().codecModel->decreasePriority(id, false);
}

void
SettingsAdapter::increaseAudioCodecPriority(unsigned int id)
{
    lrcInstance_->getCurrentAccountInfo().codecModel->increasePriority(id, false);
}

void
SettingsAdapter::decreaseVideoCodecPriority(unsigned int id)
{
    lrcInstance_->getCurrentAccountInfo().codecModel->decreasePriority(id, true);
}

void
SettingsAdapter::increaseVideoCodecPriority(unsigned int id)
{
    lrcInstance_->getCurrentAccountInfo().codecModel->increasePriority(id, true);
}

void
SettingsAdapter::set_RingtonePath(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.Ringtone.ringtonePath = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::set_FileCACert(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.certificateListFile = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::set_FileUserCert(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.certificateFile = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::set_FilePrivateKey(QString text)
{
    auto confProps = lrcInstance_->accountModel().getAccountConfig(
        lrcInstance_->getCurrentAccountId());
    confProps.TLS.privateKeyFile = text;
    lrcInstance_->accountModel().setAccountConfig(lrcInstance_->getCurrentAccountId(), confProps);
}

void
SettingsAdapter::setDefaultModerator(const QString& accountId,
                                     const QString& peerURI,
                                     const bool& state)
{
    lrcInstance_->accountModel().setDefaultModerator(accountId, peerURI, state);
}

void
SettingsAdapter::setAllModeratorsEnabled(const QString& accountId, bool enabled)
{
    lrcInstance_->accountModel().setAllModerators(accountId, enabled);
}

QStringList
SettingsAdapter::getDefaultModerators(const QString& accountId)
{
    return lrcInstance_->accountModel().getDefaultModerators(accountId);
}

void
SettingsAdapter::enableLocalModerators(const QString& accountId, const bool& isModEnabled)
{
    lrcInstance_->accountModel().enableLocalModerators(accountId, isModEnabled);
}

bool
SettingsAdapter::isLocalModeratorsEnabled(const QString& accountId)
{
    return lrcInstance_->accountModel().isLocalModeratorsEnabled(accountId);
}

bool
SettingsAdapter::isAllModeratorsEnabled(const QString& accountId)
{
    return lrcInstance_->accountModel().isAllModerators(accountId);
}

/******************************************************************************
 *    Copyright (C) 2014-2021 Savoir-faire Linux Inc.                               *
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
#include <QtCore/QTimer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include <future>

#include <configurationmanager_interface.h>
#include <datatransfer_interface.h>
#include <account_const.h>

#include "typedefs.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.ConfigurationManager
 */
class ConfigurationManagerInterface : public QObject
{
    Q_OBJECT

public:
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> confHandlers;
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> dataXferHandlers;

    ConfigurationManagerInterface()
    {
        setObjectName("ConfigurationManagerInterface");
        using DRing::exportable_callback;
        using DRing::ConfigurationSignal;
        using DRing::AudioSignal;
        using DRing::DataTransferSignal;

        setObjectName("ConfigurationManagerInterface");
        confHandlers = {
            exportable_callback<ConfigurationSignal::VolumeChanged>(
                [this](const std::string& device, double value) {
                    Q_EMIT this->volumeChanged(QString(device.c_str()), value);
                }),
            exportable_callback<ConfigurationSignal::AccountsChanged>(
                [this]() { Q_EMIT this->accountsChanged(); }),
            exportable_callback<ConfigurationSignal::AccountDetailsChanged>(
                [this](const std::string& account_id,
                       const std::map<std::string, std::string>& details) {
                    Q_EMIT this->accountDetailsChanged(QString(account_id.c_str()),
                                                       convertMap(details));
                }),
            exportable_callback<ConfigurationSignal::StunStatusFailed>(
                [this](const std::string& reason) {
                    Q_EMIT this->stunStatusFailure(QString(reason.c_str()));
                }),
            exportable_callback<ConfigurationSignal::RegistrationStateChanged>(
                [this](const std::string& accountID,
                       const std::string& registration_state,
                       unsigned detail_code,
                       const std::string& detail_str) {
                    Q_EMIT this->registrationStateChanged(QString(accountID.c_str()),
                                                          QString(registration_state.c_str()),
                                                          detail_code,
                                                          QString(detail_str.c_str()));
                }),
            exportable_callback<ConfigurationSignal::VolatileDetailsChanged>(
                [this](const std::string& accountID,
                       const std::map<std::string, std::string>& details) {
                    Q_EMIT this->volatileAccountDetailsChanged(QString(accountID.c_str()),
                                                               convertMap(details));
                }),
            exportable_callback<ConfigurationSignal::Error>(
                [this](int code) { Q_EMIT this->errorAlert(code); }),
            exportable_callback<ConfigurationSignal::CertificateExpired>(
                [this](const std::string& certId) {
                    Q_EMIT this->certificateExpired(QString(certId.c_str()));
                }),
            exportable_callback<ConfigurationSignal::CertificatePinned>(
                [this](const std::string& certId) {
                    Q_EMIT this->certificatePinned(QString(certId.c_str()));
                }),
            exportable_callback<ConfigurationSignal::CertificatePathPinned>(
                [this](const std::string& certPath, const std::vector<std::string>& list) {
                    Q_EMIT this->certificatePathPinned(QString(certPath.c_str()),
                                                       convertStringList(list));
                }),
            exportable_callback<ConfigurationSignal::CertificateStateChanged>(
                [this](const std::string& accountID,
                       const std::string& certId,
                       const std::string& state) {
                    QTimer::singleShot(0, [this, accountID, certId, state] {
                        Q_EMIT this->certificateStateChanged(QString(accountID.c_str()),
                                                             QString(certId.c_str()),
                                                             QString(state.c_str()));
                    });
                }),
            exportable_callback<DRing::ConfigurationSignal::AccountMessageStatusChanged>(
                [this](const std::string& accountID, uint64_t id, const std::string& to, int status) {
                    Q_EMIT this->accountMessageStatusChanged(QString(accountID.c_str()),
                                                             id,
                                                             QString(to.c_str()),
                                                             status);
                }),
            exportable_callback<ConfigurationSignal::IncomingTrustRequest>(
                [this](const std::string& accountId,
                       const std::string& certId,
                       const std::vector<uint8_t>& payload,
                       time_t timestamp) {
                    Q_EMIT this->incomingTrustRequest(QString(accountId.c_str()),
                                                      QString(certId.c_str()),
                                                      QByteArray(reinterpret_cast<const char*>(
                                                                     payload.data()),
                                                                 payload.size()),
                                                      timestamp);
                }),
            exportable_callback<ConfigurationSignal::KnownDevicesChanged>(
                [this](const std::string& accountId,
                       const std::map<std::string, std::string>& devices) {
                    Q_EMIT this->knownDevicesChanged(QString(accountId.c_str()),
                                                     convertMap(devices));
                }),
            exportable_callback<ConfigurationSignal::DeviceRevocationEnded>(
                [this](const std::string& accountId, const std::string& device, int status) {
                    Q_EMIT this->deviceRevocationEnded(QString(accountId.c_str()),
                                                       QString(device.c_str()),
                                                       status);
                }),
            exportable_callback<ConfigurationSignal::AccountProfileReceived>(
                [this](const std::string& accountId,
                       const std::string& displayName,
                       const std::string& userPhoto) {
                    Q_EMIT this->accountProfileReceived(QString(accountId.c_str()),
                                                        QString(displayName.c_str()),
                                                        QString(userPhoto.c_str()));
                }),
            exportable_callback<ConfigurationSignal::ExportOnRingEnded>(
                [this](const std::string& accountId, int status, const std::string& pin) {
                    Q_EMIT this->exportOnRingEnded(QString(accountId.c_str()),
                                                   status,
                                                   QString(pin.c_str()));
                }),
            exportable_callback<ConfigurationSignal::NameRegistrationEnded>(
                [this](const std::string& accountId, int status, const std::string& name) {
                    Q_EMIT this->nameRegistrationEnded(QString(accountId.c_str()),
                                                       status,
                                                       QString(name.c_str()));
                }),
            exportable_callback<ConfigurationSignal::RegisteredNameFound>(
                [this](const std::string& accountId,
                       int status,
                       const std::string& address,
                       const std::string& name) {
                    Q_EMIT this->registeredNameFound(QString(accountId.c_str()),
                                                     status,
                                                     QString(address.c_str()),
                                                     QString(name.c_str()));
                }),
            exportable_callback<ConfigurationSignal::IncomingAccountMessage>(
                [this](const std::string& account_id,
                       const std::string& msgId,
                       const std::string& from,
                       const std::map<std::string, std::string>& payloads) {
                    Q_EMIT this->incomingAccountMessage(QString(account_id.c_str()),
                                                        QString(msgId.c_str()),
                                                        QString(from.c_str()),
                                                        convertMap(payloads));
                }),
            exportable_callback<ConfigurationSignal::MediaParametersChanged>(
                [this](const std::string& account_id) {
                    Q_EMIT this->mediaParametersChanged(QString(account_id.c_str()));
                }),
            exportable_callback<AudioSignal::DeviceEvent>(
                [this]() { Q_EMIT this->audioDeviceEvent(); }),
            exportable_callback<AudioSignal::AudioMeter>([this](const std::string& id, float level) {
                Q_EMIT this->audioMeter(QString(id.c_str()), level);
            }),
            exportable_callback<ConfigurationSignal::MigrationEnded>(
                [this](const std::string& account_id, const std::string& result) {
                    Q_EMIT this->migrationEnded(QString(account_id.c_str()),
                                                QString(result.c_str()));
                }),
            exportable_callback<ConfigurationSignal::ContactAdded>(
                [this](const std::string& account_id, const std::string& uri, const bool& confirmed) {
                    Q_EMIT this->contactAdded(QString(account_id.c_str()),
                                              QString(uri.c_str()),
                                              confirmed);
                }),
            exportable_callback<ConfigurationSignal::ProfileReceived>(
                [this](const std::string& accountID,
                       const std::string& peer,
                       const std::string& vCard) {
                    Q_EMIT this->profileReceived(QString(accountID.c_str()),
                                                 QString(peer.c_str()),
                                                 QString(vCard.c_str()));
                }),
            exportable_callback<ConfigurationSignal::ContactRemoved>(
                [this](const std::string& account_id, const std::string& uri, const bool& banned) {
                    Q_EMIT this->contactRemoved(QString(account_id.c_str()),
                                                QString(uri.c_str()),
                                                banned);
                }),
            exportable_callback<ConfigurationSignal::MessageSend>(
                [this](const std::string& message) {
                    Q_EMIT this->messageSend(QString(message.c_str()));
                }),
            exportable_callback<ConfigurationSignal::ComposingStatusChanged>(
                [this](const std::string& account_id, const std::string& from, int status) {
                    Q_EMIT this->composingStatusChanged(QString(account_id.c_str()),
                                                        QString(from.c_str()),
                                                        status > 0 ? true : false);
                }),
            exportable_callback<ConfigurationSignal::UserSearchEnded>(
                [this](const std::string& account_id,
                       int status,
                       const std::string& query,
                       const std::vector<std::map<std::string, std::string>>& results) {
                    Q_EMIT this->userSearchEnded(QString(account_id.c_str()),
                                                 status,
                                                 QString(query.c_str()),
                                                 convertVecMap(results));
                }),
        };

        dataXferHandlers = {
            exportable_callback<DataTransferSignal::DataTransferEvent>(
                [this](const uint64_t& transfer_id, const uint32_t& code) {
                    Q_EMIT this->dataTransferEvent(transfer_id, code);
                }),
        };
    }

    ~ConfigurationManagerInterface() {}

public Q_SLOTS: // METHODS
    QString addAccount(MapStringString details)
    {
        QString temp(DRing::addAccount(convertMap(details)).c_str());
        return temp;
    }

    bool exportOnRing(const QString& accountID, const QString& password)
    {
        return DRing::exportOnRing(accountID.toStdString(), password.toStdString());
    }

    bool exportToFile(const QString& accountID,
                      const QString& destinationPath,
                      const QString& password = {})
    {
        return DRing::exportToFile(accountID.toStdString(),
                                   destinationPath.toStdString(),
                                   password.toStdString());
    }

    MapStringString getKnownRingDevices(const QString& accountID)
    {
        MapStringString temp = convertMap(DRing::getKnownRingDevices(accountID.toStdString()));
        return temp;
    }

    bool lookupName(const QString& accountID, const QString& nameServiceURL, const QString& name)
    {
        return DRing::lookupName(accountID.toStdString(),
                                 nameServiceURL.toStdString(),
                                 name.toStdString());
    }

    bool lookupAddress(const QString& accountID,
                       const QString& nameServiceURL,
                       const QString& address)
    {
        return DRing::lookupAddress(accountID.toStdString(),
                                    nameServiceURL.toStdString(),
                                    address.toStdString());
    }

    bool registerName(const QString& accountID, const QString& password, const QString& name)
    {
        return DRing::registerName(accountID.toStdString(),
                                   password.toStdString(),
                                   name.toStdString());
    }

    MapStringString getAccountDetails(const QString& accountID)
    {
        MapStringString temp = convertMap(DRing::getAccountDetails(accountID.toStdString()));
        return temp;
    }

    QStringList getAccountList()
    {
        QStringList temp = convertStringList(DRing::getAccountList());
        return temp;
    }

    MapStringString getAccountTemplate(const QString& accountType)
    {
        MapStringString temp = convertMap(DRing::getAccountTemplate(accountType.toStdString()));
        return temp;
    }

    // TODO: works?
    VectorUInt getActiveCodecList(const QString& accountID)
    {
        return QVector<unsigned int>::fromStdVector(
            DRing::getActiveCodecList(accountID.toStdString()));
    }

    QString getAddrFromInterfaceName(const QString& interface)
    {
        QString temp(DRing::getAddrFromInterfaceName(interface.toStdString()).c_str());
        return temp;
    }

    QStringList getAllIpInterface()
    {
        QStringList temp = convertStringList(DRing::getAllIpInterface());
        return temp;
    }

    QStringList getAllIpInterfaceByName()
    {
        QStringList temp = convertStringList(DRing::getAllIpInterfaceByName());
        return temp;
    }

    MapStringString getCodecDetails(const QString& accountID, int payload)
    {
        MapStringString temp = convertMap(
            DRing::getCodecDetails(accountID.toStdString().c_str(), payload));
        return temp;
    }

    VectorUInt getCodecList()
    {
        return QVector<unsigned int>::fromStdVector(DRing::getCodecList());
    }

    VectorMapStringString getContacts(const QString& accountID)
    {
        VectorMapStringString temp;
        for (const auto& x : DRing::getContacts(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    int getAudioInputDeviceIndex(const QString& devname)
    {
        return DRing::getAudioInputDeviceIndex(devname.toStdString());
    }

    QStringList getAudioInputDeviceList()
    {
        QStringList temp = convertStringList(DRing::getAudioInputDeviceList());
        return temp;
    }

    QString getAudioManager()
    {
        QString temp(DRing::getAudioManager().c_str());
        return temp;
    }

    int getAudioOutputDeviceIndex(const QString& devname)
    {
        return DRing::getAudioOutputDeviceIndex(devname.toStdString());
    }

    QStringList getAudioOutputDeviceList()
    {
        QStringList temp = convertStringList(DRing::getAudioOutputDeviceList());
        return temp;
    }

    QStringList getAudioPluginList()
    {
        QStringList temp = convertStringList(DRing::getAudioPluginList());
        return temp;
    }

    VectorMapStringString getCredentials(const QString& accountID)
    {
        VectorMapStringString temp;
        for (auto x : DRing::getCredentials(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    QStringList getCurrentAudioDevicesIndex()
    {
        QStringList temp = convertStringList(DRing::getCurrentAudioDevicesIndex());
        return temp;
    }

    QString getCurrentAudioOutputPlugin()
    {
        QString temp(DRing::getCurrentAudioOutputPlugin().c_str());
        return temp;
    }

    int getHistoryLimit() { return DRing::getHistoryLimit(); }

    bool getIsAlwaysRecording() { return DRing::getIsAlwaysRecording(); }

    bool getNoiseSuppressState() { return DRing::getNoiseSuppressState(); }

    QString getRecordPath()
    {
        QString temp(DRing::getRecordPath().c_str());
        return temp;
    }

    bool getRecordPreview() { return DRing::getRecordPreview(); }

    int getRecordQuality() { return DRing::getRecordQuality(); }

    QStringList getSupportedAudioManagers()
    {
        QStringList temp;
        return temp;
    }

    MapStringString getShortcuts()
    {
        MapStringString temp = convertMap(DRing::getShortcuts());
        return temp;
    }

    QStringList getSupportedTlsMethod()
    {
        QStringList temp = convertStringList(DRing::getSupportedTlsMethod());
        return temp;
    }

    MapStringString validateCertificate(const QString& unused, const QString& certificate)
    {
        MapStringString temp = convertMap(
            DRing::validateCertificate(unused.toStdString(), certificate.toStdString()));
        return temp;
    }

    MapStringString validateCertificatePath(const QString& unused,
                                            const QString& certificate,
                                            const QString& privateKey,
                                            const QString& privateKeyPass,
                                            const QString& caListPath)
    {
        MapStringString temp = convertMap(
            DRing::validateCertificatePath(unused.toStdString(),
                                           certificate.toStdString(),
                                           privateKey.toStdString(),
                                           privateKeyPass.toStdString(),
                                           caListPath.toStdString()));
        return temp;
    }

    MapStringString getCertificateDetails(const QString& certificate)
    {
        MapStringString temp = convertMap(DRing::getCertificateDetails(certificate.toStdString()));
        return temp;
    }

    MapStringString getCertificateDetailsPath(const QString& certificate,
                                              const QString& privateKey,
                                              const QString& privateKeyPass)
    {
        MapStringString temp = convertMap(
            DRing::getCertificateDetailsPath(certificate.toStdString(),
                                             privateKey.toStdString(),
                                             privateKeyPass.toStdString()));
        return temp;
    }

    QStringList getSupportedCiphers(const QString& accountID)
    {
        QStringList temp = convertStringList(DRing::getSupportedCiphers(accountID.toStdString()));
        return temp;
    }

    double getVolume(const QString& device) { return DRing::getVolume(device.toStdString()); }

    bool isAgcEnabled() { return DRing::isAgcEnabled(); }

    bool isCaptureMuted() { return DRing::isCaptureMuted(); }

    bool isDtmfMuted() { return DRing::isDtmfMuted(); }

    bool isPlaybackMuted() { return DRing::isPlaybackMuted(); }

    void muteCapture(bool mute) { DRing::muteCapture(mute); }

    void muteDtmf(bool mute) { DRing::muteDtmf(mute); }

    void mutePlayback(bool mute) { DRing::mutePlayback(mute); }

    void registerAllAccounts() { DRing::registerAllAccounts(); }

    void monitor(bool continuous) { DRing::monitor(continuous); }

    void removeAccount(const QString& accountID) { DRing::removeAccount(accountID.toStdString()); }

    bool changeAccountPassword(const QString& id,
                               const QString& currentPassword,
                               const QString& newPassword)
    {
        return DRing::changeAccountPassword(id.toStdString(),
                                            currentPassword.toStdString(),
                                            newPassword.toStdString());
    }

    void sendRegister(const QString& accountID, bool enable)
    {
        DRing::sendRegister(accountID.toStdString(), enable);
    }

    void setAccountDetails(const QString& accountID, MapStringString details)
    {
        DRing::setAccountDetails(accountID.toStdString(), convertMap(details));
    }

    void setAccountsOrder(const QString& order) { DRing::setAccountsOrder(order.toStdString()); }

    void setActiveCodecList(const QString& accountID, VectorUInt& list)
    {
        // const std::vector<unsigned int> converted = convertStringList(list);
        DRing::setActiveCodecList(accountID.toStdString(), list.toStdVector());
    }

    void setAgcState(bool enabled) { DRing::setAgcState(enabled); }

    void setAudioInputDevice(int index) { DRing::setAudioInputDevice(index); }

    bool setAudioManager(const QString& api) { return DRing::setAudioManager(api.toStdString()); }

    void setAudioOutputDevice(int index) { DRing::setAudioOutputDevice(index); }

    void setAudioPlugin(const QString& audioPlugin)
    {
        DRing::setAudioPlugin(audioPlugin.toStdString());
    }

    void setAudioRingtoneDevice(int index) { DRing::setAudioRingtoneDevice(index); }

    void setCredentials(const QString& accountID, VectorMapStringString credentialInformation)
    {
        std::vector<std::map<std::string, std::string>> temp;
        for (auto x : credentialInformation) {
            temp.push_back(convertMap(x));
        }
        DRing::setCredentials(accountID.toStdString(), temp);
    }

    void setHistoryLimit(int days) { DRing::setHistoryLimit(days); }

    void setIsAlwaysRecording(bool enabled) { DRing::setIsAlwaysRecording(enabled); }

    void setNoiseSuppressState(bool state) { DRing::setNoiseSuppressState(state); }

    bool isAudioMeterActive(const QString& id)
    {
        return DRing::isAudioMeterActive(id.toStdString());
    }

    void setAudioMeterState(const QString& id, bool state)
    {
        DRing::setAudioMeterState(id.toStdString(), state);
    }

    void setRecordPath(const QString& rec) { DRing::setRecordPath(rec.toStdString()); }

    void setRecordPreview(const bool& rec) { DRing::setRecordPreview(rec); }

    void setRecordQuality(const int& quality) { DRing::setRecordQuality(quality); }

    void setVolume(const QString& device, double value)
    {
        DRing::setVolume(device.toStdString(), value);
    }

    MapStringString getVolatileAccountDetails(const QString& accountID)
    {
        MapStringString temp = convertMap(DRing::getVolatileAccountDetails(accountID.toStdString()));
        return temp;
    }

    QStringList getPinnedCertificates()
    {
        QStringList temp = convertStringList(DRing::getPinnedCertificates());
        return temp;
    }

    QStringList pinCertificate(const QByteArray& content, bool local)
    {
        std::vector<unsigned char> raw(content.begin(), content.end());
        return convertStringList(DRing::pinCertificate(raw, local));
    }

    bool unpinCertificate(const QString& certId)
    {
        return DRing::unpinCertificate(certId.toStdString());
    }

    void pinCertificatePath(const QString& certPath)
    {
        DRing::pinCertificatePath(certPath.toStdString());
    }

    uint unpinCertificatePath(const QString& certPath)
    {
        return DRing::unpinCertificatePath(certPath.toStdString());
    }

    bool pinRemoteCertificate(const QString& accountId, const QString& certPath)
    {
        return DRing::pinRemoteCertificate(accountId.toStdString(), certPath.toStdString());
    }

    bool setCertificateStatus(const QString& accountId,
                              const QString& certPath,
                              const QString& status)
    {
        return DRing::setCertificateStatus(accountId.toStdString(),
                                           certPath.toStdString(),
                                           status.toStdString());
    }

    QStringList getCertificatesByStatus(const QString& accountId, const QString& status)
    {
        return convertStringList(
            DRing::getCertificatesByStatus(accountId.toStdString(), status.toStdString()));
    }

    VectorMapStringString getTrustRequests(const QString& accountId)
    {
        return convertVecMap(DRing::getTrustRequests(accountId.toStdString()));
    }

    bool acceptTrustRequest(const QString& accountId, const QString& from)
    {
        return DRing::acceptTrustRequest(accountId.toStdString(), from.toStdString());
    }

    bool discardTrustRequest(const QString& accountId, const QString& from)
    {
        return DRing::discardTrustRequest(accountId.toStdString(), from.toStdString());
    }

    void sendTrustRequest(const QString& accountId, const QString& from, const QByteArray& payload)
    {
        std::vector<unsigned char> raw(payload.begin(), payload.end());
        DRing::sendTrustRequest(accountId.toStdString(), from.toStdString(), raw);
    }

    void removeContact(const QString& accountId, const QString& uri, bool ban)
    {
        DRing::removeContact(accountId.toStdString(), uri.toStdString(), ban);
    }

    void revokeDevice(const QString& accountId, const QString& password, const QString& deviceId)
    {
        DRing::revokeDevice(accountId.toStdString(), password.toStdString(), deviceId.toStdString());
    }

    void addContact(const QString& accountId, const QString& uri)
    {
        DRing::addContact(accountId.toStdString(), uri.toStdString());
    }

    uint64_t sendTextMessage(const QString& accountId,
                             const QString& to,
                             const QMap<QString, QString>& payloads)
    {
        return DRing::sendAccountTextMessage(accountId.toStdString(),
                                             to.toStdString(),
                                             convertMap(payloads));
    }

    QVector<Message> getLastMessages(const QString& accountID, const uint64_t& base_timestamp)
    {
        QVector<Message> result;
        for (auto& message : DRing::getLastMessages(accountID.toStdString(), base_timestamp)) {
            result.append({message.from.c_str(), convertMap(message.payloads), message.received});
        }
        return result;
    }

    bool setCodecDetails(const QString& accountId,
                         unsigned int codecId,
                         const MapStringString& details)
    {
        return DRing::setCodecDetails(accountId.toStdString(), codecId, convertMap(details));
    }

    int getMessageStatus(uint64_t id) { return DRing::getMessageStatus(id); }

    MapStringString getNearbyPeers(const QString& accountID)
    {
        return convertMap(DRing::getNearbyPeers(accountID.toStdString()));
    }

    void connectivityChanged() { DRing::connectivityChanged(); }

    MapStringString getContactDetails(const QString& accountID, const QString& uri)
    {
        return convertMap(DRing::getContactDetails(accountID.toStdString(), uri.toStdString()));
    }

    VectorULongLong dataTransferList() { return convertVectorULongLong(DRing::dataTransferList()); }

    uint32_t sendFile(const DataTransferInfo& lrc_info, DRing::DataTransferId& id)
    {
        DRing::DataTransferInfo jamid_info;
        jamid_info.accountId = lrc_info.accountId.toStdString();
        jamid_info.lastEvent = decltype(jamid_info.lastEvent)(lrc_info.lastEvent);
        jamid_info.flags = lrc_info.flags;
        jamid_info.totalSize = lrc_info.totalSize;
        jamid_info.bytesProgress = lrc_info.bytesProgress;
        jamid_info.peer = lrc_info.peer.toStdString();
        jamid_info.displayName = lrc_info.displayName.toStdString();
        jamid_info.path = lrc_info.path.toStdString();
        jamid_info.mimetype = lrc_info.mimetype.toStdString();
        return uint32_t(DRing::sendFile(jamid_info, id));
    }

    uint32_t dataTransferInfo(uint64_t transfer_id, DataTransferInfo& lrc_info)
    {
        DRing::DataTransferInfo jamid_info;
        auto error = uint32_t(DRing::dataTransferInfo(transfer_id, jamid_info));
        lrc_info.accountId = QString::fromStdString(jamid_info.accountId);
        lrc_info.lastEvent = quint32(jamid_info.lastEvent);
        lrc_info.flags = jamid_info.flags;
        lrc_info.totalSize = jamid_info.totalSize;
        lrc_info.bytesProgress = jamid_info.bytesProgress;
        lrc_info.peer = QString::fromStdString(jamid_info.peer);
        lrc_info.displayName = QString::fromStdString(jamid_info.displayName);
        lrc_info.path = QString::fromStdString(jamid_info.path);
        lrc_info.mimetype = QString::fromStdString(jamid_info.mimetype);
        return error;
    }

    uint64_t dataTransferBytesProgress(uint64_t transfer_id, int64_t& total, int64_t& progress)
    {
        return uint32_t(DRing::dataTransferBytesProgress(transfer_id, total, progress));
    }

    uint32_t acceptFileTransfer(uint64_t transfer_id, const QString& file_path, int64_t offset)
    {
        return uint32_t(DRing::acceptFileTransfer(transfer_id, file_path.toStdString(), offset));
    }

    uint32_t cancelDataTransfer(int64_t transfer_id)
    {
        return uint32_t(DRing::cancelDataTransfer(transfer_id));
    }

    void enableProxyClient(const QString& accountID, bool enable)
    {
        DRing::enableProxyClient(accountID.toStdString(), enable);
    }

    void setPushNotificationToken(const QString& token)
    {
        DRing::setPushNotificationToken(token.toStdString());
    }

    void pushNotificationReceived(const QString& from, const MapStringString& data)
    {
        DRing::pushNotificationReceived(from.toStdString(), convertMap(data));
    }

    void setIsComposing(const QString& accountId, const QString& contactId, bool isComposing)
    {
        DRing::setIsComposing(accountId.toStdString(), contactId.toStdString(), isComposing);
    }

    bool setMessageDisplayed(const QString& accountId,
                             const QString& contactId,
                             const QString& messageId,
                             int status)
    {
        return DRing::setMessageDisplayed(accountId.toStdString(),
                                          contactId.toStdString(),
                                          messageId.toStdString(),
                                          status);
    }

    bool searchUser(const QString& accountId, const QString& query)
    {
        return DRing::searchUser(accountId.toStdString(), query.toStdString());
    }

    void setDefaultModerator(const QString& accountID, const QString& peerURI, const bool& state)
    {
        DRing::setDefaultModerator(accountID.toStdString(), peerURI.toStdString(), state);
    }

    QStringList getDefaultModerators(const QString& accountID)
    {
        return convertStringList(DRing::getDefaultModerators(accountID.toStdString()));
    }

    void enableLocalModerators(const QString& accountID, const bool& isModEnabled)
    {
        DRing::enableLocalModerators(accountID.toStdString(), isModEnabled);
    }

    bool isLocalModeratorsEnabled(const QString& accountID)
    {
        return DRing::isLocalModeratorsEnabled(accountID.toStdString());
    }

    void setAllModerators(const QString& accountID, const bool& allModerators)
    {
        DRing::setAllModerators(accountID.toStdString(), allModerators);
    }

    bool isAllModerators(const QString& accountID)
    {
        return DRing::isAllModerators(accountID.toStdString());
    }

Q_SIGNALS: // SIGNALS
    void volumeChanged(const QString& device, double value);
    void accountsChanged();
    void accountDetailsChanged(const QString& accountId, const MapStringString& details);
    void historyChanged();
    void stunStatusFailure(const QString& reason);
    void registrationStateChanged(const QString& accountID,
                                  const QString& registration_state,
                                  unsigned detail_code,
                                  const QString& detail_str);
    void stunStatusSuccess(const QString& message);
    void errorAlert(int code);
    void volatileAccountDetailsChanged(const QString& accountID, MapStringString details);
    void certificatePinned(const QString& certId);
    void certificatePathPinned(const QString& path, const QStringList& certIds);
    void certificateExpired(const QString& certId);
    void certificateStateChanged(const QString& accountId,
                                 const QString& certId,
                                 const QString& status);
    void incomingTrustRequest(const QString& accountId,
                              const QString& from,
                              const QByteArray& payload,
                              qulonglong timeStamp);
    void knownDevicesChanged(const QString& accountId, const MapStringString& devices);
    void exportOnRingEnded(const QString& accountId, int status, const QString& pin);
    void incomingAccountMessage(const QString& accountId,
                                const QString msgId,
                                const QString& from,
                                const MapStringString& payloads);
    void mediaParametersChanged(const QString& accountId);
    void audioDeviceEvent();
    void audioMeter(const QString& id, float level);
    void accountMessageStatusChanged(const QString& accountId,
                                     const uint64_t id,
                                     const QString& to,
                                     int status);
    void nameRegistrationEnded(const QString& accountId, int status, const QString& name);
    void registeredNameFound(const QString& accountId,
                             int status,
                             const QString& address,
                             const QString& name);
    void migrationEnded(const QString& accountID, const QString& result);
    void contactAdded(const QString& accountID, const QString& uri, bool banned);
    void contactRemoved(const QString& accountID, const QString& uri, bool banned);
    void profileReceived(const QString& accountID, const QString& peer, const QString& vCard);
    void dataTransferEvent(qulonglong transfer_id, uint code);
    void deviceRevocationEnded(const QString& accountId, const QString& deviceId, int status);
    void accountProfileReceived(const QString& accountId,
                                const QString& displayName,
                                const QString& userPhoto);
    void messageSend(const QString& message);
    void composingStatusChanged(const QString& accountId,
                                const QString& contactId,
                                bool isComposing);
    void userSearchEnded(const QString& accountId,
                         int status,
                         const QString& query,
                         VectorMapStringString results);
};

namespace org {
namespace ring {
namespace Ring {
typedef ::ConfigurationManagerInterface ConfigurationManager;
}
} // namespace ring
} // namespace org

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
#ifndef CONFIGURATIONMANAGER_STATIC_INTERFACE_H
#define CONFIGURATIONMANAGER_STATIC_INTERFACE_H

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

#include "typedefs.h"
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

    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> confHandlers;

    ConfigurationManagerInterface()
    {
        setObjectName("ConfigurationManagerInterface");
        using DRing::exportable_callback;
        using DRing::ConfigurationSignal;

        setObjectName("ConfigurationManagerInterface");
        confHandlers = {
            exportable_callback<ConfigurationSignal::VolumeChanged>(
                [this] (const std::string &device, double value) {
                       QTimer::singleShot(0, [this,device,value] {
                             emit this->volumeChanged(QString(device.c_str()), value);
                       });
            }),
            exportable_callback<ConfigurationSignal::AccountsChanged>(
                [this] () {
                       QTimer::singleShot(0, [this] {
                             emit this->accountsChanged();
                       });
             }),
            exportable_callback<ConfigurationSignal::StunStatusFailed>(
                [this] (const std::string &reason) {
                       QTimer::singleShot(0, [this, reason] {
                             emit this->stunStatusFailure(QString(reason.c_str()));
                       });
            }),
            exportable_callback<ConfigurationSignal::RegistrationStateChanged>(
                [this] (const std::string &accountID, const std::string& registration_state, unsigned detail_code, const std::string& detail_str) {
                       QTimer::singleShot(0, [this, accountID, registration_state, detail_code, detail_str] {
                             emit this->registrationStateChanged(QString(accountID.c_str()),
                                                                QString(registration_state.c_str()),
                                                                detail_code,
                                                                QString(detail_str.c_str()));
                       });
            }),
            exportable_callback<ConfigurationSignal::VolatileDetailsChanged>(
                [this] (const std::string &accountID, const std::map<std::string, std::string>& details) {
                       QTimer::singleShot(0, [this, accountID, details] {
                         emit this->volatileAccountDetailsChanged(QString(accountID.c_str()), convertMap(details));
                       });
            }),
            exportable_callback<ConfigurationSignal::Error>(
                [this] (int code) {
                       QTimer::singleShot(0, [this,code] {
                         emit this->errorAlert(code);
                       });
            })
        };
    }

    ~ConfigurationManagerInterface() {}

public Q_SLOTS: // METHODS
    QString addAccount(MapStringString details)
    {
        QString temp(
            DRing::addAccount(convertMap(details)).c_str());
        return temp;
    }

    MapStringString getAccountDetails(const QString &accountID)
    {
        MapStringString temp =
            convertMap(DRing::getAccountDetails(accountID.toStdString()));
        return temp;
    }

    QStringList getAccountList()
    {
        QStringList temp =
            convertStringList(DRing::getAccountList());
        return temp;
    }

    MapStringString getAccountTemplate(const QString& accountType)
    {
        MapStringString temp =
            convertMap(DRing::getAccountTemplate(accountType.toStdString()));
        return temp;
    }

    // TODO: works?
    VectorUInt getActiveCodecList(const QString &accountID)
    {
        return QVector<unsigned int>::fromStdVector(
            DRing::getActiveCodecList(accountID.toStdString()));
    }

    QString getAddrFromInterfaceName(const QString &interface)
    {
        QString temp(
            DRing::getAddrFromInterfaceName(interface.toStdString()).c_str());
        return temp;
    }

    QStringList getAllIpInterface()
    {
        QStringList temp =
            convertStringList(DRing::getAllIpInterface());
        return temp;
    }

    QStringList getAllIpInterfaceByName()
    {
        QStringList temp =
            convertStringList(DRing::getAllIpInterfaceByName());
        return temp;
    }

    MapStringString getCodecDetails(const QString accountID, int payload)
    {
        MapStringString temp =
            convertMap(DRing::getCodecDetails(
                accountID.toStdString().c_str(), payload));
        return temp;
    }

    VectorUInt getCodecList()
    {
        return QVector<unsigned int>::fromStdVector(DRing::getCodecList());
    }

    int getAudioInputDeviceIndex(const QString &devname)
    {
        return DRing::getAudioInputDeviceIndex(devname.toStdString());
    }

    QStringList getAudioInputDeviceList()
    {
        QStringList temp =
            convertStringList(DRing::getAudioInputDeviceList());
        return temp;
    }

    QString getAudioManager()
    {
        QString temp(
            DRing::getAudioManager().c_str());
        return temp;
    }

    int getAudioOutputDeviceIndex(const QString &devname)
    {
        return DRing::getAudioOutputDeviceIndex(devname.toStdString());
    }

    QStringList getAudioOutputDeviceList()
    {
        QStringList temp =
            convertStringList(DRing::getAudioOutputDeviceList());
        return temp;
    }

    QStringList getAudioPluginList()
    {
        QStringList temp =
            convertStringList(DRing::getAudioPluginList());
        return temp;
    }

    VectorMapStringString getCredentials(const QString &accountID)
    {
        VectorMapStringString temp;
        for(auto x : DRing::getCredentials(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
        return temp;
    }

    QStringList getCurrentAudioDevicesIndex()
    {
        QStringList temp =
            convertStringList(DRing::getCurrentAudioDevicesIndex());
        return temp;
    }

    QString getCurrentAudioOutputPlugin()
    {
        QString temp(
            DRing::getCurrentAudioOutputPlugin().c_str());
        return temp;
    }

    int getHistoryLimit()
    {
        return DRing::getHistoryLimit();
    }

    MapStringString getHookSettings()
    {
        MapStringString temp =
            convertMap(DRing::getHookSettings());
        return temp;
    }

    MapStringString getIp2IpDetails()
    {
        MapStringString temp =
            convertMap(DRing::getIp2IpDetails());
        return temp;
    }

    bool getIsAlwaysRecording()
    {
        return DRing::getIsAlwaysRecording();
    }

    bool getNoiseSuppressState()
    {
        return DRing::getNoiseSuppressState();
    }

    QString getRecordPath()
    {
        QString temp(
            DRing::getRecordPath().c_str());
        return temp;
    }

    QStringList getSupportedAudioManagers()
    {
        QStringList temp;
        return temp;
    }

    MapStringString getShortcuts()
    {
        MapStringString temp =
            convertMap(DRing::getShortcuts());
        return temp;
    }

    QStringList getSupportedTlsMethod()
    {
        QStringList temp =
            convertStringList(DRing::getSupportedTlsMethod());
        return temp;
    }

    MapStringString getTlsSettings()
    {
        MapStringString temp =
            convertMap(DRing::getTlsSettings());
        return temp;
    }

    MapStringString validateCertificate(const QString& unused, const QString certificate, const QString& privateKey)
    {
        MapStringString temp =
            convertMap(DRing::validateCertificate(unused.toStdString(),
                                                certificate.toStdString(),
                                                privateKey.toStdString()));
        return temp;
    }

    MapStringString validateCertificateRaw(const QString& unused, const QByteArray& content)
    {
        std::vector<unsigned char> raw(content.begin(), content.end());
        MapStringString temp =
            convertMap(DRing::validateCertificateRaw(unused.toStdString(), raw));
        return temp;
    }

    MapStringString getCertificateDetails(const QString &certificate)
    {
        MapStringString temp =
            convertMap(DRing::getCertificateDetails(certificate.toStdString()));
        return temp;
    }

    MapStringString getCertificateDetailsRaw(const QByteArray &content)
    {
        std::vector<unsigned char> raw(content.begin(), content.end());
        MapStringString temp =
            convertMap(DRing::getCertificateDetailsRaw(raw));
        return temp;
    }

    QStringList getSupportedCiphers(const QString &accountID)
    {
        QStringList temp =
            convertStringList(DRing::getSupportedCiphers(accountID.toStdString()));
        return temp;
    }

    MapStringString getTlsDefaultSettings()
    {
        MapStringString temp =
            convertMap(DRing::getTlsDefaultSettings());
        return temp;
    }

    double getVolume(const QString &device)
    {
        return DRing::getVolume(device.toStdString());
    }

    bool isAgcEnabled()
    {
        return DRing::isAgcEnabled();
    }

    bool isCaptureMuted()
    {
        return DRing::isCaptureMuted();
    }

    bool isDtmfMuted()
    {
        return DRing::isDtmfMuted();
    }

    int isIax2Enabled()
    {
        return DRing::isIax2Enabled();
    }

    bool isPlaybackMuted()
    {
        return DRing::isPlaybackMuted();
    }

    void muteCapture(bool mute)
    {
        DRing::muteCapture(mute);
    }

    void muteDtmf(bool mute)
    {
        DRing::muteDtmf(mute);
    }

    void mutePlayback(bool mute)
    {
        DRing::mutePlayback(mute);
    }

    void registerAllAccounts()
    {
        DRing::registerAllAccounts();
    }

    void removeAccount(const QString &accountID)
    {
        DRing::removeAccount(accountID.toStdString());
    }

    void sendRegister(const QString &accountID, bool enable)
    {
        DRing::sendRegister(accountID.toStdString(), enable);
    }

    void setAccountDetails(const QString &accountID, MapStringString details)
    {
        DRing::setAccountDetails(accountID.toStdString(),
            convertMap(details));
    }

    void setAccountsOrder(const QString &order)
    {
        DRing::setAccountsOrder(order.toStdString());
    }

    void setActiveCodecList(const QString &accountID, VectorUInt &list)
    {
        //const std::vector<unsigned int> converted = convertStringList(list);
        DRing::setActiveCodecList(accountID.toStdString(),
        list.toStdVector());
    }

    void setAgcState(bool enabled)
    {
        DRing::setAgcState(enabled);
    }

    void setAudioInputDevice(int index)
    {
        DRing::setAudioInputDevice(index);
    }

    bool setAudioManager(const QString &api)
    {
        return DRing::setAudioManager(api.toStdString());
    }

    void setAudioOutputDevice(int index)
    {
        DRing::setAudioOutputDevice(index);
    }

    void setAudioPlugin(const QString &audioPlugin)
    {
        DRing::setAudioPlugin(audioPlugin.toStdString());
    }

    void setAudioRingtoneDevice(int index)
    {
        DRing::setAudioRingtoneDevice(index);
    }

    void setCredentials(const QString &accountID, VectorMapStringString credentialInformation)
    {
        std::vector<std::map<std::string, std::string> > temp;
        for (auto x : credentialInformation) {
            temp.push_back(convertMap(x));
        }
        DRing::setCredentials(accountID.toStdString(), temp);
    }

    void setHistoryLimit(int days)
    {
        DRing::setHistoryLimit(days);
    }

    void setHookSettings(MapStringString settings)
    {
        DRing::setHookSettings(convertMap(settings));
    }

    void setIsAlwaysRecording(bool enabled)
    {
        DRing::setIsAlwaysRecording(enabled);
    }

    void setNoiseSuppressState(bool state)
    {
        DRing::setNoiseSuppressState(state);
    }

    void setRecordPath(const QString &rec)
    {
        DRing::setRecordPath(rec.toStdString());
    }

    void setShortcuts(MapStringString shortcutsMap)
    {
        DRing::setShortcuts(convertMap(shortcutsMap));
    }

    void setTlsSettings(MapStringString details)
    {
        DRing::setTlsSettings(convertMap(details));
    }

    void setVolume(const QString &device, double value)
    {
        DRing::setVolume(device.toStdString(), value);
    }

    MapStringString getVolatileAccountDetails(const QString &accountID)
    {
        MapStringString temp = convertMap(DRing::getVolatileAccountDetails(accountID.toStdString()));
        return temp;
    }

Q_SIGNALS: // SIGNALS
    void volumeChanged(const QString &device, double value);
    void accountsChanged();
    void historyChanged();
    void stunStatusFailure(const QString &reason);
    void registrationStateChanged(const QString& accountID, const QString& registration_state, unsigned detail_code, const QString& detail_str);
    void stunStatusSuccess(const QString &message);
    void errorAlert(int code);
    void volatileAccountDetailsChanged(const QString &accountID, MapStringString details);

};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::ConfigurationManagerInterface ConfigurationManager;
    }
  }
}
#endif

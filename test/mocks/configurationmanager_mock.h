/******************************************************************************
*   Copyright (C) 2014-2017 Savoir-faire Linux                                 *
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
#include <account_const.h>

#include "typedefs.h"
#include "../../src/qtwrapper/conversions_wrap.hpp"

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
   }

   ~ConfigurationManagerInterface() {}

public Q_SLOTS: // METHODS
    void emitIncomingAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads)
    {
        emit incomingAccountMessage(accountId, from, payloads);
    }

    QString addAccount(MapStringString details)
    {
        QString temp;
        return temp;
    }

    bool exportOnRing(const QString& accountID, const QString& password)
    {
        return false;
    }

    MapStringString getKnownRingDevices(const QString& accountID)
    {
        MapStringString temp;
        return temp;
    }

    bool lookupName(const QString& accountID, const QString& nameServiceURL, const QString& name)
    {
        return false;
    }

    bool lookupAddress(const QString& accountID, const QString& nameServiceURL, const QString& address)
    {
        return false;
    }

    bool registerName(const QString& accountID, const QString& password, const QString& name)
    {
        return false;
    }

    MapStringString getAccountDetails(const QString& accountID)
    {
        return MapStringString();
    }

    QStringList getAccountList()
    {
        return QStringList();
    }

    MapStringString getAccountTemplate(const QString& accountType)
    {
        return MapStringString();
    }

    VectorUInt getActiveCodecList(const QString& accountID)
    {
        return QVector<unsigned int>();
    }

    QString getAddrFromInterfaceName(const QString& interface)
    {
        return QString();
    }

    QStringList getAllIpInterface()
    {
        return QStringList();
    }

    QStringList getAllIpInterfaceByName()
    {
        return QStringList();
    }

    MapStringString getCodecDetails(const QString& accountID, int payload)
    {
        return MapStringString();
    }

    VectorUInt getCodecList()
    {
        return QVector<unsigned int>();
    }

    VectorMapStringString getContacts(const QString &accountID)
    {
        VectorMapStringString temp;
        return temp;
    }

    int getAudioInputDeviceIndex(const QString& devname)
    {
        return 0;
    }

    QStringList getAudioInputDeviceList()
    {
        return QStringList();
    }

    QString getAudioManager()
    {
        return QString();
    }

    int getAudioOutputDeviceIndex(const QString& devname)
    {
        return 0;
    }

    QStringList getAudioOutputDeviceList()
    {
        return QStringList();
    }

    QStringList getAudioPluginList()
    {
        return QStringList();
    }

    VectorMapStringString getCredentials(const QString& accountID)
    {
        return VectorMapStringString();
    }

    QStringList getCurrentAudioDevicesIndex()
    {
        return QStringList();
    }

    QString getCurrentAudioOutputPlugin()
    {
        return QString();
    }

    int getHistoryLimit()
    {
        return 0;
    }

    MapStringString getHookSettings()
    {
        return MapStringString();
    }

    bool getIsAlwaysRecording()
    {
        return false;
    }

    bool getNoiseSuppressState()
    {
        return false;
    }

    QString getRecordPath()
    {
        return QString();
    }

    QStringList getSupportedAudioManagers()
    {
        QStringList temp;
        return temp;
    }

    MapStringString getShortcuts()
    {
        return MapStringString();
    }

    QStringList getSupportedTlsMethod()
    {
        return QStringList();
    }

    MapStringString validateCertificate(const QString& unused, const QString& certificate)
    {
        return MapStringString();
    }

    MapStringString validateCertificatePath(const QString& unused, const QString& certificate, const QString& privateKey, const QString& privateKeyPass, const QString& caListPath)
    {
        return MapStringString();
    }

    MapStringString getCertificateDetails(const QString& certificate)
    {
        return MapStringString();
    }

    MapStringString getCertificateDetailsPath(const QString& certificate, const QString& privateKey, const QString& privateKeyPass)
    {
        return MapStringString();
    }

    QStringList getSupportedCiphers(const QString& accountID)
    {
        return QStringList();
    }

    MapStringString getTlsDefaultSettings()
    {
        return MapStringString();
    }

    double getVolume(const QString& device)
    {
        return 0;
    }

    bool isAgcEnabled()
    {
        return false;
    }

    bool isCaptureMuted()
    {
        return false;
    }

    bool isDtmfMuted()
    {
        return false;
    }

    bool isPlaybackMuted()
    {
        return false;
    }

    void muteCapture(bool mute)
    {
    }

    void muteDtmf(bool mute)
    {
    }

    void mutePlayback(bool mute)
    {
    }

    void registerAllAccounts()
    {
    }

    void removeAccount(const QString& accountID)
    {
    }

    int  exportAccounts(const QStringList& accountIDs, const QString& filePath, const QString& password)
    {
        return 0;
    }

    int importAccounts(const QString& filePath, const QString& password)
    {
        return 0;
    }

    void sendRegister(const QString& accountID, bool enable)
    {
    }

    void setAccountDetails(const QString& accountID, MapStringString details)
    {
    }

    void setAccountsOrder(const QString& order)
    {
    }

    void setActiveCodecList(const QString& accountID, VectorUInt &list)
    {
    }

    void setAgcState(bool enabled)
    {
    }

    void setAudioInputDevice(int index)
    {
    }

    bool setAudioManager(const QString& api)
    {
    }

    void setAudioOutputDevice(int index)
    {
    }

    void setAudioPlugin(const QString& audioPlugin)
    {
    }

    void setAudioRingtoneDevice(int index)
    {
    }

    void setCredentials(const QString& accountID, VectorMapStringString credentialInformation)
    {
    }

    void setHistoryLimit(int days)
    {
    }

    void setHookSettings(MapStringString settings)
    {
    }

    void setIsAlwaysRecording(bool enabled)
    {
    }

    void setNoiseSuppressState(bool state)
    {
    }

    void setRecordPath(const QString& rec)
    {
    }

    void setShortcuts(MapStringString shortcutsMap)
    {
    }

    void setVolume(const QString& device, double value)
    {
    }

    MapStringString getVolatileAccountDetails(const QString& accountID)
    {
        return MapStringString();
    }

    QStringList getPinnedCertificates()
    {
        return QStringList();
    }

    QStringList pinCertificate(const QByteArray& content, bool local)
    {
        return QStringList();
    }

    bool unpinCertificate(const QString& certId)
    {
        return false;
    }

    void pinCertificatePath(const QString& certPath)
    {
    }

    uint unpinCertificatePath(const QString& certPath)
    {
        return 0;
    }

    bool pinRemoteCertificate(const QString& accountId, const QString& certPath)
    {
        return false;
    }

    bool setCertificateStatus(const QString& accountId, const QString& certPath, const QString& status)
    {
        return false;
    }

    QStringList getCertificatesByStatus(const QString& accountId, const QString& status)
    {
        return QStringList();
    }

    VectorMapStringString getTrustRequests(const QString& accountId)
    {
        return VectorMapStringString();
    }

    bool acceptTrustRequest(const QString& accountId, const QString& from)
    {
        return false;
    }

    bool discardTrustRequest(const QString& accountId, const QString& from)
    {
        return false;
    }

    void sendTrustRequest(const QString& accountId, const QString& from, const QByteArray& payload)
    {
    }

    void removeContact(const QString &accountId, const QString &uri, bool ban)
    {
    }

    void addContact(const QString &accountId, const QString &uri)
    {
    }

    uint64_t sendTextMessage(const QString& accountId, const QString& to, const QMap<QString,QString>& payloads)
    {
        return 0;
    }

    bool setCodecDetails(const QString& accountId, unsigned int codecId, const MapStringString& details)
    {
        return false;
    }

    int getMessageStatus(uint64_t id)
    {
        return 0;
    }

    void connectivityChanged()
    {
    }

    MapStringString getContactDetails(const QString &accountID, const QString &uri)
    {
       return MapStringString();
    }

Q_SIGNALS: // SIGNALS
   void volumeChanged(const QString& device, double value);
   void accountsChanged();
   void historyChanged();
   void stunStatusFailure(const QString& reason);
   void registrationStateChanged(const QString& accountID, const QString& registration_state, unsigned detail_code, const QString& detail_str);
   void stunStatusSuccess(const QString& message);
   void errorAlert(int code);
   void volatileAccountDetailsChanged(const QString& accountID, MapStringString details);
   void certificatePinned(const QString& certId);
   void certificatePathPinned(const QString& path, const QStringList& certIds);
   void certificateExpired(const QString& certId);
   void certificateStateChanged(const QString& accountId, const QString& certId, const QString& status);
   void incomingTrustRequest(const QString& accountId, const QString& from, const QByteArray& payload, qulonglong timeStamp);
   void knownDevicesChanged(const QString& accountId, const MapStringString& devices);
   void exportOnRingEnded(const QString& accountId, int status, const QString& pin);
   void incomingAccountMessage(const QString& accountId, const QString& from, const MapStringString& payloads);
   void mediaParametersChanged(const QString& accountId);
   void audioDeviceEvent();
   void accountMessageStatusChanged(const QString& accountId, const uint64_t id, const QString& to, int status);
   void nameRegistrationEnded(const QString& accountId, int status, const QString& name);
   void registeredNameFound(const QString& accountId, int status, const QString& address, const QString& name);
   void migrationEnded(const QString &accountID, const QString &result);
   void contactAdded(const QString &accountID, const QString &uri, bool banned);
   void contactRemoved(const QString &accountID, const QString &uri, bool banned);


};

namespace org {
   namespace ring {
      namespace Ring {
         typedef ::ConfigurationManagerInterface ConfigurationManager;
      }
   }
}

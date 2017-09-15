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
#include "qtwrapper/conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.ConfigurationManager
 */
class ConfigurationManagerInterface: public QObject
{
    Q_OBJECT

private:
    QMap<QString, VectorMapStringString> accountToContactsMap;
    QStringList availableContacts_;

public:

   std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> confHandlers;

   ConfigurationManagerInterface()
   {
      setObjectName("ConfigurationManagerInterface");
      availableContacts_ << "contact0";
      availableContacts_ << "contact1";
      availableContacts_ << "contact2";
      availableContacts_ << "dummy";
      for (auto& account: getAccountList()) {
          auto contacts = VectorMapStringString();
          if (account.indexOf("ring") != -1) {
              for (auto& contactUri: availableContacts_) {
                  if (contactUri == "dummy") break;
                  auto contact = QMap<QString, QString>();
                  contact.insert("id", contactUri);
                  contact.insert("added", "true");
                  contact.insert("removed", "false");
                  contact.insert("confirmed", "true");
                  contact.insert("banned", "false");
                  contacts.push_back(contact);
              }
          }
          accountToContactsMap.insert(account, contacts);
      }
   }

   ~ConfigurationManagerInterface() {}

   void emitIncomingAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads)
   {
       emit incomingAccountMessage(accountId, from, payloads);
   }

public Q_SLOTS: // METHODS
    QString addAccount(MapStringString details)
    {
        Q_UNUSED(details)
        QString temp;
        return temp;
    }

    bool exportOnRing(const QString& accountId, const QString& password)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(password)
        return false;
    }

    MapStringString getKnownRingDevices(const QString& accountId)
    {
        Q_UNUSED(accountId)
        MapStringString temp;
        return temp;
    }

    bool lookupName(const QString& accountId, const QString& nameServiceURL, const QString& name)
    {
        Q_UNUSED(nameServiceURL)
        if (getAccountList().indexOf(accountId) == -1) return false;
        return availableContacts_.indexOf(name) != -1;
    }

    bool lookupAddress(const QString& accountId, const QString& nameServiceURL, const QString& address)
    {
        Q_UNUSED(nameServiceURL)
        if (getAccountList().indexOf(accountId) == -1) return false;
        return availableContacts_.indexOf(address) != -1;
    }

    bool registerName(const QString& accountId, const QString& password, const QString& name)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(password)
        Q_UNUSED(name)
        return false;
    }

    MapStringString getAccountDetails(const QString& accountId)
    {
        auto result = MapStringString();
        if (accountId.indexOf("ring") != -1) {
            result.insert("Account.type", "RING");
        } else {
            result.insert("Account.type", "SIP");
        }
        return result;
    }

    QStringList getAccountList()
    {
        auto accountList = QStringList();
        accountList << QString("ring0");
        accountList << QString("ring1");
        accountList << QString("sip0");
        accountList << QString("sip1");
        return accountList;
    }

    MapStringString getAccountTemplate(const QString& accountType)
    {
        Q_UNUSED(accountType)
        return MapStringString();
    }

    VectorUInt getActiveCodecList(const QString& accountId)
    {
        Q_UNUSED(accountId)
        return QVector<unsigned int>();
    }

    QString getAddrFromInterfaceName(const QString& interface)
    {
        Q_UNUSED(interface)
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

    MapStringString getCodecDetails(const QString& accountId, int payload)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(payload)
        return MapStringString();
    }

    VectorUInt getCodecList()
    {
        return QVector<unsigned int>();
    }

    VectorMapStringString getContacts(const QString &accountId)
    {
        if (accountToContactsMap.find(accountId) == accountToContactsMap.end()) {
            return VectorMapStringString();
        }
        return accountToContactsMap[accountId];
    }

    int getAudioInputDeviceIndex(const QString& devname)
    {
        Q_UNUSED(devname)
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
        Q_UNUSED(devname)
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

    VectorMapStringString getCredentials(const QString& accountId)
    {
        Q_UNUSED(accountId)
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
        Q_UNUSED(unused)
        Q_UNUSED(certificate)
        return MapStringString();
    }

    MapStringString validateCertificatePath(const QString& unused, const QString& certificate, const QString& privateKey, const QString& privateKeyPass, const QString& caListPath)
    {
        Q_UNUSED(unused)
        Q_UNUSED(certificate)
        Q_UNUSED(privateKey)
        Q_UNUSED(privateKeyPass)
        Q_UNUSED(caListPath)
        return MapStringString();
    }

    MapStringString getCertificateDetails(const QString& certificate)
    {
        Q_UNUSED(certificate)
        return MapStringString();
    }

    MapStringString getCertificateDetailsPath(const QString& certificate, const QString& privateKey, const QString& privateKeyPass)
    {
        Q_UNUSED(certificate)
        Q_UNUSED(privateKey)
        Q_UNUSED(privateKeyPass)
        return MapStringString();
    }

    QStringList getSupportedCiphers(const QString& accountId)
    {
        Q_UNUSED(accountId)
        return QStringList();
    }

    MapStringString getTlsDefaultSettings()
    {
        return MapStringString();
    }

    double getVolume(const QString& device)
    {
        Q_UNUSED(device)
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
        Q_UNUSED(mute)
    }

    void muteDtmf(bool mute)
    {
        Q_UNUSED(mute)
    }

    void mutePlayback(bool mute)
    {
        Q_UNUSED(mute)
    }

    void registerAllAccounts()
    {
    }

    void removeAccount(const QString& accountId)
    {
        Q_UNUSED(accountId)
    }

    int  exportAccounts(const QStringList& accountIds, const QString& filePath, const QString& password)
    {
        Q_UNUSED(accountIds)
        Q_UNUSED(filePath)
        Q_UNUSED(password)
        return 0;
    }

    int importAccounts(const QString& filePath, const QString& password)
    {
        Q_UNUSED(filePath)
        Q_UNUSED(password)
        return 0;
    }

    void sendRegister(const QString& accountId, bool enable)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(enable)
    }

    void setAccountDetails(const QString& accountId, MapStringString details)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(details)
    }

    void setAccountsOrder(const QString& order)
    {
        Q_UNUSED(order)
    }

    void setActiveCodecList(const QString& accountId, VectorUInt &list)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(list)
    }

    void setAgcState(bool enabled)
    {
        Q_UNUSED(enabled)
    }

    void setAudioInputDevice(int index)
    {
        Q_UNUSED(index)
    }

    bool setAudioManager(const QString& api)
    {
        Q_UNUSED(api)
        return false;
    }

    void setAudioOutputDevice(int index)
    {
        Q_UNUSED(index)
    }

    void setAudioPlugin(const QString& audioPlugin)
    {
        Q_UNUSED(audioPlugin)
    }

    void setAudioRingtoneDevice(int index)
    {
        Q_UNUSED(index)
    }

    void setCredentials(const QString& accountId, VectorMapStringString credentialInformation)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(credentialInformation)
    }

    void setHistoryLimit(int days)
    {
        Q_UNUSED(days)
    }

    void setHookSettings(MapStringString settings)
    {
        Q_UNUSED(settings)
    }

    void setIsAlwaysRecording(bool enabled)
    {
        Q_UNUSED(enabled)
    }

    void setNoiseSuppressState(bool state)
    {
        Q_UNUSED(state)
    }

    void setRecordPath(const QString& rec)
    {
        Q_UNUSED(rec)
    }

    void setShortcuts(MapStringString shortcutsMap)
    {
        Q_UNUSED(shortcutsMap)
    }

    void setVolume(const QString& device, double value)
    {
        Q_UNUSED(device)
        Q_UNUSED(value)
    }

    MapStringString getVolatileAccountDetails(const QString& accountId)
    {
        Q_UNUSED(accountId)
        return MapStringString();
    }

    QStringList getPinnedCertificates()
    {
        return QStringList();
    }

    QStringList pinCertificate(const QByteArray& content, bool local)
    {
        Q_UNUSED(content)
        Q_UNUSED(local)
        return QStringList();
    }

    bool unpinCertificate(const QString& certId)
    {
        Q_UNUSED(certId)
        return false;
    }

    void pinCertificatePath(const QString& certPath)
    {
        Q_UNUSED(certPath)
    }

    uint unpinCertificatePath(const QString& certPath)
    {
        Q_UNUSED(certPath)
        return 0;
    }

    bool pinRemoteCertificate(const QString& accountId, const QString& certPath)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(certPath)
        return false;
    }

    bool setCertificateStatus(const QString& accountId, const QString& certPath, const QString& status)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(certPath)
        Q_UNUSED(status)
        return false;
    }

    QStringList getCertificatesByStatus(const QString& accountId, const QString& status)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(status)
        return QStringList();
    }

    VectorMapStringString getTrustRequests(const QString& accountId)
    {
        Q_UNUSED(accountId)
        return VectorMapStringString();
    }

    bool acceptTrustRequest(const QString& accountId, const QString& from)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(from)
        return false;
    }

    bool discardTrustRequest(const QString& accountId, const QString& from)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(from)
        return false;
    }

    void sendTrustRequest(const QString& accountId, const QString& from, const QByteArray& payload)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(from)
        Q_UNUSED(payload)
    }

    void removeContact(const QString &accountId, const QString &uri, bool ban)
    {
        if (getAccountList().indexOf(accountId) == -1) return;
        auto contacts = accountToContactsMap[accountId];
        for (auto c = 0 ; c < contacts.size() ; ++c) {
            if (contacts.at(c)["id"] == uri) {
                contacts.remove(c);
                emit contactRemoved(accountId, uri, ban);
                return;
            }
        }
    }

    void addContact(const QString &accountId, const QString &uri)
    {
        if (getAccountList().indexOf(accountId) == -1) return;
        auto contact = QMap<QString, QString>();
        contact.insert("id", uri);
        contact.insert("added", "true");
        contact.insert("removed", "false");
        contact.insert("confirmed", "true");
        contact.insert("banned", "false");
        accountToContactsMap[accountId].push_back(contact);
        emit contactAdded(accountId, uri, true);
    }

    uint64_t sendTextMessage(const QString& accountId, const QString& to, const QMap<QString,QString>& payloads)
    {
        // NOTE used in ContactModel::sendMessage and ConversationModel::sendMessage
        Q_UNUSED(accountId)
        Q_UNUSED(to)
        Q_UNUSED(payloads)
        return 0;
    }

    bool setCodecDetails(const QString& accountId, unsigned int codecId, const MapStringString& details)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(codecId)
        Q_UNUSED(details)
        return false;
    }

    int getMessageStatus(uint64_t id)
    {
        Q_UNUSED(id)
        return 0;
    }

    void connectivityChanged()
    {
    }

    MapStringString getContactDetails(const QString &accountId, const QString &uri)
    {
        Q_UNUSED(accountId)
        Q_UNUSED(uri)
       return MapStringString();
    }

Q_SIGNALS: // SIGNALS
   void volumeChanged(const QString& device, double value);
   void accountsChanged();
   void historyChanged();
   void stunStatusFailure(const QString& reason);
   void registrationStateChanged(const QString& accountId, const QString& registration_state, unsigned detail_code, const QString& detail_str);
   void stunStatusSuccess(const QString& message);
   void errorAlert(int code);
   void volatileAccountDetailsChanged(const QString& accountId, MapStringString details);
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
   void registeredNameFound(const QString& accountId, int status, const QString& address, const QString& name); // used by conversationModel
   void migrationEnded(const QString &accountId, const QString &result);
   void contactAdded(const QString &accountId, const QString &uri, bool banned);
   void contactRemoved(const QString &accountId, const QString &uri, bool banned);


};

namespace org {
   namespace ring {
      namespace Ring {
         typedef ::ConfigurationManagerInterface ConfigurationManager;
      }
   }
}

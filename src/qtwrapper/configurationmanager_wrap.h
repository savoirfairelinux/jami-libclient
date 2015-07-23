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
#ifndef CONFIGURATIONMANAGERINTERFACE_H
#define CONFIGURATIONMANAGERINTERFACE_H

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
                           Q_EMIT this->volumeChanged(QString(device.c_str()), value);
                     });
         }),
         exportable_callback<ConfigurationSignal::AccountsChanged>(
               [this] () {
                     QTimer::singleShot(0, [this] {
                           Q_EMIT this->accountsChanged();
                     });
            }),
         exportable_callback<ConfigurationSignal::StunStatusFailed>(
               [this] (const std::string &reason) {
                     QTimer::singleShot(0, [this, reason] {
                           Q_EMIT this->stunStatusFailure(QString(reason.c_str()));
                     });
         }),
         exportable_callback<ConfigurationSignal::RegistrationStateChanged>(
               [this] (const std::string &accountID, const std::string& registration_state, unsigned detail_code, const std::string& detail_str) {
                     QTimer::singleShot(0, [this, accountID, registration_state, detail_code, detail_str] {
                           Q_EMIT this->registrationStateChanged(QString(accountID.c_str()),
                                                               QString(registration_state.c_str()),
                                                               detail_code,
                                                               QString(detail_str.c_str()));
                     });
         }),
         exportable_callback<ConfigurationSignal::VolatileDetailsChanged>(
               [this] (const std::string &accountID, const std::map<std::string, std::string>& details) {
                     QTimer::singleShot(0, [this, accountID, details] {
                        Q_EMIT this->volatileAccountDetailsChanged(QString(accountID.c_str()), convertMap(details));
                     });
         }),
         exportable_callback<ConfigurationSignal::Error>(
               [this] (int code) {
                     QTimer::singleShot(0, [this,code] {
                        Q_EMIT this->errorAlert(code);
                     });
         }),
         exportable_callback<ConfigurationSignal::CertificateExpired>(
               [this] (const std::string &certId) {
                     QTimer::singleShot(0, [this, certId] {
                           Q_EMIT this->certificateExpired(QString(certId.c_str()));
                     });
         }),

         exportable_callback<ConfigurationSignal::CertificatePinned>(
               [this] (const std::string &certId) {
                     QTimer::singleShot(0, [this, certId] {
                           Q_EMIT this->certificatePinned(QString(certId.c_str()));
                     });
         }),

         exportable_callback<ConfigurationSignal::CertificatePathPinned>(
               [this] (const std::string &certPath, const std::vector<std::string>& list) {
                     QTimer::singleShot(0, [this, certPath, list] {
                           Q_EMIT this->certificatePathPinned(QString(certPath.c_str()),convertStringList(list));
                     });
         }),

         exportable_callback<ConfigurationSignal::IncomingTrustRequest>(
               [this] (const std::string &accountId, const std::string &certId, const std::vector<uint8_t> &payload, time_t timestamp) {
                     QTimer::singleShot(0, [this, certId,accountId,payload,timestamp] {
                           Q_EMIT this->incomingTrustRequest(QString(accountId.c_str()), QString(certId.c_str()), QByteArray(reinterpret_cast<const char*>(payload.data()), payload.size()), timestamp);
                     });
         }),

         exportable_callback<ConfigurationSignal::IncomingAccountMessage>(
               [this] (const std::string& account_id, const std::string& from, const std::string& message) {
                     QTimer::singleShot(0, [this, account_id,from,message] {
                           Q_EMIT this->incomingAccountMessage(QString(account_id.c_str()), QString(from.c_str()), QString(message.c_str()));
                     });
         }),

         exportable_callback<ConfigurationSignal::MediaParametersChanged>(
               [this] (const std::string& account_id) {
                     QTimer::singleShot(0, [this, account_id] {
                           Q_EMIT this->mediaParametersChanged(QString(account_id.c_str()));
                     });
         }),
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

   MapStringString getAccountDetails(const QString& accountID)
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
   VectorUInt getActiveCodecList(const QString& accountID)
   {
      return QVector<unsigned int>::fromStdVector(
         DRing::getActiveCodecList(accountID.toStdString()));
   }

   QString getAddrFromInterfaceName(const QString& interface)
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

   MapStringString getCodecDetails(const QString& accountID, int payload)
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

   int getAudioInputDeviceIndex(const QString& devname)
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

   int getAudioOutputDeviceIndex(const QString& devname)
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

   VectorMapStringString getCredentials(const QString& accountID)
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

   MapStringString validateCertificate(const QString& unused, const QString& certificate)
   {
      MapStringString temp =
         convertMap(DRing::validateCertificate(unused.toStdString(),
                                             certificate.toStdString()));
      return temp;
   }

   MapStringString validateCertificatePath(const QString& unused, const QString& certificate, const QString& privateKey, const QString& privateKeyPass, const QString& caListPath)
   {
      MapStringString temp =
         convertMap(DRing::validateCertificatePath(unused.toStdString(),
                                             certificate.toStdString(),
                                             privateKey.toStdString(),
                                             privateKeyPass.toStdString(),
                                             caListPath.toStdString()));
      return temp;
   }

   MapStringString getCertificateDetails(const QString& certificate)
   {
      MapStringString temp =
         convertMap(DRing::getCertificateDetails(certificate.toStdString()));
      return temp;
   }

   MapStringString getCertificateDetailsPath(const QString& certificate, const QString& privateKey, const QString& privateKeyPass)
   {
      MapStringString temp =
         convertMap(DRing::getCertificateDetailsPath(certificate.toStdString(),
                                                     privateKey.toStdString(),
                                                     privateKeyPass.toStdString()));
      return temp;
   }

   QStringList getSupportedCiphers(const QString& accountID)
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

   double getVolume(const QString& device)
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

   void removeAccount(const QString& accountID)
   {
      DRing::removeAccount(accountID.toStdString());
   }

   void sendRegister(const QString& accountID, bool enable)
   {
      DRing::sendRegister(accountID.toStdString(), enable);
   }

   void setAccountDetails(const QString& accountID, MapStringString details)
   {
      DRing::setAccountDetails(accountID.toStdString(),
         convertMap(details));
   }

   void setAccountsOrder(const QString& order)
   {
      DRing::setAccountsOrder(order.toStdString());
   }

   void setActiveCodecList(const QString& accountID, VectorUInt &list)
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

   bool setAudioManager(const QString& api)
   {
      return DRing::setAudioManager(api.toStdString());
   }

   void setAudioOutputDevice(int index)
   {
      DRing::setAudioOutputDevice(index);
   }

   void setAudioPlugin(const QString& audioPlugin)
   {
      DRing::setAudioPlugin(audioPlugin.toStdString());
   }

   void setAudioRingtoneDevice(int index)
   {
      DRing::setAudioRingtoneDevice(index);
   }

   void setCredentials(const QString& accountID, VectorMapStringString credentialInformation)
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

   void setRecordPath(const QString& rec)
   {
      DRing::setRecordPath(rec.toStdString());
   }

   void setShortcuts(MapStringString shortcutsMap)
   {
      DRing::setShortcuts(convertMap(shortcutsMap));
   }

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
      QStringList temp =
         convertStringList(DRing::getPinnedCertificates());
      return temp;
   }

   QStringList pinCertificate(const QByteArray& content, bool local)
   {
      std::vector<unsigned char> raw(content.begin(), content.end());
      return convertStringList(DRing::pinCertificate(raw,local));
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

   bool setCertificateStatus(const QString& accountId, const QString& certPath, const QString& status)
   {
      return DRing::setCertificateStatus(accountId.toStdString(), certPath.toStdString(), status.toStdString());
   }

   QStringList getCertificatesByStatus(const QString& accountId, const QString& status)
   {
      return convertStringList(DRing::getCertificatesByStatus(accountId.toStdString(), status.toStdString()));
   }

   MapStringString getTrustRequests(const QString& accountId)
   {
      return convertMap(DRing::getTrustRequests(accountId.toStdString()));
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

   void sendTextMessage(const QString& accountId, const QString& to, const QString& message)
   {
      DRing::sendAccountTextMessage(accountId.toStdString(), to.toStdString(), message.toStdString());
   }

   bool setCodecDetails(const QString& accountId, unsigned int codecId, const MapStringString& details)
   {
      DRing::setCodecDetails(accountId.toStdString(), codecId, convertMap(details));
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
   void incomingTrustRequest(const QString& accountId, const QString& from, const QByteArray& payload, qulonglong timeStamp);
   void incomingAccountMessage(const QString& accountId, const QString& from, const QString& message);
   void mediaParametersChanged(const QString& accountId);

};

namespace org {
   namespace ring {
      namespace Ring {
         typedef ::ConfigurationManagerInterface ConfigurationManager;
      }
   }
}
#endif

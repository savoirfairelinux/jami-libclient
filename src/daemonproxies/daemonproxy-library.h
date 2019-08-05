/*
 *  Copyright (C) 2004-2019 Savoir-faire Linux Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#pragma once
#include <sigc++/sigc++.h>
#include <callmanager_interface.h>
#include <configurationmanager_interface.h>
#include <datatransfer_interface.h>
#include <presencemanager_interface.h>
#ifdef ENABLE_VIDEO
    #include <videomanager_interface.h>
#endif

using namespace DRing;

class DaemonProxy
{
public:
    static DaemonProxy& instance() {
        static DaemonProxy daemon;
        return daemon;
    }

    DaemonProxy();
    ~DaemonProxy();

    // wrappers for callmanager_interface.h
    std::string placeCall(const std::string& accountID, const std::string& to, const std::map<std::string, std::string>& VolatileCallDetails = {});
    bool refuse(const std::string& callID);
    bool accept(const std::string& callID);
    bool hangUp(const std::string& callID);
    bool hold(const std::string& callID);
    bool unhold(const std::string& callID);
    bool muteLocalMedia(const std::string& callid, const std::string& mediaType, bool mute);
    bool transfer(const std::string& callID, const std::string& to);
    bool attendedTransfer(const std::string& transferID, const std::string& targetID);
    std::map<std::string, std::string> getCallDetails(const std::string& callID);
    std::vector<std::string> getCallList();
    void removeConference(const std::string& conference_id);
    bool joinParticipant(const std::string& sel_callID, const std::string& drag_callID);
    void createConfFromParticipantList(const std::vector<std::string>& participants);
    bool isConferenceParticipant(const std::string& call_id);
    bool addParticipant(const std::string& callID, const std::string& confID);
    bool addMainParticipant(const std::string& confID);
    bool detachLocalParticipant();
    bool detachParticipant(const std::string& callID);
    bool joinConference(const std::string& sel_confID, const std::string& drag_confID);
    bool hangUpConference(const std::string& confID);
    bool holdConference(const std::string& confID);
    bool unholdConference(const std::string& confID);
    std::vector<std::string> getConferenceList();
    std::vector<std::string> getParticipantList(const std::string& confID);
    std::vector<std::string> getDisplayNames(const std::string& confID);
    std::string getConferenceId(const std::string& callID);
    std::map<std::string, std::string> getConferenceDetails(const std::string& callID);
    void startSmartInfo(uint32_t refreshTimeMs);
    void stopSmartInfo();
    bool startRecordedFilePlayback(const std::string& filepath);
    void stopRecordedFilePlayback();
    bool toggleRecording(const std::string& callID);
    void setRecording(const std::string& callID);
    void recordPlaybackSeek(double value);
    bool getIsRecording(const std::string& callID);
    std::string getCurrentAudioCodecName(const std::string& callID);
    void playDTMF(const std::string& key);
    void startTone(int32_t start, int32_t type);
    bool switchInput(const std::string& callID, const std::string& resource);
    void sendTextMessage(const std::string& callID, const std::map<std::string, std::string>& messages, const std::string& from, bool isMixed);

    // wrappers for configurationmanager_interface.h
    std::map<std::string, std::string> getAccountDetails(const std::string& accountID);
    std::map<std::string, std::string> getVolatileAccountDetails(const std::string& accountID);
    void setAccountDetails(const std::string& accountID, const std::map<std::string, std::string>& details);
    std::map<std::string, std::string> testAccountICEInitialization(const std::string& accountID);
    void setAccountActive(const std::string& accountID, bool active);
    std::map<std::string, std::string> getAccountTemplate(const std::string& accountType);
    std::string addAccount(const std::map<std::string, std::string>& details);
    bool exportOnRing(const std::string& accountID, const std::string& password);
    bool exportToFile(const std::string& accountID, const std::string& destinationPath, const std::string& password = {});
    bool revokeDevice(const std::string& accountID, const std::string& password, const std::string& deviceID);
    std::map<std::string, std::string> getKnownRingDevices(const std::string& accountID);
    bool changeAccountPassword(const std::string& accountID, const std::string& password_old, const std::string& password_new);
    bool lookupName(const std::string& account, const std::string& nameserver, const std::string& name);
    bool lookupAddress(const std::string& account, const std::string& nameserver, const std::string& address);
    bool registerName(const std::string& account, const std::string& password, const std::string& name);
    void removeAccount(const std::string& accountID);
    std::vector<std::string> getAccountList();
    void sendRegister(const std::string& accountID, bool enable);
    void registerAllAccounts(void);
    uint64_t sendAccountTextMessage(const std::string& accountID, const std::string& to, const std::map<std::string, std::string>& payloads);
    bool cancelMessage(const std::string& accountID, uint64_t message);
    std::vector<Message> getLastMessages(const std::string& accountID, const uint64_t& base_timestamp);
    std::map<std::string, std::string> getNearbyPeers(const std::string& accountID);
    int getMessageStatus(uint64_t id);
    int getMessageStatus(const std::string& accountID, uint64_t id);
    std::map<std::string, std::string> getTlsDefaultSettings();
    std::vector<unsigned> getCodecList();
    std::vector<std::string> getSupportedTlsMethod();
    std::vector<std::string> getSupportedCiphers(const std::string& accountID);
    std::map<std::string, std::string> getCodecDetails(const std::string& accountID, const unsigned& codecId);
    bool setCodecDetails(const std::string& accountID, const unsigned& codecId, const std::map<std::string, std::string>& details);
    std::vector<unsigned> getActiveCodecList(const std::string& accountID);
    void setActiveCodecList(const std::string& accountID, const std::vector<unsigned>& list);
    std::vector<std::string> getAudioPluginList();
    void setAudioPlugin(const std::string& audioPlugin);
    std::vector<std::string> getAudioOutputDeviceList();
    void setAudioOutputDevice(int32_t index);
    void setAudioInputDevice(int32_t index);
    void setAudioRingtoneDevice(int32_t index);
    std::vector<std::string> getAudioInputDeviceList();
    std::vector<std::string> getCurrentAudioDevicesIndex();
    int32_t getAudioInputDeviceIndex(const std::string& name);
    int32_t getAudioOutputDeviceIndex(const std::string& name);
    std::string getCurrentAudioOutputPlugin();
    bool getNoiseSuppressState();
    void setNoiseSuppressState(bool state);
    bool isAgcEnabled();
    void setAgcState(bool enabled);
    void muteDtmf(bool mute);
    bool isDtmfMuted();
    bool isCaptureMuted();
    void muteCapture(bool mute);
    bool isPlaybackMuted();
    void mutePlayback(bool mute);
    bool isRingtoneMuted();
    void muteRingtone(bool mute);
    std::vector<std::string> getSupportedAudioManagers();
    std::string getAudioManager();
    bool setAudioManager(const std::string& api);
    std::string getRecordPath();
    void setRecordPath(const std::string& recPath);
    bool getIsAlwaysRecording();
    void setIsAlwaysRecording(bool rec);
    bool getRecordPreview();
    void setRecordPreview(bool rec);
    int getRecordQuality();
    void setRecordQuality(int quality);
    void setHistoryLimit(int32_t days);
    int32_t getHistoryLimit();
    void setRingingTimeout(int32_t timeout);
    int32_t getRingingTimeout();
    void setAccountsOrder(const std::string& order);
    std::map<std::string, std::string> getHookSettings();
    void setHookSettings(const std::map<std::string, std::string>& settings);
    std::vector<std::map<std::string, std::string>> getCredentials(const std::string& accountID);
    void setCredentials(const std::string& accountID, const std::vector<std::map<std::string, std::string>>& details);
    std::string getAddrFromInterfaceName(const std::string& iface);
    std::vector<std::string> getAllIpInterface();
    std::vector<std::string> getAllIpInterfaceByName();
    std::map<std::string, std::string> getShortcuts();
    void setShortcuts(const std::map<std::string, std::string> &shortcutsMap);
    void setVolume(const std::string& device, double value);
    double getVolume(const std::string& device);
    std::map<std::string, std::string> validateCertificate(const std::string& accountId, const std::string& certificate);
    std::map<std::string, std::string> validateCertificatePath(const std::string& accountId, const std::string& certificatePath, const std::string& privateKey, const std::string& privateKeyPassword, const std::string& caList);
    std::map<std::string, std::string> getCertificateDetails(const std::string& certificate);
    std::map<std::string, std::string> getCertificateDetailsPath(const std::string& certificatePath, const std::string& privateKey, const std::string& privateKeyPassword);
    std::vector<std::string> getPinnedCertificates();
    std::vector<std::string> pinCertificate(const std::vector<uint8_t>& certificate, bool local);
    bool unpinCertificate(const std::string& certId);
    void pinCertificatePath(const std::string& path);
    unsigned unpinCertificatePath(const std::string& path);
    bool pinRemoteCertificate(const std::string& accountId, const std::string& certId);
    bool setCertificateStatus(const std::string& account, const std::string& certId, const std::string& status);
    std::vector<std::string> getCertificatesByStatus(const std::string& account, const std::string& status);
    std::vector<std::map<std::string, std::string>> getTrustRequests(const std::string& accountId);
    bool acceptTrustRequest(const std::string& accountId, const std::string& from);
    bool discardTrustRequest(const std::string& accountId, const std::string& from);
    void sendTrustRequest(const std::string& accountId, const std::string& to, const std::vector<uint8_t>& payload = {});
    void addContact(const std::string& accountId, const std::string& uri);
    void removeContact(const std::string& accountId, const std::string& uri, bool ban);
    std::map<std::string, std::string> getContactDetails(const std::string& accountId, const std::string& uri);
    std::vector<std::map<std::string, std::string>> getContacts(const std::string& accountId);
    int exportAccounts(const std::vector<std::string>& accountIDs, const std::string& filepath, const std::string& password);
    int importAccounts(const std::string& archivePath, const std::string& password);
    void connectivityChanged();
    void enableProxyClient(const std::string& accountID, bool enable);
    void setPushNotificationToken(const std::string& pushDeviceToken);
    void pushNotificationReceived(const std::string& from, const std::map<std::string, std::string>& data);
    bool isAudioMeterActive(const std::string& id);
    void setAudioMeterState(const std::string& id, bool state);

    // wrappers for datatransfer_interface.h
    std::vector<DataTransferId> dataTransferList() noexcept;
    DataTransferError sendFile(const DataTransferInfo& info, DataTransferId& id) noexcept;
    DataTransferError acceptFileTransfer(const DataTransferId& id, const std::string& file_path, int64_t offset) noexcept;
    DataTransferError cancelDataTransfer(const DataTransferId& id) noexcept;
    DataTransferError dataTransferInfo(const DataTransferId& id, DataTransferInfo& info) noexcept;
    DataTransferError dataTransferBytesProgress(const DataTransferId& id, int64_t& total, int64_t& progress) noexcept;

    // wrappers for presencemanager_interface.h
    void publish(const std::string& accountID, bool status, const std::string& note);
    void answerServerRequest(const std::string& uri, bool flag);
    void subscribeBuddy(const std::string& accountID, const std::string& uri, bool flag);
    std::vector<std::map<std::string, std::string>> getSubscriptions(const std::string& accountID);
    void setSubscriptions(const std::string& accountID, const std::vector<std::string>& uris);

#ifdef ENABLE_VIDEO
    // wrappers for videomanager_interface.h
    std::vector<std::string> getDeviceList();
    VideoCapabilities getCapabilities(const std::string& name);
    std::map<std::string, std::string> getSettings(const std::string& name);
    void applySettings(const std::string& name, const std::map<std::string, std::string>& settings);
    void setDefaultDevice(const std::string& name);
    void setDeviceOrientation(const std::string& name, int angle);
    std::map<std::string, std::string> getDeviceParams(const std::string& name);
    std::string getDefaultDevice();
    void startCamera();
    void stopCamera();
    bool hasCameraStarted();
    void startAudioDevice();
    void stopAudioDevice();
    bool switchInput(const std::string& resource);
    bool switchToCamera();
    void registerSinkTarget(const std::string& sinkId, const SinkTarget& target);
    void registerAVSinkTarget(const std::string& sinkId, const AVSinkTarget& target);
    std::map<std::string, std::string> getRenderer(const std::string& callId);
    std::string startLocalRecorder(const bool& audioOnly, const std::string& filepath);
    void stopLocalRecorder(const std::string& filepath);
    bool getDecodingAccelerated();
    void setDecodingAccelerated(bool state);
    bool getEncodingAccelerated();
    void setEncodingAccelerated(bool state);
#endif

#include "daemonproxysignals.h"
};

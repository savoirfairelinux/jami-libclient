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

#include "daemonproxy-library.h"
#include <dring.h>
#include <callmanager_interface.h>
#include <configurationmanager_interface.h>
#include <datatransfer_interface.h>
#include <presencemanager_interface.h>
#ifdef ENABLE_VIDEO
    #include <videomanager_interface.h>
#endif

DaemonProxy::DaemonProxy()
{
    using namespace std::placeholders;
    using std::bind;
    using DRing::exportable_callback;
    using DRing::CallSignal;
    using DRing::ConfigurationSignal;
    using DRing::DataTransferSignal;
    using DRing::PresenceSignal;
    using DRing::AudioSignal;
#ifdef ENABLE_VIDEO
    using DRing::VideoSignal;
#endif
    using SharedCallback = std::shared_ptr<DRing::CallbackWrapperBase>;

    // Call event handlers
    const std::map<std::string, SharedCallback> callEvHandlers = {
        exportable_callback<CallSignal::AudioMuted>(bind(&sigc::signal<void, const std::string&, const bool&>::emit, &this->mSignalAudioMuted, _1, _2)),
        exportable_callback<CallSignal::ConferenceChanged>(bind(&sigc::signal<void, const std::string&, const std::string&>::emit, &this->mSignalConferenceChanged, _1, _2)),
        exportable_callback<CallSignal::ConferenceCreated>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalConferenceCreated, _1)),
        exportable_callback<CallSignal::ConferenceRemoved>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalConferenceRemoved, _1)),
        exportable_callback<CallSignal::IncomingCall>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::string&>::emit, &this->mSignalIncomingCall, _1, _2, _3)),
        exportable_callback<CallSignal::IncomingMessage>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&>::emit, &this->mSignalIncomingMessage, _1, _2, _3)),
        exportable_callback<CallSignal::NewCallCreated>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::string&>::emit, &this->mSignalNewCallCreated, _1, _2, _3)),
        exportable_callback<CallSignal::PeerHold>(bind(&sigc::signal<void, const std::string&, const bool&>::emit, &this->mSignalPeerHold, _1, _2)),
        exportable_callback<CallSignal::RecordingStateChanged>(bind(&sigc::signal<void, const std::string&, const bool&>::emit, &this->mSignalRecordingStateChanged, _1, _2)),
        exportable_callback<CallSignal::RecordPlaybackFilepath>(bind(&sigc::signal<void, const std::string&, const std::string&>::emit, &this->mSignalRecordPlaybackFilepath, _1, _2)),
        exportable_callback<CallSignal::RecordPlaybackStopped>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalRecordPlaybackStopped, _1)),
        exportable_callback<CallSignal::RtcpReportReceived>(bind(&sigc::signal<void, const std::string&, const std::map<std::string, int32_t>&>::emit, &this->mSignalOnRtcpReportReceived, _1, _2)),
        exportable_callback<CallSignal::SecureSdesOff>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalSecureSdesOff, _1)),
        exportable_callback<CallSignal::SecureSdesOn>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalSecureSdesOn, _1)),
        exportable_callback<CallSignal::SmartInfo>(bind(&sigc::signal<void, const std::map<std::string, std::string>&>::emit, &this->mSignalSmartInfo, _1)),
        exportable_callback<CallSignal::StateChange>(bind(&sigc::signal<void, const std::string&, const std::string&, const int32_t&>::emit, &this->mSignalCallStateChanged, _1, _2, _3)),
        exportable_callback<CallSignal::TransferFailed>(bind(&sigc::signal<void>::emit, &this->mSignalTransferFailed)),
        exportable_callback<CallSignal::TransferSucceeded>(bind(&sigc::signal<void>::emit, &this->mSignalTransferSucceeded)),
        exportable_callback<CallSignal::UpdatePlaybackScale>(bind(&sigc::signal<void, const std::string&, const int32_t&, const int32_t&>::emit, &this->mSignalUpdatePlaybackScale, _1, _2, _3)),
        exportable_callback<CallSignal::VideoMuted>(bind(&sigc::signal<void, const std::string&, const bool&>::emit, &this->mSignalVideoMuted, _1, _2)),
        exportable_callback<CallSignal::VoiceMailNotify>(bind(&sigc::signal<void, const std::string&, const int32_t&>::emit, &this->mSignalVoiceMailNotify, _1, _2)),
    };

    // Configuration event handlers
    const std::map<std::string, SharedCallback> configEvHandlers = {
        exportable_callback<ConfigurationSignal::AccountDetailsChanged>(bind(&sigc::signal<void, const std::string&, const std::map<std::string, std::string>&>::emit, &this->mSignalAccountDetailsChanged, _1, _2)),
        exportable_callback<ConfigurationSignal::AccountMessageStatusChanged>(bind(&sigc::signal<void, const std::string&, const uint64_t&, const std::string&, const int32_t&>::emit, &this->mSignalAccountMessageStatusChanged, _1, _2, _3, _4)),
        exportable_callback<ConfigurationSignal::AccountsChanged>(bind(&sigc::signal<void>::emit, &this->mSignalAccountsChanged)),
        exportable_callback<ConfigurationSignal::CertificateExpired>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalCertificateExpired, _1)),
        exportable_callback<ConfigurationSignal::CertificatePathPinned>(bind(&sigc::signal<void, const std::string&, const std::vector<std::string>&>::emit, &this->mSignalCertificatePathPinned, _1, _2)),
        exportable_callback<ConfigurationSignal::CertificatePinned>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalCertificatePinned, _1)),
        exportable_callback<ConfigurationSignal::CertificateStateChanged>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::string&>::emit, &this->mSignalCertificateStateChanged, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::ContactAdded>(bind(&sigc::signal<void, const std::string&, const std::string&, const bool&>::emit, &this->mSignalContactAdded, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::ContactRemoved>(bind(&sigc::signal<void, const std::string&, const std::string&, const bool&>::emit, &this->mSignalContactRemoved, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::DeviceRevocationEnded>(bind(&sigc::signal<void, const std::string&, const std::string&, const int32_t&>::emit, &this->mSignalDeviceRevocationEnded, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::Error>(bind(&sigc::signal<void, const int32_t&>::emit, &this->mSignalErrorAlert, _1)),
        exportable_callback<ConfigurationSignal::ExportOnRingEnded>(bind(&sigc::signal<void, const std::string&, const int32_t&, const std::string&>::emit, &this->mSignalExportOnRingEnded, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::HardwareDecodingChanged>(bind(&sigc::signal<void, const bool&>::emit, &this->mSignalHardwareDecodingChanged, _1)),
        exportable_callback<ConfigurationSignal::HardwareEncodingChanged>(bind(&sigc::signal<void, const bool&>::emit, &this->mSignalHardwareEncodingChanged, _1)),
        exportable_callback<ConfigurationSignal::IncomingAccountMessage>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&>::emit, &this->mSignalIncomingAccountMessage, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::IncomingTrustRequest>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::vector<uint8_t>&, const uint64_t&>::emit, &this->mSignalIncomingTrustRequest, _1, _2, _3, _4)),
        exportable_callback<ConfigurationSignal::KnownDevicesChanged>(bind(&sigc::signal<void, const std::string&, const std::map<std::string, std::string>&>::emit, &this->mSignalKnownDevicesChanged, _1, _2 )),
        exportable_callback<ConfigurationSignal::MediaParametersChanged>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalMediaParametersChanged, _1)),
        exportable_callback<ConfigurationSignal::MigrationEnded>(bind(&sigc::signal<void, const std::string&, const std::string&>::emit, &this->mSignalMigrationEnded, _1, _2)),
        exportable_callback<ConfigurationSignal::NameRegistrationEnded>(bind(&sigc::signal<void, const std::string&, const int32_t&, const std::string&>::emit, &this->mSignalNameRegistrationEnded, _1, _2, _3)),
        exportable_callback<ConfigurationSignal::RegisteredNameFound>(bind(&sigc::signal<void, const std::string&, const int32_t&, const std::string&, const std::string&>::emit, &this->mSignalRegisteredNameFound, _1, _2, _3, _4)),
        exportable_callback<ConfigurationSignal::RegistrationStateChanged>(bind(&sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&>::emit, &this->mSignalRegistrationStateChanged, _1, _2, _3, _4)),
        exportable_callback<ConfigurationSignal::StunStatusFailed>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalStunStatusFailure, _1)),
        exportable_callback<ConfigurationSignal::VolatileDetailsChanged>(bind(&sigc::signal<void, const std::string&, const std::map<std::string, std::string>&>::emit, &this->mSignalVolatileAccountDetailsChanged, _1, _2)),
        exportable_callback<ConfigurationSignal::VolumeChanged>(bind(&sigc::signal<void, const std::string&, const double&>::emit, &this->mSignalVolumeChanged, _1, _2)),
    };

    // Presence event handlers
    const std::map<std::string, SharedCallback> presEvHandlers = {
        exportable_callback<PresenceSignal::NearbyPeerNotification>(bind(&sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&>::emit, &this->mSignalNearbyPeerNotification, _1, _2, _3, _4)),
        exportable_callback<PresenceSignal::NewBuddyNotification>(bind(&sigc::signal<void, const std::string&, const std::string&, const bool&, const std::string&>::emit, &this->mSignalNewBuddyNotification, _1, _2, _3, _4)),
        exportable_callback<PresenceSignal::NewServerSubscriptionRequest>(bind(&sigc::signal<void, const std::string&>::emit, &this->mSignalNewServerSubscriptionRequest, _1)),
        exportable_callback<PresenceSignal::ServerError>(bind(&sigc::signal<void, const std::string&, const std::string&, const std::string&>::emit, &this->mSignalServerError, _1, _2, _3)),
        exportable_callback<PresenceSignal::SubscriptionStateChanged>(bind(&sigc::signal<void, const std::string&, const std::string&, const bool&>::emit, &this->mSignalSubscriptionStateChanged, _1, _2, _3)),
    };

    // Audio event handlers
    const std::map<std::string, SharedCallback> audioEvHandlers = {
        exportable_callback<AudioSignal::AudioMeter>(bind(&sigc::signal<void, const std::string&, const double&>::emit, &this->mSignalAudioMeter, _1, _2)),
        exportable_callback<AudioSignal::DeviceEvent>(bind(&sigc::signal<void>::emit, &this->mSignalAudioDeviceEvent)),
    };

    const std::map<std::string, SharedCallback> dataXferEvHandlers = {
        exportable_callback<DataTransferSignal::DataTransferEvent>(bind(&sigc::signal<void, const uint64_t&, const int32_t&>::emit, &this->mSignalDataTransferEvent, _1, _2)),
    };

#ifdef ENABLE_VIDEO
    // Video event handlers
    const std::map<std::string, SharedCallback> videoEvHandlers = {
        exportable_callback<VideoSignal::DecodingStarted>(bind(&sigc::signal<void, const std::string&, const std::string&, const int32_t&, const int32_t&, const bool&>::emit, &this->mSignalStartedDecoding, _1, _2, _3, _4, _5)),
        exportable_callback<VideoSignal::DecodingStopped>(bind(&sigc::signal<void, const std::string&, const std::string&, const bool&>::emit, &this->mSignalStoppedDecoding, _1, _2, _3)),
        exportable_callback<VideoSignal::DeviceEvent>(bind(&sigc::signal<void>::emit, &this->mSignalDeviceEvent)),
    };
#endif

#ifdef DAEMON_MUTING_IS_ON
    auto flags = 0;
#else
    auto flags = DRing::DRING_FLAG_DEBUG | DRing::DRING_FLAG_CONSOLE_LOG;
#endif
    if (!DRing::init(static_cast<DRing::InitFlag>(flags)))
        throw std::runtime_error("DRing::init() failed.");

    DRing::registerSignalHandlers(callEvHandlers);
    DRing::registerSignalHandlers(configEvHandlers);
    DRing::registerSignalHandlers(presEvHandlers);
    DRing::registerSignalHandlers(audioEvHandlers);
    DRing::registerSignalHandlers(dataXferEvHandlers);
#ifdef ENABLE_VIDEO
    DRing::registerSignalHandlers(videoEvHandlers);
#endif

    if (!DRing::start())
        throw std::runtime_error("DRing::start() failed.\n");

}

DaemonProxy::~DaemonProxy()
{
    DRing::unregisterSignalHandlers();
    DRing::fini();
}

// wrappers for callmanager_interface.h
std::string DaemonProxy::placeCall(const std::string& accountID, const std::string& to, const std::map<std::string, std::string>& VolatileCallDetails) { return DRing::placeCall(accountID, to, VolatileCallDetails); }
bool DaemonProxy::refuse(const std::string& callID) { return DRing::refuse(callID); }
bool DaemonProxy::accept(const std::string& callID) { return DRing::accept(callID); }
bool DaemonProxy::hangUp(const std::string& callID) { return DRing::hangUp(callID); }
bool DaemonProxy::hold(const std::string& callID) { return DRing::hold(callID); }
bool DaemonProxy::unhold(const std::string& callID) { return DRing::unhold(callID); }
bool DaemonProxy::muteLocalMedia(const std::string& callid, const std::string& mediaType, bool mute) { return DRing::muteLocalMedia(callid, mediaType, mute); }
bool DaemonProxy::transfer(const std::string& callID, const std::string& to) { return DRing::transfer(callID, to); }
bool DaemonProxy::attendedTransfer(const std::string& transferID, const std::string& targetID) { return DRing::attendedTransfer(transferID, targetID); }
std::map<std::string, std::string> DaemonProxy::getCallDetails(const std::string& callID) { return DRing::getCallDetails(callID); }
std::vector<std::string> DaemonProxy::getCallList() { return DRing::getCallList(); }
void DaemonProxy::removeConference(const std::string& conference_id) { DRing::removeConference(conference_id); }
bool DaemonProxy::joinParticipant(const std::string& sel_callID, const std::string& drag_callID) { return DRing::joinParticipant(sel_callID, drag_callID); }
void DaemonProxy::createConfFromParticipantList(const std::vector<std::string>& participants) { DRing::createConfFromParticipantList(participants); }
bool DaemonProxy::isConferenceParticipant(const std::string& call_id) { return DRing::isConferenceParticipant(call_id); }
bool DaemonProxy::addParticipant(const std::string& callID, const std::string& confID) { return DRing::addParticipant(callID, confID); }
bool DaemonProxy::addMainParticipant(const std::string& confID) { return DRing::addMainParticipant(confID); }
bool DaemonProxy::detachLocalParticipant() { return DRing::detachLocalParticipant(); }
bool DaemonProxy::detachParticipant(const std::string& callID) { return DRing::detachParticipant(callID); }
bool DaemonProxy::joinConference(const std::string& sel_confID, const std::string& drag_confID) { return DRing::joinConference(sel_confID, drag_confID); }
bool DaemonProxy::hangUpConference(const std::string& confID) { return DRing::hangUpConference(confID); }
bool DaemonProxy::holdConference(const std::string& confID) { return DRing::holdConference(confID); }
bool DaemonProxy::unholdConference(const std::string& confID) { return DRing::unholdConference(confID); }
std::vector<std::string> DaemonProxy::getConferenceList() { return DRing::getConferenceList(); }
std::vector<std::string> DaemonProxy::getParticipantList(const std::string& confID) { return DRing::getParticipantList(confID); }
std::vector<std::string> DaemonProxy::getDisplayNames(const std::string& confID) { return DRing::getDisplayNames(confID); }
std::string DaemonProxy::getConferenceId(const std::string& callID) { return DRing::getConferenceId(callID); }
std::map<std::string, std::string> DaemonProxy::getConferenceDetails(const std::string& callID) { return DRing::getConferenceDetails(callID); }
void DaemonProxy::startSmartInfo(uint32_t refreshTimeMs) { DRing::startSmartInfo(refreshTimeMs); }
void DaemonProxy::stopSmartInfo() { DRing::stopSmartInfo(); }
bool DaemonProxy::startRecordedFilePlayback(const std::string& filepath) { return DRing::startRecordedFilePlayback(filepath); }
void DaemonProxy::stopRecordedFilePlayback() { DRing::stopRecordedFilePlayback(); }
bool DaemonProxy::toggleRecording(const std::string& callID) { return DRing::toggleRecording(callID); }
void DaemonProxy::setRecording(const std::string& callID) { DRing::setRecording(callID); }
void DaemonProxy::recordPlaybackSeek(double value) { DRing::recordPlaybackSeek(value); }
bool DaemonProxy::getIsRecording(const std::string& callID) { return DRing::getIsRecording(callID); }
std::string DaemonProxy::getCurrentAudioCodecName(const std::string& callID) { return DRing::getCurrentAudioCodecName(callID); }
void DaemonProxy::playDTMF(const std::string& key) { DRing::playDTMF(key); }
void DaemonProxy::startTone(int32_t start, int32_t type) { DRing::startTone(start, type); }
bool DaemonProxy::switchInput(const std::string& callID, const std::string& resource) { return DRing::switchInput(callID, resource); }
void DaemonProxy::sendTextMessage(const std::string& callID, const std::map<std::string, std::string>& messages, const std::string& from, bool isMixed) { DRing::sendTextMessage(callID, messages, from, isMixed); }

// wrappers for configurationmanager_interface.h
std::map<std::string, std::string> DaemonProxy::getAccountDetails(const std::string& accountID) { return DRing::getAccountDetails(accountID); }
std::map<std::string, std::string> DaemonProxy::getVolatileAccountDetails(const std::string& accountID) { return DRing::getVolatileAccountDetails(accountID); }
void DaemonProxy::setAccountDetails(const std::string& accountID, const std::map<std::string, std::string>& details) { DRing::setAccountDetails(accountID, details); }
std::map<std::string, std::string> DaemonProxy::testAccountICEInitialization(const std::string& accountID) { return DRing::testAccountICEInitialization(accountID); }
void DaemonProxy::setAccountActive(const std::string& accountID, bool active) { DRing::setAccountActive(accountID, active); }
std::map<std::string, std::string> DaemonProxy::getAccountTemplate(const std::string& accountType) { return DRing::getAccountTemplate(accountType); }
std::string DaemonProxy::addAccount(const std::map<std::string, std::string>& details) { return DRing::addAccount(details); }
bool DaemonProxy::exportOnRing(const std::string& accountID, const std::string& password) { return DRing::exportOnRing(accountID, password); }
bool DaemonProxy::exportToFile(const std::string& accountID, const std::string& destinationPath, const std::string& password) { return DRing::exportToFile(accountID, destinationPath, password); }
bool DaemonProxy::revokeDevice(const std::string& accountID, const std::string& password, const std::string& deviceID) { return DRing::revokeDevice(accountID, password, deviceID); }
std::map<std::string, std::string> DaemonProxy::getKnownRingDevices(const std::string& accountID) { return DRing::getKnownRingDevices(accountID); }
bool DaemonProxy::changeAccountPassword(const std::string& accountID, const std::string& password_old, const std::string& password_new) { return DRing::changeAccountPassword(accountID, password_old, password_new); }
bool DaemonProxy::lookupName(const std::string& account, const std::string& nameserver, const std::string& name) { return DRing::lookupName(account, nameserver, name); }
bool DaemonProxy::lookupAddress(const std::string& account, const std::string& nameserver, const std::string& address) { return DRing::lookupAddress(account, nameserver, address); }
bool DaemonProxy::registerName(const std::string& account, const std::string& password, const std::string& name) { return DRing::registerName(account, password, name); }
void DaemonProxy::removeAccount(const std::string& accountID) { DRing::removeAccount(accountID); }
std::vector<std::string> DaemonProxy::getAccountList() { return DRing::getAccountList(); }
void DaemonProxy::sendRegister(const std::string& accountID, bool enable) { DRing::sendRegister(accountID, enable); }
void DaemonProxy::registerAllAccounts() { return DRing::registerAllAccounts(); }
uint64_t DaemonProxy::sendAccountTextMessage(const std::string& accountID, const std::string& to, const std::map<std::string, std::string>& payloads) { return DRing::sendAccountTextMessage(accountID, to, payloads); }
bool DaemonProxy::cancelMessage(const std::string& accountID, uint64_t message) { return DRing::cancelMessage(accountID, message); }
std::vector<Message> DaemonProxy::getLastMessages(const std::string& accountID, const uint64_t& base_timestamp) { return DRing::getLastMessages(accountID, base_timestamp); }
std::map<std::string, std::string> DaemonProxy::getNearbyPeers(const std::string& accountID) { return DRing::getNearbyPeers(accountID); }
int DaemonProxy::getMessageStatus(uint64_t id) { return DRing::getMessageStatus(id); }
int DaemonProxy::getMessageStatus(const std::string& accountID, uint64_t id) { return DRing::getMessageStatus(accountID, id); }
std::map<std::string, std::string> DaemonProxy::getTlsDefaultSettings() { return DRing::getTlsDefaultSettings(); }
std::vector<unsigned> DaemonProxy::getCodecList() { return DRing::getCodecList(); }
std::vector<std::string> DaemonProxy::getSupportedTlsMethod() { return DRing::getSupportedTlsMethod(); }
std::vector<std::string> DaemonProxy::getSupportedCiphers(const std::string& accountID) { return DRing::getSupportedCiphers(accountID); }
std::map<std::string, std::string> DaemonProxy::getCodecDetails(const std::string& accountID, const unsigned& codecId) { return DRing::getCodecDetails(accountID, codecId); }
bool DaemonProxy::setCodecDetails(const std::string& accountID, const unsigned& codecId, const std::map<std::string, std::string>& details) { return DRing::setCodecDetails( accountID, codecId, details); }
std::vector<unsigned> DaemonProxy::getActiveCodecList(const std::string& accountID) { return DRing::getActiveCodecList(accountID); }
void DaemonProxy::setActiveCodecList(const std::string& accountID, const std::vector<unsigned>& list) { DRing::setActiveCodecList(accountID, list); }
std::vector<std::string> DaemonProxy::getAudioPluginList() { return DRing::getAudioPluginList(); }
void DaemonProxy::setAudioPlugin(const std::string& audioPlugin) { DRing::setAudioPlugin(audioPlugin); }
std::vector<std::string> DaemonProxy::getAudioOutputDeviceList() { return DRing::getAudioOutputDeviceList(); }
void DaemonProxy::setAudioOutputDevice(int32_t index) { DRing::setAudioOutputDevice(index); }
void DaemonProxy::setAudioInputDevice(int32_t index) { DRing::setAudioInputDevice(index); }
void DaemonProxy::setAudioRingtoneDevice(int32_t index) { DRing::setAudioRingtoneDevice(index); }
std::vector<std::string> DaemonProxy::getAudioInputDeviceList() { return DRing::getAudioInputDeviceList(); }
std::vector<std::string> DaemonProxy::getCurrentAudioDevicesIndex() { return DRing::getCurrentAudioDevicesIndex(); }
int32_t DaemonProxy::getAudioInputDeviceIndex(const std::string& name) { return DRing::getAudioInputDeviceIndex(name); }
int32_t DaemonProxy::getAudioOutputDeviceIndex(const std::string& name) { return DRing::getAudioOutputDeviceIndex(name); }
std::string DaemonProxy::getCurrentAudioOutputPlugin() { return DRing::getCurrentAudioOutputPlugin(); }
bool DaemonProxy::getNoiseSuppressState() { return DRing::getNoiseSuppressState(); }
void DaemonProxy::setNoiseSuppressState(bool state) { DRing::setNoiseSuppressState(state); }
bool DaemonProxy::isAgcEnabled() { return DRing::isAgcEnabled(); }
void DaemonProxy::setAgcState(bool enabled) { DRing::setAgcState(enabled); }
void DaemonProxy::muteDtmf(bool mute) { DRing::muteDtmf(mute); }
bool DaemonProxy::isDtmfMuted() { return DRing::isDtmfMuted(); }
bool DaemonProxy::isCaptureMuted() { return DRing::isCaptureMuted(); }
void DaemonProxy::muteCapture(bool mute) { DRing::muteCapture(mute); }
bool DaemonProxy::isPlaybackMuted() { return DRing::isPlaybackMuted(); }
void DaemonProxy::mutePlayback(bool mute) { DRing::mutePlayback(mute); }
bool DaemonProxy::isRingtoneMuted() { return DRing::isRingtoneMuted(); }
void DaemonProxy::muteRingtone(bool mute) { DRing::muteRingtone(mute); }
std::vector<std::string> DaemonProxy::getSupportedAudioManagers() { return DRing::getSupportedAudioManagers(); }
std::string DaemonProxy::getAudioManager() { return DRing::getAudioManager(); }
bool DaemonProxy::setAudioManager(const std::string& api) { return DRing::setAudioManager(api); }
std::string DaemonProxy::getRecordPath() { return DRing::getRecordPath(); }
void DaemonProxy::setRecordPath(const std::string& recPath) { DRing::setRecordPath(recPath); }
bool DaemonProxy::getIsAlwaysRecording() { return DRing::getIsAlwaysRecording(); }
void DaemonProxy::setIsAlwaysRecording(bool rec) { DRing::setIsAlwaysRecording(rec); }
bool DaemonProxy::getRecordPreview() { return DRing::getRecordPreview(); }
void DaemonProxy::setRecordPreview(bool rec) { DRing::setRecordPreview(rec); }
int DaemonProxy::getRecordQuality() { return DRing::getRecordQuality(); }
void DaemonProxy::setRecordQuality(int quality) { DRing::setRecordQuality(quality); }
void DaemonProxy::setHistoryLimit(int32_t days) { DRing::setHistoryLimit(days); }
int32_t DaemonProxy::getHistoryLimit() { return DRing::getHistoryLimit(); }
void DaemonProxy::setRingingTimeout(int32_t timeout) { DRing::setRingingTimeout(timeout); }
int32_t DaemonProxy::getRingingTimeout() { return DRing::getRingingTimeout(); }
void DaemonProxy::setAccountsOrder(const std::string& order) { DRing::setAccountsOrder(order); }
std::map<std::string, std::string> DaemonProxy::getHookSettings() { return DRing::getHookSettings(); }
void DaemonProxy::setHookSettings(const std::map<std::string, std::string>& settings) { DRing::setHookSettings(settings); }
std::vector<std::map<std::string, std::string>> DaemonProxy::getCredentials(const std::string& accountID) { return DRing::getCredentials(accountID); }
void DaemonProxy::setCredentials(const std::string& accountID, const std::vector<std::map<std::string, std::string>>& details) { DRing::setCredentials(accountID, details); }
std::string DaemonProxy::getAddrFromInterfaceName(const std::string& iface) { return DRing::getAddrFromInterfaceName(iface); }
std::vector<std::string> DaemonProxy::getAllIpInterface() { return DRing::getAllIpInterface(); }
std::vector<std::string> DaemonProxy::getAllIpInterfaceByName() { return DRing::getAllIpInterfaceByName(); }
std::map<std::string, std::string> DaemonProxy::getShortcuts() { return DRing::getShortcuts(); }
void DaemonProxy::setShortcuts(const std::map<std::string, std::string> &shortcutsMap) { DRing::setShortcuts(shortcutsMap); }
void DaemonProxy::setVolume(const std::string& device, double value) { DRing::setVolume(device, value); }
double DaemonProxy::getVolume(const std::string& device) { return DRing::getVolume(device); }
std::map<std::string, std::string> DaemonProxy::validateCertificate(const std::string& accountId, const std::string& certificate) { return DRing::validateCertificate(accountId, certificate); }
std::map<std::string, std::string> DaemonProxy::validateCertificatePath(const std::string& accountId, const std::string& certificatePath, const std::string& privateKey, const std::string& privateKeyPassword, const std::string& caList) { return DRing::validateCertificatePath(accountId, certificatePath, privateKey, privateKeyPassword, caList); }
std::map<std::string, std::string> DaemonProxy::getCertificateDetails(const std::string& certificate) { return DRing::getCertificateDetails(certificate); }
std::map<std::string, std::string> DaemonProxy::getCertificateDetailsPath(const std::string& certificatePath, const std::string& privateKey, const std::string& privateKeyPassword) { return DRing::getCertificateDetailsPath(certificatePath, privateKey, privateKeyPassword); }
std::vector<std::string> DaemonProxy::getPinnedCertificates() { return DRing::getPinnedCertificates(); }
std::vector<std::string> DaemonProxy::pinCertificate(const std::vector<uint8_t>& certificate, bool local) { return DRing::pinCertificate(certificate, local); }
bool DaemonProxy::unpinCertificate(const std::string& certId) { return DRing::unpinCertificate(certId); }
void DaemonProxy::pinCertificatePath(const std::string& path) { DRing::pinCertificatePath(path); }
unsigned DaemonProxy::unpinCertificatePath(const std::string& path) { return DRing::unpinCertificatePath(path); }
bool DaemonProxy::pinRemoteCertificate(const std::string& accountId, const std::string& certId) { return DRing::pinRemoteCertificate(accountId, certId); }
bool DaemonProxy::setCertificateStatus(const std::string& account, const std::string& certId, const std::string& status) { return DRing::setCertificateStatus(account, certId, status); }
std::vector<std::string> DaemonProxy::getCertificatesByStatus(const std::string& account, const std::string& status) { return DRing::getCertificatesByStatus(account, status); }
std::vector<std::map<std::string, std::string>> DaemonProxy::getTrustRequests(const std::string& accountId) { return DRing::getTrustRequests(accountId); }
bool DaemonProxy::acceptTrustRequest(const std::string& accountId, const std::string& from) { return DRing::acceptTrustRequest(accountId, from); }
bool DaemonProxy::discardTrustRequest(const std::string& accountId, const std::string& from) { return DRing::discardTrustRequest(accountId, from); }
void DaemonProxy::sendTrustRequest(const std::string& accountId, const std::string& to, const std::vector<uint8_t>& payload) { DRing::sendTrustRequest(accountId, to, payload); }
void DaemonProxy::addContact(const std::string& accountId, const std::string& uri) { DRing::addContact(accountId, uri); }
void DaemonProxy::removeContact(const std::string& accountId, const std::string& uri, bool ban) { DRing::removeContact(accountId, uri, ban); }
std::map<std::string, std::string> DaemonProxy::getContactDetails(const std::string& accountId, const std::string& uri) { return DRing::getContactDetails(accountId, uri); }
std::vector<std::map<std::string, std::string>> DaemonProxy::getContacts(const std::string& accountId) { return DRing::getContacts(accountId); }
int DaemonProxy::exportAccounts(const std::vector<std::string>& accountIDs, const std::string& filepath, const std::string& password) { return DRing::exportAccounts(accountIDs, filepath, password); }
int DaemonProxy::importAccounts(const std::string& archivePath, const std::string& password) { return DRing::importAccounts(archivePath, password); }
void DaemonProxy::connectivityChanged() { DRing::connectivityChanged(); }
void DaemonProxy::enableProxyClient(const std::string& accountID, bool enable) { DRing::enableProxyClient(accountID, enable); }
void DaemonProxy::setPushNotificationToken(const std::string& pushDeviceToken) { DRing::setPushNotificationToken(pushDeviceToken); }
void DaemonProxy::pushNotificationReceived(const std::string& from, const std::map<std::string, std::string>& data) { DRing::pushNotificationReceived(from, data); }
bool DaemonProxy::isAudioMeterActive(const std::string& id) { return DRing::isAudioMeterActive(id); }
void DaemonProxy::setAudioMeterState(const std::string& id, bool state) { DRing::setAudioMeterState(id, state); }

// wrappers for datatransfer_interface.h
std::vector<DataTransferId> DaemonProxy::dataTransferList() noexcept { return DRing::dataTransferList(); }
DataTransferError DaemonProxy::sendFile(const DataTransferInfo& info, DataTransferId& id) noexcept { return DRing::sendFile(info, id); }
DataTransferError DaemonProxy::acceptFileTransfer(const DataTransferId& id, const std::string& file_path, int64_t offset) noexcept { return DRing::acceptFileTransfer(id, file_path, offset); }
DataTransferError DaemonProxy::cancelDataTransfer(const DataTransferId& id) noexcept { return DRing::cancelDataTransfer(id); }
DataTransferError DaemonProxy::dataTransferInfo(const DataTransferId& id, DataTransferInfo& info) noexcept { return DRing::dataTransferInfo(id, info); }
DataTransferError DaemonProxy::dataTransferBytesProgress(const DataTransferId& id, int64_t& total, int64_t& progress) noexcept { return DRing::dataTransferBytesProgress(id, total, progress); }

// wrappers for presencemanager_interface.h
void DaemonProxy::publish(const std::string& accountID, bool status, const std::string& note) { DRing::publish(accountID, status, note); }
void DaemonProxy::answerServerRequest(const std::string& uri, bool flag) { DRing::answerServerRequest(uri, flag); }
void DaemonProxy::subscribeBuddy(const std::string& accountID, const std::string& uri, bool flag) { DRing::subscribeBuddy(accountID, uri, flag); }
std::vector<std::map<std::string, std::string>> DaemonProxy::getSubscriptions(const std::string& accountID) { return DRing::getSubscriptions(accountID); }
void DaemonProxy::setSubscriptions(const std::string& accountID, const std::vector<std::string>& uris) { DRing::setSubscriptions(accountID, uris); }

#ifdef ENABLE_VIDEO
// wrappers for videomanager_interface.h
std::vector<std::string> DaemonProxy::getDeviceList() { return DRing::getDeviceList(); }
VideoCapabilities DaemonProxy::getCapabilities(const std::string& name) { return DRing::getCapabilities(name); }
std::map<std::string, std::string> DaemonProxy::getSettings(const std::string& name) { return DRing::getSettings(name); }
void DaemonProxy::applySettings(const std::string& name, const std::map<std::string, std::string>& settings) { DRing::applySettings(name, settings); }
void DaemonProxy::setDefaultDevice(const std::string& name) { DRing::setDefaultDevice(name); }
void DaemonProxy::setDeviceOrientation(const std::string& name, int angle) { DRing::setDeviceOrientation(name, angle); }
std::map<std::string, std::string> DaemonProxy::getDeviceParams(const std::string& name) { return DRing::getDeviceParams(name); }
std::string DaemonProxy::getDefaultDevice() { return DRing::getDefaultDevice(); }
void DaemonProxy::startCamera() { DRing::startCamera(); }
void DaemonProxy::stopCamera() { DRing::stopCamera(); }
bool DaemonProxy::hasCameraStarted() { return DRing::hasCameraStarted(); }
void DaemonProxy::startAudioDevice() { DRing::startAudioDevice(); }
void DaemonProxy::stopAudioDevice() { DRing::stopAudioDevice(); }
bool DaemonProxy::switchInput(const std::string& resource) { return DRing::switchInput(resource); }
bool DaemonProxy::switchToCamera() { return DRing::switchToCamera(); }
void DaemonProxy::registerSinkTarget(const std::string& sinkId, const SinkTarget& target) { DRing::registerSinkTarget(sinkId, target); }
void DaemonProxy::registerAVSinkTarget(const std::string& sinkId, const AVSinkTarget& target) { DRing::registerAVSinkTarget(sinkId, target); }
std::map<std::string, std::string> DaemonProxy::getRenderer(const std::string& callId) { return DRing::getRenderer(callId); }
std::string DaemonProxy::startLocalRecorder(const bool& audioOnly, const std::string& filepath) { return DRing::startLocalRecorder(audioOnly, filepath); }
void DaemonProxy::stopLocalRecorder(const std::string& filepath) { DRing::stopLocalRecorder(filepath); }
bool DaemonProxy::getDecodingAccelerated() { return DRing::getDecodingAccelerated(); }
void DaemonProxy::setDecodingAccelerated(bool state) { DRing::setDecodingAccelerated(state); }
bool DaemonProxy::getEncodingAccelerated() { return DRing::getEncodingAccelerated(); }
void DaemonProxy::setEncodingAccelerated(bool state) { DRing::setEncodingAccelerated(state); }
#endif

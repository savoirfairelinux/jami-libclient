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
#include <sdbus-c++/sdbus-c++.h>
#include "net.jami.daemon1.CallManager.proxy.h"
#include "net.jami.daemon1.ConfigurationManager.proxy.h"
#include "net.jami.daemon1.DataTransfer.proxy.h"
#include "net.jami.daemon1.Instance.proxy.h"
#include "net.jami.daemon1.PresenceManager.proxy.h"
#ifdef ENABLE_VIDEO
  #include "net.jami.daemon1.VideoManager.proxy.h"
#endif

#include <sigc++/sigc++.h>
#include <string>

#ifdef WINDOWS
#include <processthreadsapi.h>
#else
#include <unistd.h>
#endif

class DaemonProxy : public sdbus::ProxyInterfaces<net::jami::daemon1::CallManager_proxy
                                                 ,net::jami::daemon1::ConfigurationManager_proxy
                                                 ,net::jami::daemon1::DataTransfer_proxy
                                                 ,net::jami::daemon1::Instance_proxy
                                                 ,net::jami::daemon1::PresenceManager_proxy
#ifdef ENABLE_VIDEO
                                                 ,net::jami::daemon1::VideoManager_proxy
#endif
                                                 >
{
public:
    /* The proxy will automatically start a procesing loop in a separate internal thread.
     * Handlers for incoming signals and asynchronous method replies
     * will be executed in the context of that thread. */
    static DaemonProxy& instance() {
        static DaemonProxy daemon("net.jami.daemon1", "/net/jami/daemon1");
        return daemon;
    }

    DaemonProxy(std::string destination, std::string objectPath)
        : sdbus::ProxyInterfaces(std::move(sdbus::createSessionBusConnection()),
                                 std::move(destination), std::move(objectPath))
    {
        registerProxy();
#ifdef WINDOWS
        auto pid = GetCurrentProcessId();
#else
        auto pid = getpid();
#endif
        Register(pid, "libjamiclient");
    }

    ~DaemonProxy()
    {
#ifdef WINDOWS
        auto pid = GetCurrentProcessId();
#else
        auto pid = getpid();
#endif
        Unregister(pid);
        unregisterProxy();
    }

#include "daemonproxysignals.h"

private:
    // net.jami.daemon1.CallManager signal handlers
    void onSmartInfo(const std::map<std::string, std::string>& info) override { mSignalSmartInfo.emit(info); }
    void onRecordPlaybackFilepath(const std::string& callID, const std::string& filepath) override { mSignalRecordPlaybackFilepath.emit(callID, filepath); }
    void onRecordPlaybackStopped(const std::string& filepath) override { mSignalRecordPlaybackStopped.emit(filepath); }
    void onUpdatePlaybackScale(const std::string& filepath, const int32_t& position, const int32_t& size) override { mSignalUpdatePlaybackScale.emit(filepath, position, size); }
    void onNewCallCreated(const std::string& accountID, const std::string& callID, const std::string& to) override { mSignalNewCallCreated.emit(accountID, callID, to); }
    void onIncomingCall(const std::string& accountID, const std::string& callID, const std::string& from) override { mSignalIncomingCall.emit(accountID, callID, from); }
    void onIncomingMessage(const std::string& callID, const std::string& from, const std::map<std::string, std::string>& messages) override { mSignalIncomingMessage.emit(callID, from, messages); }
    void onCallStateChanged(const std::string& callID, const std::string& state, const int32_t& code) override { mSignalCallStateChanged.emit(callID, state, code); }
    void onConferenceChanged(const std::string& confID, const std::string& state) override { mSignalConferenceChanged.emit(confID, state); }
    void onConferenceCreated(const std::string& confID) override { mSignalConferenceCreated.emit(confID); }
    void onConferenceRemoved(const std::string& confID) override { mSignalConferenceRemoved.emit(confID); }
    void onVoiceMailNotify(const std::string& accountID, const int32_t& count) override { mSignalVoiceMailNotify.emit(accountID, count); }
    void onTransferSucceeded() override { mSignalTransferSucceeded.emit(); }
    void onTransferFailed() override { mSignalTransferFailed.emit(); }
    void onSecureSdesOn(const std::string& callID) override { mSignalSecureSdesOn.emit(callID); }
    void onSecureSdesOff(const std::string& callID) override { mSignalSecureSdesOff.emit(callID); }
    void onRecordingStateChanged(const std::string& callID, const bool& recordingState) override { mSignalRecordingStateChanged.emit(callID, recordingState); }
    void onOnRtcpReportReceived(const std::string& callID, const std::map<std::string, int32_t>& report) override { mSignalOnRtcpReportReceived.emit(callID, report); }
    void onPeerHold(const std::string& callID, const bool& peerHolding) override { mSignalPeerHold.emit(callID, peerHolding); }
    void onAudioMuted(const std::string& callID, const bool& audioMuted) override { mSignalAudioMuted.emit(callID, audioMuted); }
    void onVideoMuted(const std::string& callID, const bool& videoMuted) override { mSignalVideoMuted.emit(callID, videoMuted); }

    // net.jami.daemon1.ConfigurationManager signal handlers
    void onExportOnRingEnded(const std::string& accountID, const int32_t& status, const std::string& PIN) override { mSignalExportOnRingEnded.emit(accountID, status, PIN); }
    void onDeviceRevocationEnded(const std::string& accountID, const std::string& deviceId, const int32_t& status) override { mSignalDeviceRevocationEnded.emit(accountID, deviceId, status); }
    void onKnownDevicesChanged(const std::string& accountID, const std::map<std::string, std::string>& devices) override { mSignalKnownDevicesChanged.emit(accountID, devices); }
    void onRegisteredNameFound(const std::string& accountID, const int32_t& status, const std::string& address, const std::string& name) override { mSignalRegisteredNameFound.emit(accountID, status, address, name); }
    void onNameRegistrationEnded(const std::string& accountID, const int32_t& status, const std::string& name) override { mSignalNameRegistrationEnded.emit(accountID, status, name); }
    void onIncomingAccountMessage(const std::string& accountID, const std::string& from, const std::map<std::string, std::string>& payloads) override { mSignalIncomingAccountMessage.emit(accountID, from, payloads); }
    void onAccountMessageStatusChanged(const std::string& accountID, const uint64_t& id, const std::string& to, const int32_t& status) override { mSignalAccountMessageStatusChanged.emit(accountID, id, to, status); }
    void onVolumeChanged(const std::string& device, const double& value) override { mSignalVolumeChanged.emit(device, value); }
    void onHardwareDecodingChanged(const bool& state) override { mSignalHardwareDecodingChanged.emit(state); }
    void onHardwareEncodingChanged(const bool& state) override { mSignalHardwareEncodingChanged.emit(state); }
    void onAudioDeviceEvent() override { mSignalAudioDeviceEvent.emit(); }
    void onAudioMeter(const std::string& id, const double& level) override { mSignalAudioMeter.emit(id, level); }
    void onAccountsChanged() override { mSignalAccountsChanged.emit(); }
    void onAccountDetailsChanged(const std::string& accountID, const std::map<std::string, std::string>& details) override { mSignalAccountDetailsChanged.emit(accountID, details); }
    void onRegistrationStateChanged(const std::string& accountID, const std::string& registrationState, const int32_t& registrationDetail, const std::string& registrationDetailStr) override { mSignalRegistrationStateChanged.emit(accountID, registrationState, registrationDetail, registrationDetailStr); }
    void onVolatileAccountDetailsChanged(const std::string& accountID, const std::map<std::string, std::string>& details) override { mSignalVolatileAccountDetailsChanged.emit(accountID, details); }
    void onStunStatusFailure(const std::string& reason) override { mSignalStunStatusFailure.emit(reason); }
    void onErrorAlert(const int32_t& code) override { mSignalErrorAlert.emit(code); }
    void onCertificateStateChanged(const std::string& accountId, const std::string& certId, const std::string& state) override { mSignalCertificateStateChanged.emit(accountId, certId, state); }
    void onCertificatePinned(const std::string& certId) override { mSignalCertificatePinned.emit(certId); }
    void onCertificatePathPinned(const std::string& path, const std::vector<std::string>& certIds) override { mSignalCertificatePathPinned.emit(path, certIds); }
    void onCertificateExpired(const std::string& certId) override { mSignalCertificateExpired.emit(certId); }
    void onIncomingTrustRequest(const std::string& accountID, const std::string& from, const std::vector<uint8_t>& payload, const uint64_t& receiveTime) override { mSignalIncomingTrustRequest.emit(accountID, from, payload, receiveTime); }
    void onContactAdded(const std::string& accountID, const std::string& uri, const bool& confirmed) override { mSignalContactAdded.emit(accountID, uri, confirmed); }
    void onContactRemoved(const std::string& accountID, const std::string& uri, const bool& banned) override { mSignalContactRemoved.emit(accountID, uri, banned); }
    void onMediaParametersChanged(const std::string& accountID) override { mSignalMediaParametersChanged.emit(accountID); }
    void onMigrationEnded(const std::string& accountID, const std::string& result) override { mSignalMigrationEnded.emit(accountID, result); }
    void onDebugMessageReceived(const std::string& message) override { mSignalDebugMessageReceived.emit(message); }

    // net.jami.daemon1.DataTransfer signal handlers
    void onDataTransferEvent(const uint64_t& id, const int32_t& code) override { mSignalDataTransferEvent.emit(id, code); }

    // net.jami.daemon1.Instance signal handlers
    void onStarted() override { mSignalStarted.emit(); }

    // net.jami.daemon1.PresenceManager signal handlers
    void onNewBuddyNotification(const std::string& accountID, const std::string& buddyUri, const bool& status, const std::string& lineStatus) override { mSignalNewBuddyNotification.emit(accountID, buddyUri, status, lineStatus); }
    void onNearbyPeerNotification(const std::string& accountID, const std::string& buddyUri, const int32_t& status, const std::string& displayname) override { mSignalNearbyPeerNotification.emit(accountID, buddyUri, status, displayname); }
    void onSubscriptionStateChanged(const std::string& accountID, const std::string& buddyUri, const bool& state) override { mSignalSubscriptionStateChanged.emit(accountID, buddyUri, state); }
    void onNewServerSubscriptionRequest(const std::string& buddyUri) override { mSignalNewServerSubscriptionRequest.emit(buddyUri); }
    void onServerError(const std::string& accountID, const std::string& error, const std::string& msg) override { mSignalServerError.emit(accountID, error, msg); }

#ifdef ENABLE_VIDEO
    // net.jami.daemon1.VideoManager signal handlers
    void onDeviceEvent() override { mSignalDeviceEvent.emit(); }
    void onStartedDecoding(const std::string& id, const std::string& shmPath, const int32_t& width, const int32_t& height, const bool& isMixer) override { mSignalStartedDecoding.emit(id, shmPath, width, height, isMixer); }
    void onStoppedDecoding(const std::string& id, const std::string& shmPath, const bool& isMixer) override { mSignalStoppedDecoding.emit(id, shmPath, isMixer); }
#endif
};

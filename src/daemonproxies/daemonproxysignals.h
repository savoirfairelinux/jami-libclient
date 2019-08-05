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

public:
    // signal getters for net.jami.daemon1.CallManager
    sigc::signal<void, const std::map<std::string, std::string>&> signalSmartInfo() { return mSignalSmartInfo; }
    sigc::signal<void, const std::string&, const std::string&> signalRecordPlaybackFilepath() { return mSignalRecordPlaybackFilepath; }
    sigc::signal<void, const std::string&> signalRecordPlaybackStopped() { return mSignalRecordPlaybackStopped; }
    sigc::signal<void, const std::string&, const int32_t&, const int32_t&> signalUpdatePlaybackScale() { return mSignalUpdatePlaybackScale; }
    sigc::signal<void, const std::string&, const std::string&, const std::string&> signalNewCallCreated() { return mSignalNewCallCreated; }
    sigc::signal<void, const std::string&, const std::string&, const std::string&> signalIncomingCall() { return mSignalIncomingCall; }
    sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&> signalIncomingMessage() { return mSignalIncomingMessage; }
    sigc::signal<void, const std::string&, const std::string&, const int32_t&> signalCallStateChanged() { return mSignalCallStateChanged; }
    sigc::signal<void, const std::string&, const std::string&> signalConferenceChanged() { return mSignalConferenceChanged; }
    sigc::signal<void, const std::string&> signalConferenceCreated() { return mSignalConferenceCreated; }
    sigc::signal<void, const std::string&> signalConferenceRemoved() { return mSignalConferenceRemoved; }
    sigc::signal<void, const std::string&, const int32_t&> signalVoiceMailNotify() { return mSignalVoiceMailNotify; }
    sigc::signal<void> signalTransferSucceeded() { return mSignalTransferSucceeded; }
    sigc::signal<void> signalTransferFailed() { return mSignalTransferFailed; }
    sigc::signal<void, const std::string&> signalSecureSdesOn() { return mSignalSecureSdesOn; }
    sigc::signal<void, const std::string&> signalSecureSdesOff() { return mSignalSecureSdesOff; }
    sigc::signal<void, const std::string&, const bool&> signalRecordingStateChanged() { return mSignalRecordingStateChanged; }
    sigc::signal<void, const std::string&, const std::map<std::string, int32_t>&> signalOnRtcpReportReceived() { return mSignalOnRtcpReportReceived; }
    sigc::signal<void, const std::string&, const bool&> signalPeerHold() { return mSignalPeerHold; }
    sigc::signal<void, const std::string&, const bool&> signalAudioMuted() { return mSignalAudioMuted; }
    sigc::signal<void, const std::string&, const bool&> signalVideoMuted() { return mSignalVideoMuted; }

    // signal getters for net.jami.daemon1.ConfigurationManager
    sigc::signal<void, const std::string&, const int32_t&, const std::string&> signalExportOnRingEnded() { return mSignalExportOnRingEnded; }
    sigc::signal<void, const std::string&, const std::string&, const int32_t&> signalDeviceRevocationEnded() { return mSignalDeviceRevocationEnded; }
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> signalKnownDevicesChanged() { return mSignalKnownDevicesChanged; }
    sigc::signal<void, const std::string&, const int32_t&, const std::string&, const std::string&> signalRegisteredNameFound() { return mSignalRegisteredNameFound; }
    sigc::signal<void, const std::string&, const int32_t&, const std::string&> signalNameRegistrationEnded() { return mSignalNameRegistrationEnded; }
    sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&> signalIncomingAccountMessage() { return mSignalIncomingAccountMessage; }
    sigc::signal<void, const std::string&, const uint64_t&, const std::string&, const int32_t&> signalAccountMessageStatusChanged() { return mSignalAccountMessageStatusChanged; }
    sigc::signal<void, const std::string&, const double&> signalVolumeChanged() { return mSignalVolumeChanged; }
    sigc::signal<void, const bool&> signalHardwareDecodingChanged() { return mSignalHardwareDecodingChanged; }
    sigc::signal<void, const bool&> signalHardwareEncodingChanged() { return mSignalHardwareEncodingChanged; }
    sigc::signal<void> signalAudioDeviceEvent() { return mSignalAudioDeviceEvent; }
    sigc::signal<void, const std::string&, const double&> signalAudioMeter() { return mSignalAudioMeter; }
    sigc::signal<void> signalAccountsChanged() { return mSignalAccountsChanged; }
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> signalAccountDetailsChanged() { return mSignalAccountDetailsChanged; }
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&> signalRegistrationStateChanged() { return mSignalRegistrationStateChanged; }
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> signalVolatileAccountDetailsChanged() { return mSignalVolatileAccountDetailsChanged; }
    sigc::signal<void, const std::string&> signalStunStatusFailure() { return mSignalStunStatusFailure; }
    sigc::signal<void, const int32_t&> signalErrorAlert() { return mSignalErrorAlert; }
    sigc::signal<void, const std::string&, const std::string&, const std::string&> signalCertificateStateChanged() { return mSignalCertificateStateChanged; }
    sigc::signal<void, const std::string&> signalCertificatePinned() { return mSignalCertificatePinned; }
    sigc::signal<void, const std::string&, const std::vector<std::string>&> signalCertificatePathPinned() { return mSignalCertificatePathPinned; }
    sigc::signal<void, const std::string&> signalCertificateExpired() { return mSignalCertificateExpired; }
    sigc::signal<void, const std::string&, const std::string&, const std::vector<uint8_t>&, const uint64_t&> signalIncomingTrustRequest() { return mSignalIncomingTrustRequest; }
    sigc::signal<void, const std::string&, const std::string&, const bool&> signalContactAdded() { return mSignalContactAdded; }
    sigc::signal<void, const std::string&, const std::string&, const bool&> signalContactRemoved() { return mSignalContactRemoved; }
    sigc::signal<void, const std::string&> signalMediaParametersChanged() { return mSignalMediaParametersChanged; }
    sigc::signal<void, const std::string&, const std::string&> signalMigrationEnded() { return mSignalMigrationEnded; }
    sigc::signal<void, const std::string&> signalDebugMessageReceived() { return mSignalDebugMessageReceived; }

    // signal getters for net.jami.daemon1.DataTransfer
    sigc::signal<void, const uint64_t&, const int32_t&> signalDataTransferEvent() { return mSignalDataTransferEvent; }

    // signal getters for net.jami.daemon1.Instance
    sigc::signal<void> signalStarted() { return mSignalStarted; }

    // signal getters for net.jami.daemon1.PresenceManager
    sigc::signal<void, const std::string&, const std::string&, const bool&, const std::string&> signalNewBuddyNotification() { return mSignalNewBuddyNotification; }
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&> signalNearbyPeerNotification() { return mSignalNearbyPeerNotification; }
    sigc::signal<void, const std::string&, const std::string&, const bool&> signalSubscriptionStateChanged() { return mSignalSubscriptionStateChanged; }
    sigc::signal<void, const std::string&> signalNewServerSubscriptionRequest() { return mSignalNewServerSubscriptionRequest; }
    sigc::signal<void, const std::string&, const std::string&, const std::string&> signalServerError() { return mSignalServerError; }

#ifdef ENABLE_VIDEO
    // signal getters for net.jami.daemon1.VideoManager
    sigc::signal<void> signalDeviceEvent() { return mSignalDeviceEvent; }
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const int32_t&, const bool&> signalStartedDecoding() { return mSignalStartedDecoding; }
    sigc::signal<void, const std::string&, const std::string&, const bool&> signalStoppedDecoding() { return mSignalStoppedDecoding; }
#endif

protected:
    // signals for net.jami.daemon1.CallManager
    sigc::signal<void, const std::map<std::string, std::string>&> mSignalSmartInfo;
    sigc::signal<void, const std::string&, const std::string&> mSignalRecordPlaybackFilepath;
    sigc::signal<void, const std::string&> mSignalRecordPlaybackStopped;
    sigc::signal<void, const std::string&, const int32_t&, const int32_t&> mSignalUpdatePlaybackScale;
    sigc::signal<void, const std::string&, const std::string&, const std::string&> mSignalNewCallCreated;
    sigc::signal<void, const std::string&, const std::string&, const std::string&> mSignalIncomingCall;
    sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&> mSignalIncomingMessage;
    sigc::signal<void, const std::string&, const std::string&, const int32_t&> mSignalCallStateChanged;
    sigc::signal<void, const std::string&, const std::string&> mSignalConferenceChanged;
    sigc::signal<void, const std::string&> mSignalConferenceCreated;
    sigc::signal<void, const std::string&> mSignalConferenceRemoved;
    sigc::signal<void, const std::string&, const int32_t&> mSignalVoiceMailNotify;
    sigc::signal<void> mSignalTransferSucceeded;
    sigc::signal<void> mSignalTransferFailed;
    sigc::signal<void, const std::string&> mSignalSecureSdesOn;
    sigc::signal<void, const std::string&> mSignalSecureSdesOff;
    sigc::signal<void, const std::string&, const bool&> mSignalRecordingStateChanged;
    sigc::signal<void, const std::string&, const std::map<std::string, int32_t>&> mSignalOnRtcpReportReceived;
    sigc::signal<void, const std::string&, const bool&> mSignalPeerHold;
    sigc::signal<void, const std::string&, const bool&> mSignalAudioMuted;
    sigc::signal<void, const std::string&, const bool&> mSignalVideoMuted;

    // signals for net.jami.daemon1.ConfigurationManager
    sigc::signal<void, const std::string&, const int32_t&, const std::string&> mSignalExportOnRingEnded;
    sigc::signal<void, const std::string&, const std::string&, const int32_t&> mSignalDeviceRevocationEnded;
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> mSignalKnownDevicesChanged;
    sigc::signal<void, const std::string&, const int32_t&, const std::string&, const std::string&> mSignalRegisteredNameFound;
    sigc::signal<void, const std::string&, const int32_t&, const std::string&> mSignalNameRegistrationEnded;
    sigc::signal<void, const std::string&, const std::string&, const std::map<std::string, std::string>&> mSignalIncomingAccountMessage;
    sigc::signal<void, const std::string&, const uint64_t&, const std::string&, const int32_t&> mSignalAccountMessageStatusChanged;
    sigc::signal<void, const std::string&, const double&> mSignalVolumeChanged;
    sigc::signal<void, const bool&> mSignalHardwareDecodingChanged;
    sigc::signal<void, const bool&> mSignalHardwareEncodingChanged;
    sigc::signal<void> mSignalAudioDeviceEvent;
    sigc::signal<void, const std::string&, const double&> mSignalAudioMeter;
    sigc::signal<void> mSignalAccountsChanged;
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> mSignalAccountDetailsChanged;
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&> mSignalRegistrationStateChanged;
    sigc::signal<void, const std::string&, const std::map<std::string, std::string>&> mSignalVolatileAccountDetailsChanged;
    sigc::signal<void, const std::string&> mSignalStunStatusFailure;
    sigc::signal<void, const int32_t&> mSignalErrorAlert;
    sigc::signal<void, const std::string&, const std::string&, const std::string&> mSignalCertificateStateChanged;
    sigc::signal<void, const std::string&> mSignalCertificatePinned;
    sigc::signal<void, const std::string&, const std::vector<std::string>&> mSignalCertificatePathPinned;
    sigc::signal<void, const std::string&> mSignalCertificateExpired;
    sigc::signal<void, const std::string&, const std::string&, const std::vector<uint8_t>&, const uint64_t&> mSignalIncomingTrustRequest;
    sigc::signal<void, const std::string&, const std::string&, const bool&> mSignalContactAdded;
    sigc::signal<void, const std::string&, const std::string&, const bool&> mSignalContactRemoved;
    sigc::signal<void, const std::string&> mSignalMediaParametersChanged;
    sigc::signal<void, const std::string&, const std::string&> mSignalMigrationEnded;
    sigc::signal<void, const std::string&> mSignalDebugMessageReceived;

    // signals for net.jami.daemon1.DataTransfer
    sigc::signal<void, const uint64_t&, const int32_t&> mSignalDataTransferEvent;

    // signals for net.jami.daemon1.Instance
    sigc::signal<void> mSignalStarted;

    // signals for net.jami.daemon1.PresenceManager
    sigc::signal<void, const std::string&, const std::string&, const bool&, const std::string&> mSignalNewBuddyNotification;
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const std::string&> mSignalNearbyPeerNotification;
    sigc::signal<void, const std::string&, const std::string&, const bool&> mSignalSubscriptionStateChanged;
    sigc::signal<void, const std::string&> mSignalNewServerSubscriptionRequest;
    sigc::signal<void, const std::string&, const std::string&, const std::string&> mSignalServerError;

#ifdef ENABLE_VIDEO
    // signals for net.jami.daemon1.VideoManager
    sigc::signal<void> mSignalDeviceEvent;
    sigc::signal<void, const std::string&, const std::string&, const int32_t&, const int32_t&, const bool&> mSignalStartedDecoding;
    sigc::signal<void, const std::string&, const std::string&, const bool&> mSignalStoppedDecoding;
#endif

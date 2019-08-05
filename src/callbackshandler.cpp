/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "callbackshandler.h"

// Models and database
#include "api/account.h"
#include "api/lrc.h"
#include "api/newaccountmodel.h"
#include "api/datatransfermodel.h"
#include "api/behaviorcontroller.h"

#include <sigc++/sigc++.h>
#include "daemonproxy.h"

// DRing
#include <datatransfer_interface.h>

namespace lrc
{

using namespace api;

CallbacksHandler::CallbacksHandler(const Lrc& parent)
: QObject()
, parent(parent)
{
    auto& daemon = DaemonProxy::instance();

    // Get signals from daemon
    daemon.signalIncomingAccountMessage().connect(sigc::mem_fun(*this, &CallbacksHandler::slotNewAccountMessage));
    daemon.signalNewBuddyNotification().connect(sigc::mem_fun(*this, &CallbacksHandler::slotNewBuddySubscription));
    daemon.signalNearbyPeerNotification().connect(sigc::mem_fun(*this, &CallbacksHandler::slotNearbyPeerSubscription));
    daemon.signalContactAdded().connect(sigc::mem_fun(*this, &CallbacksHandler::slotContactAdded));
    daemon.signalContactRemoved().connect(sigc::mem_fun(*this, &CallbacksHandler::slotContactRemoved));
    daemon.signalIncomingTrustRequest().connect(sigc::mem_fun(*this, &CallbacksHandler::slotIncomingContactRequest));
    daemon.signalAccountMessageStatusChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotAccountMessageStatusChanged));
    daemon.signalAccountDetailsChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotAccountDetailsChanged));
    daemon.signalAccountsChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotAccountsChanged));
    daemon.signalRegistrationStateChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotRegistrationStateChanged));
    daemon.signalIncomingCall().connect(sigc::mem_fun(*this, &CallbacksHandler::slotIncomingCall));
    daemon.signalCallStateChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotCallStateChanged));
    daemon.signalConferenceCreated().connect(sigc::mem_fun(*this, &CallbacksHandler::slotConferenceCreated));
    daemon.signalConferenceRemoved().connect(sigc::mem_fun(*this, &CallbacksHandler::slotConferenceRemoved));
    daemon.signalConferenceChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotConferenceChanged));
    daemon.signalIncomingMessage().connect(sigc::mem_fun(*this, &CallbacksHandler::slotIncomingMessage));
    daemon.signalDataTransferEvent().connect(sigc::mem_fun(*this, &CallbacksHandler::slotDataTransferEvent));
    daemon.signalKnownDevicesChanged().connect(sigc::mem_fun(*this, &CallbacksHandler::slotKnownDevicesChanged));
    daemon.signalDeviceRevocationEnded().connect(sigc::mem_fun(*this, &CallbacksHandler::slotDeviceRevokationEnded));
    daemon.signalExportOnRingEnded().connect(sigc::mem_fun(*this, &CallbacksHandler::slotExportOnRingEnded));
    daemon.signalNameRegistrationEnded().connect(sigc::mem_fun(*this, &CallbacksHandler::slotNameRegistrationEnded));
    daemon.signalRegisteredNameFound().connect(sigc::mem_fun(*this, &CallbacksHandler::slotRegisteredNameFound));
    daemon.signalMigrationEnded().connect(sigc::mem_fun(*this, &CallbacksHandler::slotMigrationEnded));
    daemon.signalAudioMeter().connect(sigc::mem_fun(*this, &CallbacksHandler::slotAudioMeterReceived));

#ifdef ENABLE_VIDEO
    daemon.signalStartedDecoding().connect(sigc::mem_fun(*this, &CallbacksHandler::slotStartedDecoding));
    daemon.signalStoppedDecoding().connect(sigc::mem_fun(*this, &CallbacksHandler::slotStoppedDecoding));
    daemon.signalDeviceEvent().connect(sigc::mem_fun(*this, &CallbacksHandler::slotDeviceEvent));
#endif
}

CallbacksHandler::~CallbacksHandler()
{
}

void
CallbacksHandler::subscribeToDebugReceived()
{
    DaemonProxy::instance().signalDebugMessageReceived().connect(sigc::mem_fun(*this, &CallbacksHandler::slotDebugMessageReceived));
}

void
CallbacksHandler::slotNewAccountMessage(const std::string& accountId,
                                        const std::string& from,
                                        const std::map<std::string, std::string>& payloads )
{
    Q_EMIT newAccountMessage(accountId, from, payloads);
}

void
CallbacksHandler::slotNewBuddySubscription(const std::string& accountId,
                                           const std::string& uri,
                                           const bool& status,
                                           const std::string& message )
{
    Q_EMIT newBuddySubscription(uri, status);
}

void
CallbacksHandler::slotNearbyPeerSubscription(const std::string& accountId,
                                             const std::string& contactUri,
                                             const int32_t& state,
                                             const std::string& displayname )
{
    Q_EMIT newPeerSubscription(accountId, contactUri, state, displayname);
}

void
CallbacksHandler::slotContactAdded(const std::string& accountId,
                                   const std::string& contactUri,
                                   const bool& confirmed )
{
    Q_EMIT contactAdded(accountId, contactUri, confirmed);
}

void
CallbacksHandler::slotContactRemoved(const std::string& accountId,
                                     const std::string& contactUri,
                                     const bool& banned )
{
    Q_EMIT contactRemoved(accountId, contactUri, banned);
}

void
CallbacksHandler::slotIncomingContactRequest(const std::string& accountId,
                                             const std::string& ringId,
                                             const std::vector<uint8_t>& payload,
                                             const uint64_t& time )
{
    Q_EMIT incomingContactRequest(accountId, ringId, std::string(payload.begin(), payload.end()));
}

void
CallbacksHandler::slotIncomingCall(const std::string& accountId,
                                   const std::string& callId,
                                   const std::string& fromUri )
{
    if (fromUri.find("ring.dht") != std::string::npos) {
        Q_EMIT incomingCall(accountId, callId, fromUri.substr(fromUri.size()-50, 40));
    } else {
        auto left = fromUri.find_first_of('<')+1;
        auto right = fromUri.find_first_of('@');
        Q_EMIT incomingCall(accountId, callId, fromUri.substr(left, right-left));
    }
}

void
CallbacksHandler::slotCallStateChanged(const std::string& callId, const std::string& state, const int32_t& code)
{
    Q_EMIT callStateChanged(callId, state, code);
}

void
CallbacksHandler::slotAccountDetailsChanged(const std::string& accountId,
                                            const std::map<std::string, std::string>& details )
{
    Q_EMIT accountDetailsChanged(accountId, details);
}

void
CallbacksHandler::slotAccountsChanged()
{
    Q_EMIT accountsChanged();
}

void
CallbacksHandler::slotRegistrationStateChanged(const std::string& accountId,
                                               const std::string& registration_state,
                                               const int32_t& detail_code,
                                               const std::string& detail_str )
{
    Q_EMIT accountStatusChanged(accountId, lrc::api::account::to_status(registration_state));
}

void
CallbacksHandler::slotIncomingMessage(const std::string& callId,
                                      const std::string& from,
                                      const std::map<std::string, std::string>& interaction )
{
    std::string from2;
    if (from.find("ring.dht") != std::string::npos) {
        from2 = from.substr(0,40);
    }
    else {
        auto left = from.find_first_of(':')+1;
        auto right = from.find_first_of('@');
        from2 = from.substr(left, right-left);
    }

    for (auto& e : interaction) {
        if (e.first.find("x-ring/ring.profile.vcard") != std::string::npos) {
            auto index = e.first.find_first_of(';');
            auto piece = e.first.substr(index+1);
            auto comma1 = piece.find_first_of(',');
            auto comma2 = piece.find_first_of(',', comma1+1);
            auto comma3 = piece.find_first_of(',', comma2+1);
            auto pair1 = piece.substr(comma1, comma2-comma1);
            auto pair2 = piece.substr(comma2, (comma3 == std::string::npos)? std::string::npos : comma3-comma2);
            index = pair1.find_first_of('=');
            auto value1 = std::stoi(pair1.substr(index+1));
            index = pair2.find_first_of('=');
            auto value2 = std::stoi(pair2.substr(index+1));
            Q_EMIT incomingVCardChunk(callId, from2, value1, value2, e.second);
        } else { // we consider it as an usual message interaction
            Q_EMIT incomingCallMessage(callId, from2, e.second);
        }
    }
}

void
CallbacksHandler::slotConferenceCreated(const std::string& callId)
{
    Q_EMIT conferenceCreated(callId);
}

void
CallbacksHandler::slotConferenceChanged(const std::string& callId, const std::string& state)
{
    slotCallStateChanged(callId, state, 0);
}

void
CallbacksHandler::slotConferenceRemoved(const std::string& callId)
{
    Q_EMIT conferenceRemoved(callId);
}

void
CallbacksHandler::slotAccountMessageStatusChanged(const std::string& accountId,
                                                  const uint64_t& id,
                                                  const std::string& to,
                                                  const int32_t& status )
{
    Q_EMIT accountMessageStatusChanged(accountId, id, to, status);
}

void
CallbacksHandler::slotDataTransferEvent(const uint64_t& dringId, const int32_t& codeStatus)
{
    auto event = DRing::DataTransferEventCode(codeStatus);

    api::datatransfer::Info info;

    // FIXME: This will be called on DBus thread!
    parent.getDataTransferModel().transferInfo(dringId, info);

    // WARNING: info.status could be INVALID in case of async signaling
    // So listeners must only take account of dringId in such case.
    // Is useful for "termination" status like unjoinable_peer.

    switch (event) {
    case DRing::DataTransferEventCode::created:
        Q_EMIT transferStatusCreated(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::closed_by_host:
    case DRing::DataTransferEventCode::closed_by_peer:
        Q_EMIT transferStatusCanceled(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::wait_peer_acceptance:
        Q_EMIT transferStatusAwaitingPeer(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::wait_host_acceptance:
        Q_EMIT transferStatusAwaitingHost(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::ongoing:
        Q_EMIT transferStatusOngoing(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::finished:
        Q_EMIT transferStatusFinished(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::invalid_pathname:
    case DRing::DataTransferEventCode::unsupported:
        Q_EMIT transferStatusError(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::timeout_expired:
        Q_EMIT transferStatusTimeoutExpired(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::unjoinable_peer:
        Q_EMIT transferStatusUnjoinable(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::invalid:
        break;
    }
}

void
CallbacksHandler::slotKnownDevicesChanged(const std::string& accountId,
                                          const std::map<std::string, std::string>& devices )
{
    Q_EMIT knownDevicesChanged(accountId, devices);
}

void
CallbacksHandler::slotDeviceRevokationEnded(const std::string& accountId,
                                            const std::string& deviceId,
                                            const int32_t& status )
{
    Q_EMIT deviceRevocationEnded(accountId, deviceId, status);
}

void
CallbacksHandler::slotExportOnRingEnded(const std::string& accountId, const int32_t& status, const std::string& pin)
{
    Q_EMIT exportOnRingEnded(accountId, status, pin);
}

void
CallbacksHandler::slotNameRegistrationEnded(const std::string& accountId, const int32_t& status, const std::string& name)
{
    Q_EMIT nameRegistrationEnded(accountId, status, name);
}

void
CallbacksHandler::slotRegisteredNameFound(const std::string& accountId,
                                          const int32_t& status,
                                          const std::string& address,
                                          const std::string& name )
{
    Q_EMIT registeredNameFound(accountId, status, address, name);
}

void
CallbacksHandler::slotMigrationEnded(const std::string& accountId, const std::string& status)
{
    Q_EMIT migrationEnded(accountId, status == "SUCCESS");
}

void
CallbacksHandler::slotDebugMessageReceived(const std::string& message)
{
    Q_EMIT parent.getBehaviorController().debugMessageReceived(message);
}

void
CallbacksHandler::slotStartedDecoding(const std::string& id,
                                      const std::string& shmPath,
                                      const int32_t& width,
                                      const int32_t& height,
                                      const bool& )
{
    Q_EMIT startedDecoding(id, shmPath, width, height);
}

void
CallbacksHandler::slotStoppedDecoding(const std::string& id, const std::string& shmPath, const bool& )
{
    Q_EMIT stoppedDecoding(id, shmPath);
}

void
CallbacksHandler::slotDeviceEvent()
{
    Q_EMIT deviceEvent();
}

void
CallbacksHandler::slotAudioMeterReceived(const std::string& id, const double& level)
{
    Q_EMIT audioMeter(id, level);
}

} // namespace lrc

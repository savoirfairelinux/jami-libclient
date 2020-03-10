/****************************************************************************
 *    Copyright (C) 2017-2020 Savoir-faire Linux Inc.                       *
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

// Lrc
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "dbus/videomanager.h"

// DRing
#include <datatransfer_interface.h>

#ifdef ENABLE_LIBWRAP
// For the debugMessageReceived connection that queues const std::string refs
// when not using dbus
Q_DECLARE_METATYPE(std::string);
#endif

namespace lrc
{

using namespace api;

CallbacksHandler::CallbacksHandler(const Lrc& parent)
: QObject()
, parent(parent)
{
    // Get signals from daemon
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingAccountMessage,
            this,
            &CallbacksHandler::slotNewAccountMessage,
            Qt::QueuedConnection);

    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::newBuddyNotification,
            this,
            &CallbacksHandler::slotNewBuddySubscription,
            Qt::QueuedConnection);

    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::nearbyPeerNotification,
            this,
            &CallbacksHandler::slotNearbyPeerSubscription,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactAdded,
            this,
            &CallbacksHandler::slotContactAdded,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactRemoved,
            this,
            &CallbacksHandler::slotContactRemoved,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingTrustRequest,
            this,
            &CallbacksHandler::slotIncomingContactRequest,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::accountMessageStatusChanged,
            this,
            &CallbacksHandler::slotAccountMessageStatusChanged,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::accountDetailsChanged,
            this,
            &CallbacksHandler::slotAccountDetailsChanged,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::volatileAccountDetailsChanged,
            this,
            &CallbacksHandler::slotVolatileAccountDetailsChanged,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::accountsChanged,
            this,
            &CallbacksHandler::slotAccountsChanged);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::registrationStateChanged,
            this,
            &CallbacksHandler::slotRegistrationStateChanged,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::incomingCall,
            this,
            &CallbacksHandler::slotIncomingCall,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::callStateChanged,
            this,
            &CallbacksHandler::slotCallStateChanged,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceCreated,
            this,
            &CallbacksHandler::slotConferenceCreated,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceRemoved,
            this,
            &CallbacksHandler::slotConferenceRemoved,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceChanged,
            this,
            &CallbacksHandler::slotConferenceChanged,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::incomingMessage,
            this,
            &CallbacksHandler::slotIncomingMessage,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::recordPlaybackStopped,
            this,
            &CallbacksHandler::slotRecordPlaybackStopped,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::voiceMailNotify,
            this,
            &CallbacksHandler::slotVoiceMailNotify,
            Qt::QueuedConnection);

    connect(&CallManager::instance(),
            &CallManagerInterface::remoteRecordingChanged,
            this,
            &CallbacksHandler::slotRemoteRecordingChanged,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::dataTransferEvent,
            this,
            &CallbacksHandler::slotDataTransferEvent,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::knownDevicesChanged,
            this,
            &CallbacksHandler::slotKnownDevicesChanged,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::deviceRevocationEnded,
            this,
            &CallbacksHandler::slotDeviceRevokationEnded,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::exportOnRingEnded,
            this,
            &CallbacksHandler::slotExportOnRingEnded,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::nameRegistrationEnded,
            this,
            &CallbacksHandler::slotNameRegistrationEnded,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::registeredNameFound,
            this,
            &CallbacksHandler::slotRegisteredNameFound,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::migrationEnded,
            this,
            &CallbacksHandler::slotMigrationEnded,
            Qt::QueuedConnection);

    connect(&VideoManager::instance(),
            &VideoManagerInterface::startedDecoding,
            this,
            &CallbacksHandler::slotStartedDecoding,
            Qt::QueuedConnection);

    connect(&VideoManager::instance(),
            &VideoManagerInterface::stoppedDecoding,
            this,
            &CallbacksHandler::slotStoppedDecoding,
            Qt::QueuedConnection);

    connect(&VideoManager::instance(),
            &VideoManagerInterface::deviceEvent,
            this,
            &CallbacksHandler::slotDeviceEvent,
            Qt::QueuedConnection);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::audioMeter,
            this,
            &CallbacksHandler::slotAudioMeterReceived,
            Qt::QueuedConnection);
}

CallbacksHandler::~CallbacksHandler()
{
}

void
CallbacksHandler::subscribeToDebugReceived()
{
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::debugMessageReceived,
            this,
            &CallbacksHandler::slotDebugMessageReceived,
            Qt::QueuedConnection);
}

void
CallbacksHandler::slotNewAccountMessage(const QString& accountId,
                                        const QString& msgId,
                                        const QString& from,
                                        const MapStringString& payloads)
{
    auto from2 = QString(from).replace("@ring.dht", "");
    emit newAccountMessage(accountId, msgId, from2, payloads);
}

void
CallbacksHandler::slotNewBuddySubscription(const QString& accountId,
                                           const QString& uri,
                                           bool status,
                                           const QString& message)
{
    Q_UNUSED(accountId)
    Q_UNUSED(status)
    Q_UNUSED(message)
    emit newBuddySubscription(uri, status);
}

void
CallbacksHandler::slotNearbyPeerSubscription(const QString& accountId,
                                             const QString& contactUri,
                                             int state,
                                             const QString& displayname)
{
    emit newPeerSubscription(accountId, contactUri, state, displayname);
}

void
CallbacksHandler::slotVoiceMailNotify(const QString& accountId, int newCount, int oldCount, int urgentCount)
{
    emit voiceMailNotify(accountId, newCount, oldCount, urgentCount);
}

void
CallbacksHandler::slotRecordPlaybackStopped(const QString& filePath)
{
    emit recordPlaybackStopped(filePath);
}

void
CallbacksHandler::slotContactAdded(const QString& accountId,
                                   const QString& contactUri,
                                   bool confirmed)
{
    emit contactAdded(accountId, contactUri, confirmed);
}

void
CallbacksHandler::slotContactRemoved(const QString& accountId,
                                     const QString& contactUri,
                                     bool banned)
{
    emit contactRemoved(accountId, contactUri, banned);
}

void
CallbacksHandler::slotIncomingContactRequest(const QString& accountId,
                                             const QString& ringId,
                                             const QByteArray& payload,
                                             time_t time)
{
    Q_UNUSED(time)
    emit incomingContactRequest(accountId, ringId, payload);
}

void
CallbacksHandler::slotIncomingCall(const QString &accountId, const QString &callId, const QString &fromUri)
{
    QString displayname;
    QString fromQString;
    if (fromUri.contains("ring.dht")) {
        auto qDisplayname = fromUri.left(fromUri.indexOf("<") + 1);
        if (qDisplayname.size() > 2) {
            displayname = qDisplayname.left(qDisplayname.indexOf("<") - 1);
        }
        fromQString = fromUri.right(50);
        fromQString = fromQString.left(40);
    } else {
        auto left = fromUri.indexOf("<") + 1;
        auto right = fromUri.indexOf("@");
        fromQString = fromUri.mid(left, right-left);
        displayname = fromUri.left(fromUri.indexOf("<") - 1);
    }
    emit incomingCall(accountId, callId, fromQString, displayname);
}

void
CallbacksHandler::slotCallStateChanged(const QString& callId, const QString& state, int code)
{
    emit callStateChanged(callId, state, code);
}

void
CallbacksHandler::slotAccountDetailsChanged(const QString& accountId,
                                            const MapStringString& details)
{
    emit accountDetailsChanged(accountId, details);
}

void
CallbacksHandler::slotVolatileAccountDetailsChanged(const QString& accountId,
                                                    const MapStringString& details)
{
    emit volatileAccountDetailsChanged(accountId, details);
}

void
CallbacksHandler::slotAccountsChanged()
{
    emit accountsChanged();
}

void
CallbacksHandler::slotRegistrationStateChanged(const QString& accountId,
                                               const QString& registration_state,
                                               unsigned detail_code,
                                               const QString& detail_str)
{
    (void) detail_code;
    (void) detail_str;
    emit accountStatusChanged(accountId, lrc::api::account::to_status(registration_state));
}

void
CallbacksHandler::slotIncomingMessage(const QString& callId,
                                      const QString& from,
                                      const MapStringString& interaction)
{
    QString from2;
    if (from.contains("@ring.dht")) {
        from2 = QString(from).replace("@ring.dht", "");
    } else {
        auto left = from.indexOf(":")+1;
        auto right = from.indexOf("@");
        from2 = from.mid(left, right-left);
    }

    for (auto& e : interaction.toStdMap()) {
        if (e.first.contains("x-ring/ring.profile.vcard")) {
            auto pieces0 = e.first.split( ";" );
            auto pieces1 = pieces0[1].split( "," );
            auto pieces2 = pieces1[1].split( "=" );
            auto pieces3 = pieces1[2].split( "=" );
            emit incomingVCardChunk(callId,
                                    from2,
                                    pieces2[1].toInt(),
                                    pieces3[1].toInt(),
                                    e.second);
        } else { // we consider it as an usual message interaction
            emit incomingCallMessage(callId, from2, e.second);
        }
    }
}

void
CallbacksHandler::slotConferenceCreated(const QString& callId)
{
    emit conferenceCreated(callId);
}

void
CallbacksHandler::slotConferenceChanged(const QString& callId, const QString& state)
{
    slotCallStateChanged(callId, state, 0);
}

void
CallbacksHandler::slotConferenceRemoved(const QString& callId)
{
    emit conferenceRemoved(callId);
}

void
CallbacksHandler::slotAccountMessageStatusChanged(const QString& accountId,
                                                  const uint64_t id,
                                                  const QString& to, int status)
{
    emit accountMessageStatusChanged(accountId, id,
                                     to, status);
}

void
CallbacksHandler::slotDataTransferEvent(qulonglong dringId, uint codeStatus)
{
    auto event = DRing::DataTransferEventCode(codeStatus);

    api::datatransfer::Info info;
    parent.getDataTransferModel().transferInfo(dringId, info);

    // WARNING: info.status could be INVALID in case of async signaling
    // So listeners must only take account of dringId in such case.
    // Is useful for "termination" status like unjoinable_peer.

    switch (event) {
    case DRing::DataTransferEventCode::created:
        emit transferStatusCreated(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::closed_by_host:
    case DRing::DataTransferEventCode::closed_by_peer:
        emit transferStatusCanceled(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::wait_peer_acceptance:
        emit transferStatusAwaitingPeer(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::wait_host_acceptance:
        emit transferStatusAwaitingHost(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::ongoing:
        emit transferStatusOngoing(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::finished:
        emit transferStatusFinished(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::invalid_pathname:
    case DRing::DataTransferEventCode::unsupported:
        emit transferStatusError(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::timeout_expired:
        emit transferStatusTimeoutExpired(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::unjoinable_peer:
        emit transferStatusUnjoinable(static_cast<long long>(dringId), info);
        break;
    case DRing::DataTransferEventCode::invalid:
        break;
    }
}

void
CallbacksHandler::slotKnownDevicesChanged(const QString& accountId,
                                          const MapStringString& devices)
{
    emit knownDevicesChanged(accountId, devices);
}

void
CallbacksHandler::slotDeviceRevokationEnded(const QString& accountId,
                                            const QString& deviceId,
                                            const int status)
{
    emit deviceRevocationEnded(accountId, deviceId, status);
}

void
CallbacksHandler::slotExportOnRingEnded(const QString& accountId, int status, const QString& pin)
{
    emit exportOnRingEnded(accountId, status, pin);
}

void
CallbacksHandler::slotNameRegistrationEnded(const QString& accountId, int status, const QString& name)
{
    emit nameRegistrationEnded(accountId, status, name);
}

void
CallbacksHandler::slotRegisteredNameFound(const QString& accountId, int status, const QString& address, const QString& name)
{
    emit registeredNameFound(accountId, status, address, name);
}

void
CallbacksHandler::slotMigrationEnded(const QString& accountId, const QString& status)
{
    emit migrationEnded(accountId, status == "SUCCESS");
}

void
CallbacksHandler::slotDebugMessageReceived(const QString& message)
{
    emit parent.getBehaviorController().debugMessageReceived(message);
}

void
CallbacksHandler::slotStartedDecoding(const QString& id, const QString& shmPath, int width, int height)
{
    emit startedDecoding(id, shmPath, width, height);
}

void
CallbacksHandler::slotStoppedDecoding(const QString& id, const QString& shmPath)
{
    emit stoppedDecoding(id, shmPath);
}

void
CallbacksHandler::slotDeviceEvent()
{
    emit deviceEvent();
}

void
CallbacksHandler::slotAudioMeterReceived(const QString& id, float level)
{
    emit audioMeter(id, level);
}

void
CallbacksHandler::slotRemoteRecordingChanged(const QString& callId, bool state)
{
    emit remoteRecordingChanged(callId, state);
}

} // namespace lrc

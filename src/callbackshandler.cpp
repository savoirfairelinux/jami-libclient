/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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

// Lrc
#include "account.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "namedirectory.h"

// DRing
#include <datatransfer_interface.h>

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
            &CallbacksHandler::slotNewAccountMessage);

    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::newBuddyNotification,
            this,
            &CallbacksHandler::slotNewBuddySubscription);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactAdded,
            this,
            &CallbacksHandler::slotContactAdded);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::contactRemoved,
            this,
            &CallbacksHandler::slotContactRemoved);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::incomingTrustRequest,
            this,
            &CallbacksHandler::slotIncomingContactRequest);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::accountMessageStatusChanged,
            this,
            &CallbacksHandler::slotAccountMessageStatusChanged);

    connect(&NameDirectory::instance(),
            &NameDirectory::registeredNameFound,
            this,
            &CallbacksHandler::slotRegisteredNameFound);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::registrationStateChanged,
            this,
            &CallbacksHandler::slotRegistrationStateChanged);

    connect(&CallManager::instance(),
            &CallManagerInterface::incomingCall,
            this,
            &CallbacksHandler::slotIncomingCall);

    connect(&CallManager::instance(),
            &CallManagerInterface::callStateChanged,
            this,
            &CallbacksHandler::slotCallStateChanged);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceCreated,
            this,
            &CallbacksHandler::slotConferenceCreated);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceRemoved,
            this,
            &CallbacksHandler::slotConferenceRemoved);

    connect(&CallManager::instance(),
            &CallManagerInterface::conferenceChanged,
            this,
            &CallbacksHandler::slotConferenceChanged);

    connect(&CallManager::instance(),
            &CallManagerInterface::incomingMessage,
            this,
            &CallbacksHandler::slotIncomingMessage);

    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::dataTransferEvent,
            this,
            &CallbacksHandler::slotDataTransferEvent);
}

CallbacksHandler::~CallbacksHandler()
{

}

void
CallbacksHandler::slotNewAccountMessage(const QString& accountId,
                                        const QString& from,
                                        const QMap<QString,QString>& payloads)
{
    std::map<std::string,std::string> stdPayloads;

    for (auto item : payloads.keys()) {
        stdPayloads[item.toStdString()] = payloads.value(item).toStdString();
    }

    auto accountId2 = accountId.toStdString();
    auto from2 = from.toStdString();

    emit newAccountMessage(accountId2, from2, stdPayloads);
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
    emit newBuddySubscription(uri.toStdString(), status);
}

void
CallbacksHandler::slotContactAdded(const QString& accountId,
                                   const QString& contactUri,
                                   bool confirmed)
{
    emit contactAdded(accountId.toStdString(), contactUri.toStdString(), confirmed);
}

void
CallbacksHandler::slotContactRemoved(const QString& accountId,
                                     const QString& contactUri,
                                     bool banned)
{
    emit contactRemoved(accountId.toStdString(), contactUri.toStdString(), banned);
}

void
CallbacksHandler::slotIncomingContactRequest(const QString& accountId,
                                             const QString& ringId,
                                             const QByteArray& payload,
                                             time_t time)
{
    Q_UNUSED(time)
    emit incomingContactRequest(accountId.toStdString(), ringId.toStdString(), payload.toStdString());
}

void
CallbacksHandler::slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                          const QString& address, const QString& name)
{
    if (!account) return;
    if (status == NameDirectory::LookupStatus::SUCCESS) {
        emit registeredNameFound(account->id().toStdString(), address.toStdString(), name.toStdString());
    }
}

void
CallbacksHandler::slotIncomingCall(const QString &accountId, const QString &callId, const QString &fromUri)
{
    if (fromUri.contains("ring.dht")) {
        auto fromQString = fromUri.right(50);
        fromQString = fromQString.left(40);
        emit incomingCall(accountId.toStdString(), callId.toStdString(), fromQString.toStdString());
    } else {
        auto left = fromUri.indexOf("<")+1;
        auto right = fromUri.indexOf("@");
        auto fromQString = fromUri.mid(left, right-left);

        emit incomingCall(accountId.toStdString(), callId.toStdString(), fromQString.toStdString());
    }
}

void
CallbacksHandler::slotCallStateChanged(const QString& callId, const QString& state, int code)
{
    emit callStateChanged(callId.toStdString(), state.toStdString(), code);
}

void
CallbacksHandler::slotRegistrationStateChanged(const QString& accountId,
                                               const QString& registration_state,
                                               unsigned detail_code,
                                               const QString& detail_str)
{
    (void) detail_code;
    (void) detail_str;
    emit accountStatusChanged(accountId.toStdString(), lrc::api::account::to_status(registration_state.toStdString()));
}

void
CallbacksHandler::slotIncomingMessage(const QString& callId,
                                      const QString& from,
                                      const QMap<QString,QString>& interaction)
{
    std::string from2;
    if (from.contains("ring.dht")) {
        from2 = from.left(40).toStdString();
    }
    else {
        auto left = from.indexOf(":")+1;
        auto right = from.indexOf("@");
        from2 = from.mid(left, right-left).toStdString();
    }

    for (auto& e : interaction.toStdMap()) {
        if (e.first.contains("x-ring/ring.profile.vcard")) {
            auto pieces0 = e.first.split( ";" );
            auto pieces1 = pieces0[1].split( "," );
            auto pieces2 = pieces1[1].split( "=" );
            auto pieces3 = pieces1[2].split( "=" );
            emit incomingVCardChunk(callId.toStdString(),
                                    from2,
                                    pieces2[1].toInt(),
                                    pieces3[1].toInt(),
                                    e.second.toStdString());
        } else { // we consider it as an usual message interaction
            emit incomingCallMessage(callId.toStdString(), from2, e.second.toStdString());
        }
    }
}

void
CallbacksHandler::slotConferenceCreated(const QString& callId)
{
    emit conferenceCreated(callId.toStdString());
}

void
CallbacksHandler::slotConferenceChanged(const QString& callId, const QString& state)
{
    slotCallStateChanged(callId, state, 0);
}

void
CallbacksHandler::slotConferenceRemoved(const QString& callId)
{
    emit conferenceRemoved(callId.toStdString());
}

void
CallbacksHandler::slotAccountMessageStatusChanged(const QString& accountId,
                                                  const uint64_t id,
                                                  const QString& to, int status)
{
    emit accountMessageStatusChanged(accountId.toStdString(), id,
                                     to.toStdString(), status);
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
    }
}

} // namespace lrc

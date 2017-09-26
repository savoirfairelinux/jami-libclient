/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#pragma once

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"
#include "namedirectory.h"

namespace lrc
{

namespace api
{
class Lrc;

namespace account
{
    enum class Status;
}
}

class CallbacksHandler : public QObject {
    Q_OBJECT

public:
    CallbacksHandler(const api::Lrc& parent);
    ~CallbacksHandler();

Q_SIGNALS:
    /**
     * Connect this signal to get incoming text interaction from the DHT.
     * @param accountId, interaction receiver.
     * @param from, interaction sender.
     * @param payloads.
     */
    void newAccountMessage(std::string& accountId,
                           std::string& from,
                           std::map<std::string,std::string> payloads) const;
    /**
     * Connect this signal to get information when a peer is online.
     * @param contactUri, the peer.
     * @param present, if the peer is online.
     */
    void newBuddySubscription(const std::string& contactUri, bool present) const;
    /**
     * Connect this signal to know when a contact is removed by the daemon.
     * @param accountId, the one who lost a contact.
     * @param contactUri, the contact removed.
     * @param banned, if the contact was banned
     */
    void contactRemoved(const std::string& accountId, const std::string& contactUri, bool banned) const;
    /**
     * Connect this signal to know when a contact is added by the daemon.
     * @param accountId, the one who got a new contact.
     * @param contactUri, the new contact.
     * @param confirmed, if the contact is trusted.
     */
    void contactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed) const;
    /**
     * Connect this signal to know when an incoming request is added by the daemon
     * @param accountId, the one who got the request
     * @param ringId the peer contact
     * @param payload the VCard
     */
    void incomingContactRequest(const std::string& accountId, const std::string& ringId, const std::string& payload);
    /**
     * Connect this signal to know when a call arrives
     * @param accountId, the one who receives the call
     * @param callId, the call id
     * @param fromUri, the caller uri
     */
    void incomingCall(const std::string& accountId, const std::string& callId, const std::string& fromUri);
    /**
     * Connect this signal to know when a call is updated
     * @param callId, the call id
     * @param state, the new state
     * @param code
     */
    void callStateChanged(const std::string& callId, const std::string &state, int code);
    /**
     * Connect this signal to know when the account status changed
     * @param accountId, the one who changes
     * @param status, the new status
     */
    void accountStatusChanged(const std::string& accountId, const api::account::Status status);
    /**
     * Connect this signal to know when a registeredName is found
     * @param accountId, the account who receives this signal
     * @param uri, the URI of the profile found
     * @param registeredName, the registeredName linked to this URI
     */
    void registeredNameFound(const std::string& accountId, const std::string& uri, const std::string& registeredName);
    /**
     * Connect this signal to know where a VCard is incoming
     * @param callId, the call linked to this VCard
     * @param from, the sender URI
     * @param part, the number of the part
     * @param numberOfParts of the VCard
     * @param payload, content of the VCard
     */
    void incomingVcardChunk(const std::string& callId, const std::string& from, int part, int numberOfParts, const std::string& payload);
    /**
     * Connect this signal to get incoming text interaction from SIP.
     * @param callId, the call linked.
     * @param from, interaction sender.
     * @param body, the text received.
     */
    void incomingCallMessage(const std::string& callId, const std::string& from, const std::string& body) const;

private Q_SLOTS:
    /**
     * Emit newAccountMessage
     * @param accountId
     * @param from
     * @param payloads of the interaction
     */
    void slotNewAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads);
    /**
     * Emit newBuddySubscription
     * @param accountId
     * @param contactUri
     * @param status, if the contact is present
     * @param message, unused for now
     */
    void slotNewBuddySubscription(const QString& accountId, const QString& contactUri, bool status, const QString& message);
    /**
     * Emit contactAdded
     * @param accountId, account linked
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const QString& accountId, const QString& contactUri, bool confirmed);
    /**
     * Emit contactRemoved
     * @param accountId, account linked
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const QString& accountId, const QString& contactUri, bool banned);
    /**
     * Emit incomingContactRequest
     * @param accountId, the linked id
     * @param ringId, the peer contact
     * @param payload, the VCard
     * @param time, when the request was received
     */
    void slotIncomingContactRequest(const QString& accountId, const QString& ringId, const QByteArray& payload, time_t time);
    /**
     * Emit accountStatusChanged
     * @param accountId
     * @param registration_state
     * @param detail_code
     * @param detail_str
     */
    void slotRegistrationStateChanged(const QString& accountId, const QString& registration_state, unsigned detail_code, const QString& detail_str);
    /**
     * Listen from the daemon when a profile is found.
     * @param account, account linked.
     * @param status,  if the method succeeds
     * @param address, the URI of the profile
     * @param name, registeredName
     */
    void slotRegisteredNameFound(const Account* account, NameDirectory::LookupStatus status,
                                 const QString& address, const QString& name);
    /**
     * Get the URI of the peer and emit incomingCall
     * @param accountId, account linked
     * @param callId, the incoming call id
     * @param fromQString, the uri of the peer
     */
    void slotIncomingCall(const QString &accountId, const QString &callId, const QString &fromUri);
    /**
     * Emit callStateChanged
     * @param callId, the call which changes.
     * @param state, the new state
     * @param code, unused for now
     */
    void slotCallStateChanged(const QString& callId, const QString &state, int code);
    /**
     * Parse a call message and emit incomingVcardChunk if it's a VCard chunk
     * else incomingCallMessage if it's a text message
     * @param callId, call linked
     * @param from, the URI
     * @param interaction, the content of the Message.
     */
    void slotIncomingMessage(const QString& callId, const QString& from, const QMap<QString,QString>& interaction);

private:
    const api::Lrc& parent;
};

} // namespace lrc

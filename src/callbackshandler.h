/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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

namespace lrc
{

namespace api
{
class Lrc;
}

class CallbacksHandler : public QObject {
    Q_OBJECT

public:
    CallbacksHandler(const api::Lrc& parent);
    ~CallbacksHandler();

Q_SIGNALS:
    /**
     * Connect this signal to get incoming message.
     * @param accountId, message reciever.
     * @param from, message sender.
     * @param payloads.
     */
    void NewAccountMessage(const std::string& accountId,
                           const std::string& toStdString,
                           const std::map<std::string,std::string> payloads) const;
    /**
     * Connect this signal to get information when a peer is online.
     * @param contactUri, the peer.
     */
    void NewBuddySubscription(const std::string& contactUri) const;
    /**
     * Connect this signal to know when a contact was removed by the daemon.
     * @param accountId, the one who lost a contact.
     * @param contactUri, the contact removed.
     * @param banned, true if the contact was banned
     */
    void contactRemoved(const std::string& accountId, const std::string& contactUri, bool banned) const;
    /**
     * Connect this signal to know when a contact was addeb by the daemon.
     * @param accountId, the one who got a new contact.
     * @param contactUri, the new contact.
     * @param confirmed, true if the contact is trusted.
     */
    void contactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed) const;
    // TODO: add docs
    void incomingContactRequest(const std::string& accountId, const std::string& ringID, const std::string& payload);
    void incomingCall(const std::string& accountID, const std::string& callID, const std::string& fromId);
    void callStateChanged(const std::string& callId, const std::string &state, int code);

private Q_SLOTS:
    /**
     * Add the incoming message from the daemon to the database
     * @param accountId
     * @param from
     * @param payloads of the message
     */
    void slotNewAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads);
    /**
     * Update the presence of a contact for an account
     * @param accountId
     * @param contactUri
     * @param status if the contact is present
     * @param message unused for now
     */
    void slotNewBuddySubscription(const QString& accountId, const QString& contactUri, bool status, const QString& message);
    /**
     * Add a contact in the contact list of an account
     * @param accountId
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const QString& accountId, const QString& contactUri, bool confirmed);
    /**
     * Remove a contact from a contact list of an account
     * @param accountId
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const QString& accountId, const QString& contactUri, bool banned);

    // TODO: add docs
    void slotIncomingContactRequest(const QString& accountId, const QString& ringID, const QByteArray& payload, time_t time);

private:
    const api::Lrc& parent;
};

} // namespace lrc

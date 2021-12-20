/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
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
#pragma once

#include "typedefs.h"
#include "api/behaviorcontroller.h"

#include <QObject>

#include <memory>

namespace lrc {

class CallbacksHandler;
class Database;
class ContactModelPimpl;

namespace api {

namespace contact {
struct Info;
}
namespace account {
struct Info;
}
namespace datatransfer {
struct Info;
}
class NewAccountModel;
class ConversationModel;

/**
 *  @brief Class that manages contact information associated to an account.
 */
class LIB_EXPORT ContactModel : public QObject
{
    Q_OBJECT
public:
    using ContactInfoMap = QMap<QString, contact::Info>;

    const account::Info& owner;

    ContactModel(const account::Info& owner,
                 Database& db,
                 const CallbacksHandler& callbacksHandler,
                 const BehaviorController& behaviorController);
    ~ContactModel();

    /**
     * Ask the daemon to add a contact.
     * @param contactInfo
     */
    void addContact(contact::Info contactInfo);
    /**
     * Add a pending item into the contact list
     * @param contactUri
     */
    void addToContacts(const QString& contactUri);
    /**
     * Ask the daemon to remove a contact.
     * @param contactUri
     * @param banned
     */
    void removeContact(const QString& contactUri, bool banned = false);
    /**
     * get contact information.
     * @param  contactUri
     * @return the contact::Info structure for a contact
     * @throws out_of_range exception if can't find the contact
     */
    const contact::Info getContact(const QString& contactUri) const;
    ContactInfoMap getSearchResults() const;

    /**
     * Retrieve when a contact is added
     */
    time_t getAddedTs(const QString& contactUri) const;
    /**
     * get list of banned contacts.
     * @return list of banned contacts uris as string
     */
    const QList<QString>& getBannedContacts() const;
    /**
     * @return all contacts for this account.
     */
    const ContactInfoMap& getAllContacts() const;

    void searchContact(const QString& query);
    /**
     * Send a text interaction to a contact over the Dht.
     * @param contactUri
     * @param body
     * @return id from daemon
     */
    uint64_t sendDhtMessage(const QString& uri, const QString& body) const;
    /**
     * Get best id for contact
     * @param contactUri
     */
    const QString bestIdForContact(const QString& contactUri) const;
    /**
     * Get best name for contact
     * @param contactUri
     */
    const QString bestNameForContact(const QString& contactUri) const;

    /**
     * Get avatar from storage
     * @param contactUri
     */
    QString avatar(const QString& avatar) const;

Q_SIGNALS:
    /**
     * Connect this signal to know when this model was updated.
     */
    void modelUpdated(const QString& uri) const;
    /**
     * Connect this signal to know when a contact was added.
     * @param contactUri
     */
    void contactAdded(const QString& contactUri) const;
    /**
     * Connect this signal to know when a contact request received
     * @param contactUri
     */
    void incomingContactRequest(const QString& contactUri) const;
    /**
     * Connect this signal to know when a pending contact was accepted.
     * @param contactUri
     */
    void pendingContactAccepted(const QString& contactUri) const;
    /**
     * Connect this signal to know when an account was removed.
     * @param contactUri
     */
    void contactRemoved(const QString& contactUri) const;
    /**
     * Connect this signal to know when a call is incoming.
     * @param fromId peer profile uri
     * @param callId incoming call id
     */
    void incomingCall(const QString& from, const QString& callId) const;
    /**
     * Connect this signal to know when a text message arrives for this account
     * @param accountId
     * @param msgId     Interaction's id
     * @param from peer uri
     * @param payloads content of the message
     */
    void newAccountMessage(const QString& accountId,
                           const QString& from,
                           const QString& msgId,
                           const MapStringString& payloads) const;
    /**
     * Connect this signal to know when a file transfer interaction is incoming
     * @param fileId Daemon's ID for incoming transfer
     * @param transferInfo DataTransferInfo structure from daemon
     */
    void newAccountTransfer(const QString& fileId, datatransfer::Info info) const;
    /**
     * Connect this signal to know when a contact is banned or unbanned
     * @param contactUri
     * @param banned whether contact was banned or unbanned
     */
    void bannedStatusChanged(const QString& contactUri, bool banned) const;
    /**
     * Connect this signal to know when contact profile was updated.
     * @param contactUri
     */
    void profileUpdated(const QString& contactUri) const;

private:
    /**
     * Get best id from contact info
     * @param contactInfo
     */
    const QString bestIdFromContactInfo(const contact::Info& contactInfo) const;

    std::unique_ptr<ContactModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::ContactModel*)

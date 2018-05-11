/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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

// Std
#include <string>
#include <memory>
#include <map>

// Qt
#include <qobject.h>

// LRC
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class Database;
class ContactModelPimpl;

namespace api
{

namespace contact { struct Info; }
namespace account { struct Info; }
namespace datatransfer { struct Info; }
class NewAccountModel;
class ConversationModel;

/**
  *  @brief Class that manages contact information associated to an account.
  */
class LIB_EXPORT ContactModel : public QObject {
    Q_OBJECT
public:
    using ContactInfoMap = std::map<std::string, contact::Info>;

    const account::Info& owner;

    ContactModel(const account::Info& owner,
                 Database& database,
                 const CallbacksHandler& callbacksHandler);
    ~ContactModel();

    /**
     * Ask the daemon to add a contact.
     * @param contactInfo
     */
    void addContact(contact::Info contactInfo);
    /**
     * Ask the daemon to remove a contact.
     * @param contactUri
     * @param banned
     */
    void removeContact(const std::string& contactUri, bool banned=false);
    /**
     * get contact information.
     * @param  contactUri
     * @return the contact::Info structure for a contact
     * @throws out_of_range exception if can't find the contact
     */
    const contact::Info getContact(const std::string& contactUri) const;
    /**
     * get list of banned contacts.
     * @return list of banned contacts uris as string
     */
    const std::list<std::string>& getBannedContacts() const;
    /**
     * @param  contactUri
     * @return empty string if no contact, else the uri in db
     */
    const std::string getContactProfileId(const std::string& contactUri) const;
    /**
     * @return all contacts for this account.
     */
    const ContactInfoMap& getAllContacts() const;
    /**
     * @return if pending requests exists.
     */
    bool hasPendingRequests() const;
    /**
     * @return number of pending requests
     */
    int pendingRequestCount() const;
    /**
     * Search a SIP or a Ring contact from a query.
     * @param query
     */
    void searchContact(const std::string& query);
    /**
     * Send a text interaction to a contact over the Dht.
     * @param contactUri
     * @param body
     * @return id from daemon
     */
    uint64_t sendDhtMessage(const std::string& uri, const std::string& body) const;

Q_SIGNALS:
    /**
     * Connect this signal to know when this model was updated.
     */
    void modelUpdated(const std::string& uri, bool needsSorted=true) const;
    /**
     * Connect this signal to know when a contact was added.
     * @param contactUri
     */
    void contactAdded(const std::string& contactUri) const;
    /**
     * Connect this signal to know when a pending contact was accepted.
     * @param contactUri
     */
    void pendingContactAccepted(const std::string& contactUri) const;
    /**
     * Connect this signal to know when an account was removed.
     * @param contactUri
     */
    void contactRemoved(const std::string& contactUri) const;
    /**
     * Connect this signal to know when an incoming call comes from a pending contact.
     * @param fromId peer profile uri
     * @param callId incoming call id
     */
    void incomingCallFromPending(const std::string& from, const std::string& callId) const;
    /**
     * Connect this signal to know when a text message arrives for this account
     * @param accountId
     * @param from peer uri
     * @param payloads content of the message
     */
    void newAccountMessage(std::string& accountId, std::string& from, std::map<std::string,std::string> payloads) const;
    /**
     * Connect this signal to know when a file transfer interaction is incoming
     * @param dringId Daemon's ID for incoming transfer
     * @param transferInfo DataTransferInfo structure from daemon
     */
    void newAccountTransfer(long long dringId, datatransfer::Info info) const;

private:
    std::unique_ptr<ContactModelPimpl> pimpl_;

};

} // namespace api
} // namespace lrc

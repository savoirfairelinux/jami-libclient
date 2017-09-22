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
     * @param contactUri
     */
    //~ void addContact(const std::string& contactUri);
    void addContact(contact::Info contactInfo);
    /**
     * Ask the daemon to remove a contact
     * @param contactUri
     * @param banned
     */
    void removeContact(const std::string& contactUri, bool banned=false);
    /**
     * @param  contactUri
     * @return the contact::Info structure for a contact
     * @throws out_of_range exception if can't find the contact
     */
    const contact::Info getContact(const std::string& contactUri) const;
    /**
     * @return all contacts for this account
     */
    const ContactInfoMap& getAllContacts() const;
    /**
     * @return if pending requests exists
     */
    bool hasPendingRequests() const;
    /**
     * Search a SIP or a Ring contact from a query
     * @param query
     */
    void searchContact(const std::string& query);
    /**
     * Send a text message to a contact over the Dht
     * @param contactUri
     * @param body
     */
    void sendDhtMessage(const std::string& uri, const std::string& body) const;

Q_SIGNALS:
    void modelUpdated() const;
    void contactAdded(const std::string& contactUri) const;
    void contactRemoved(const std::string& contactUri) const;
    void incomingCallFromPending(const std::string& fromId, const std::string& callId) const;

private:
    std::unique_ptr<ContactModelPimpl> pimpl_;

};

} // namespace api
} // namespace lrc

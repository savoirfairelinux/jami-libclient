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

// Data
#include "data/contact.h"
#include "data/account.h"

namespace lrc
{

class NewAccountModel;
class ConversationModel;
class Database;

class ContactModel : public QObject {
    Q_OBJECT

    friend class NewAccountModel;
    friend class ConversationModel;

public:
    const account::Info& owner;

    ~ContactModel();

    /**
     * Ask the daemon to add a contact.
     * @param contactUri
     */
    void addContact(const std::string& contactUri);
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
    const contact::Info& getContact(const std::string& contactUri);
    /**
     * @return all contacts for this account
     */
    const ContactsInfoMap& getAllContacts() const;

    // TODO
    void nameLookup(const std::string& uri) const;
    void addressLookup(const std::string& name) const;

Q_SIGNALS:
    void contactsChanged();

private:
    explicit ContactModel(NewAccountModel& parent,
                          const Database& db,
                          const account::Info& info);
    /**
     * Fills with contacts from daemon
     * @return if the method succeeds
     */
    bool fillsWithContacts();
    /**
     * Update the presence status of a contact
     * @param contactUri
     * @param status
     */
    void setContactPresent(const std::string& uri, bool status);
    /**
     * @param contactUri
     * @param confirmed]
     */
    void slotContactAdded(const std::string& contactUri, bool confirmed);
    /**
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const std::string& contactUri, bool banned);
    /**
     * Send a text message to a contact
     * @param contactUri
     * @param body
     */
    void sendMessage(const std::string& contactUri, const std::string& body) const;

    ContactsInfoMap contacts_;
    const Database& db_;
    NewAccountModel& parent_;

};

}

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

// Qt
#include <qobject.h>

// Data
#include "api/account.h"
#include "api/contact.h"

namespace lrc
{

class CallbacksHandler;
class Database;

namespace api
{

class NewAccountModel;
class ConversationModel;
class ContactModelPimpl;


using ContactInfoMap = std::map<std::string, std::shared_ptr<contact::Info>>;

class ContactModel : public QObject {
    Q_OBJECT
public:
    const account::Info& owner;

    ContactModel(NewAccountModel& parent,
                 const Database& database,
                 const CallbacksHandler& callbacksHandler,
                 const account::Info& info);
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
    const ContactInfoMap& getAllContacts() const;

    // TODO
    void nameLookup(const std::string& uri) const;
    void addressLookup(const std::string& name) const;

Q_SIGNALS:
    void contactsChanged();

private:
    std::unique_ptr<ContactModelPimpl> pimpl_;

};

} // namespace api
} // namespace lrc

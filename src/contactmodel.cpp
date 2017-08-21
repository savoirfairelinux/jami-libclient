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
#include "api/contactmodel.h"

// Models and database
#include "database.h"
#include "api/newaccountmodel.h"

namespace lrc
{

namespace api
{

class ContactModelPimpl
{
public:
    ContactModelPimpl(NewAccountModel& p, const Database& d);
    ~ContactModelPimpl();
    ContactModelPimpl(const ContactModelPimpl& contactModelPimpl);

    bool fillsWithContacts();
    void sendMessage(const std::string& uri, const std::string& body) const;
    void setContactPresent(const std::string& uri, bool status);

    const Database& db;
    ContactInfoMap contacts;
    NewAccountModel& parent;

public Q_SLOTS:
    // TODO remove this from here when LRC signals are added
    void slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed);
    void slotContactsRemoved(const QString &accountID, const QString &uri, bool status);
};


ContactModel::ContactModel(NewAccountModel& parent, const Database& database, const account::Info& info)
: pimpl_(std::make_unique<ContactModelPimpl>(parent, database))
, owner(info)
{

}

ContactModel::~ContactModel()
{

}

const contact::Info&
ContactModel::getContact(const std::string& uri) const
{
    return {};
}

const ContactInfoMap&
ContactModel::getAllContacts() const
{
    return pimpl_->contacts;
}

void
ContactModel::addContact(const std::string& uri)
{

}

void
ContactModel::removeContact(const std::string& uri)
{

}

void
ContactModel::nameLookup(const std::string& uri) const
{

}

void
ContactModel::addressLookup(const std::string& name) const
{

}

ContactModelPimpl::ContactModelPimpl(NewAccountModel& p, const Database& d)
: db(d)
, parent(p)
{

}

ContactModelPimpl::~ContactModelPimpl()
{

}

ContactModelPimpl::ContactModelPimpl(const ContactModelPimpl& contactModelPimpl)
: db(contactModelPimpl.db)
, contacts(contactModelPimpl.contacts)
, parent(contactModelPimpl.parent)
{

}

void
ContactModelPimpl::sendMessage(const std::string& uri, const std::string& body) const
{

}

bool
ContactModelPimpl::fillsWithContacts()
{
    return false;
}

void
ContactModelPimpl::setContactPresent(const std::string& uri, bool status)
{

}

void
ContactModelPimpl::slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed)
{

}

void
ContactModelPimpl::slotContactsRemoved(const QString &accountID, const QString &uri, bool status)
{

}

} // namespace api
} // namespace lrc

#include "api/moc_contactmodel.cpp"

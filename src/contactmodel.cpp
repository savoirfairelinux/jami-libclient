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
#include "contactmodel.h"

#include "availableaccountmodel.h"
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"
#include "contactmethod.h"

namespace lrc
{

ContactModel::ContactModel(const DatabaseManager& dbm, const std::string& accountId)
: QObject(), dbm_(dbm), accountId_(accountId)
{

}

ContactModel::~ContactModel()
{

}

const contact::Info&
ContactModel::addContact(const std::string& uri)
{
    return contact::Info();
}

void
ContactModel::removeContact(const std::string& uri)
{

}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{

}

const contact::Info&
ContactModel::getContact(const std::string& uri)
{
    return contact::Info();
}

const ContactsInfoMap&
ContactModel::getAllContacts() const
{
    return contacts_;
}

void
ContactModel::nameLookup(const std::string& uri) const
{

}

void
ContactModel::addressLookup(const std::string& name) const
{

}

bool
ContactModel::fillsWithContacts()
{
    return false;
}

void
ContactModel::slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status, const QString& message)
{

}

} // namespace lrc

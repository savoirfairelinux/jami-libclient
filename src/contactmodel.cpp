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

ContactModel::ContactModel(QObject* parent)
: QObject(parent)
{

}

ContactModel::~ContactModel()
{

}

const Contact::Info&
ContactModel::addContact(const std::string& uri)
{
    return Contact::Info();
}

void
ContactModel::removeContact(const std::string& uri)
{

}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{

}

const Contact::Info&
ContactModel::getContact(const std::string& uri/*TODO: add type*/) const
{
    return Contact::Info();
}

const ContactsInfo&
ContactModel::getContacts() const
{
    return ContactsInfo();
}

void
ContactModel::nameLookup(const std::string& uri) const
{

}

void
ContactModel::addressLookup(const std::string& name) const
{

}

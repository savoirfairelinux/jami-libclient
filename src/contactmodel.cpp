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

#include "dbus/configurationmanager.h"
#include "contactmethod.h"

namespace lrc
{

ContactModel::ContactModel(QObject* parent)
: QObject(parent)
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

bool
ContactModel::isAContact(const std::string& uri) const
{
    auto i = std::find_if(contacts_.begin(), contacts_.end(),
    [uri](const std::pair<std::string, std::shared_ptr<contact::Info>>& contact) {
        return contact.second->uri == uri;
    });
    return (i != contacts_.end());
}

void
ContactModel::removeContact(const std::string& uri)
{

}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{

}

std::shared_ptr<contact::Info>
ContactModel::getContact(const std::string& uri)
{
    return std::shared_ptr<contact::Info>();
}

const ContactsInfoMap&
ContactModel::getContacts() const
{
    return ContactsInfoMap();
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
    if (account_->protocol() != Account::Protocol::RING) {
        qDebug() << "fillsWithContacts, account is not a RING account";
        return false;
    }

    auto contacts = account_->getContacts();

    // Clear the list
    contacts_.clear();

    auto type = contact::Type::RING;

    // Add contacts to the list
    for (auto c : contacts) {
        auto uri = c->uri().toStdString();
        auto avatar = dbm_->getAvatar(uri);
        auto registeredName = c->registeredName().toStdString();
        auto alias = c->bestName().toStdString();
        auto isTrusted = false; // TODO: handle trust
        auto isPresent = c->isPresent();

        auto contact = std::make_shared<contact::Info>();
        contact->uri = uri;
        contact->avatar = avatar;
        contact->registeredName = registeredName;
        contact->alias = alias;
        contact->isTrusted = isTrusted;
        contact->isPresent = isPresent;
        contact->type = type;

        contacts_[uri] = contact;
    }

    return true;
}

}

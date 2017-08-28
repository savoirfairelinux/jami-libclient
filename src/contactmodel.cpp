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

// Std
#include <stdexcept>

// Data
#include "data/message.h"

// Models and database
#include "database.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

// Lrc
#include "availableaccountmodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

namespace lrc
{

ContactModel::ContactModel(NewAccountModel& parent, const Database& db, const account::Info& info)
: parent_(parent)
, db_(db)
, owner(info)
, QObject()
{
    fillsWithContacts();
}

ContactModel::~ContactModel()
{

}

void
ContactModel::addContact(const std::string& contactUri)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()));
}

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()), banned);
}

const contact::Info&
ContactModel::getContact(const std::string& contactUri)
{
    auto contact = contacts_.find(contactUri);
    if (contact == contacts_.end()) {
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);
    }
    return *contact->second.get();
}

const ContactsInfoMap&
ContactModel::getAllContacts() const
{
    return contacts_;
}

void
ContactModel::nameLookup(const std::string& uri) const
{
    // TODO
}

void
ContactModel::addressLookup(const std::string& name) const
{
    // TODO
}

bool
ContactModel::fillsWithContacts()
{
    // TODO: For now, we only get contacts for RING accounts.
    // In the future, we will directly get contacts from daemon (or SIP)
    // and avoid "account->getContacts();"
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::fillsWithContacts(), nullptr";
    }
    if (account->protocol() != Account::Protocol::RING) {
        qDebug() << "fillsWithContacts, account is not a RING account";
        return false;
    }

    // Clear current contacts
    contacts_.clear();

    // Add contacts
    auto contacts = account->getContacts();
    for (auto c : contacts) {
        auto contact = std::make_shared<contact::Info>();
        auto contactUri = c->uri().toStdString();
        contact->uri = contactUri;
        contact->avatar = db_.getContactAttribute(contactUri, "photo");
        contact->registeredName = db_.getContactAttribute(contactUri, "username");
        contact->alias = db_.getContactAttribute(contactUri, "alias");
        contact->isTrusted = c->isConfirmed();
        contact->isPresent = c->isPresent();
        switch (owner.type)
        {
        case account::Type::RING:
            contact->type = contact::Type::RING;
            break;
        case account::Type::SIP:
            contact->type = contact::Type::SIP;
            break;
        case account::Type::INVALID:
        default:
            contact->type = contact::Type::INVALID;
            break;
        }

        contacts_[contactUri] = contact;
    }

    return true;
}

void
ContactModel::setContactPresent(const std::string& contactUri, bool status)
{
    if (contacts_.find(contactUri) != contacts_.end()) {
        contacts_[contactUri]->isPresent = status;
    }
}

void
ContactModel::slotContactAdded(const std::string& contactUri, bool confirmed)
{
    // TODO: For now, we only get contacts for RING accounts.
    // Moreover, we still use ContactMethod class.
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    if (contacts_.find(contactUri) == contacts_.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        auto contact = std::make_shared<contact::Info>();
        contact->uri = contactUri;
        contact->avatar = db_.getContactAttribute(contactUri, "photo");
        contact->registeredName = db_.getContactAttribute(contactUri, "username");
        contact->alias = db_.getContactAttribute(contactUri, "alias");
        contact->isTrusted = confirmed;
        contact->isPresent = cm->isPresent();
        switch (owner.type)
        {
        case account::Type::RING:
            contact->type = contact::Type::RING;
            break;
        case account::Type::SIP:
            contact->type = contact::Type::SIP;
            break;
        case account::Type::INVALID:
        default:
            contact->type = contact::Type::INVALID;
            break;
        }

        contacts_[contactUri] = contact;

        // Add to database
        message::Info msg;
        msg.contact = contactUri;
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        db_.addMessage(owner.id, msg);
        emit contactsChanged();
    }
}

void
ContactModel::slotContactRemoved(const std::string& contactUri, bool banned)
{
    // TODO handle banned contacts
    if (contacts_.find(contactUri) != contacts_.end()) {
        db_.clearHistory(owner.id, contactUri, true);
        contacts_.erase(contactUri);
        emit contactsChanged();
    }
}

void
ContactModel::sendMessage(const std::string& contactUri, const std::string& body) const
{
    // Send message
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();
    ConfigurationManager::instance().sendTextMessage(QString(owner.id.c_str()),
                                                     QString(contactUri.c_str()),
                                                     payloads);

    // Store it into the database
    message::Info msg;
    msg.contact = contactUri;
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;
    db_.addMessage(owner.id, msg);
}

} // namespace lrc

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

// std
#include <stdexcept>

// lrc
#include "availableaccountmodel.h"
#include "dbus/configurationmanager.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

namespace lrc
{

ContactModel::ContactModel(const Database& db, const std::string& accountId)
: QObject(), db_(db), accountId_(accountId)
{
    fillsWithContacts();

    // TODO move into LRC
    connect(&ConfigurationManager::instance(), &ConfigurationManagerInterface::contactAdded, this,
            &ContactModel::slotContactsAdded);
    connect(&ConfigurationManager::instance(), &ConfigurationManagerInterface::contactRemoved, this,
            &ContactModel::slotContactsRemoved);

}

ContactModel::~ContactModel()
{

}

void
ContactModel::addContact(const std::string& uri)
{
    // Add contact to daemon
    ConfigurationManager::instance().addContact(QString(accountId_.c_str()),
    QString(uri.c_str()));
    // TODO connect signal from LRC
}

void
ContactModel::removeContact(const std::string& uri)
{
    // Remove contact from daemon contacts
    ConfigurationManager::instance().removeContact(QString(accountId_.c_str()), QString(uri.c_str()), false);
    // TODO do this when daemon emit contactRemoved
    contacts_.erase(uri);
}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{
    // Send message
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();
    unsigned int id = ConfigurationManager::instance().sendTextMessage(QString(accountId_.c_str()), uri.c_str(), payloads);

    // Store it into the database
    message::Info msg;
    msg.uid = std::to_string(id);
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;
    db_.addMessage(accountId_, msg);
}

const contact::Info&
ContactModel::getContact(const std::string& uri)
{
    auto contact = contacts_.find(uri);
    if (contact == contacts_.end()) {
        throw std::out_of_range("ContactModel::getContact, invalid uri");
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

void
ContactModel::setContactPresent(const std::string& uri, bool status)
{
    if (contacts_.find(uri) != contacts_.end()) {
        contacts_[uri]->isPresent = status;
    }
}

bool
ContactModel::fillsWithContacts()
{
    auto account = AccountModel::instance().getById(accountId_.c_str());
    if (not account) {
        qDebug() << "ContactModel::fillsWithContacts(), nullptr";
    }
    // TODO improve this
    if (account->protocol() != Account::Protocol::RING) {
        qDebug() << "fillsWithContacts, account is not a RING account";
        return false;
    }

    auto contacts = account->getContacts();

    // Clear the list
    contacts_.clear();

    auto type = contact::Type::RING;

    // Add contacts to the list
    for (auto c : contacts) {
        auto uri = c->uri().toStdString();
        auto avatar = db_.getContactAttribute(uri, "photo");
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

void
ContactModel::slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed)
{
    auto account = AccountModel::instance().getById(accountId_.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    // TODO disconnect signal from LRC
    if (contacts_.find(uri.toStdString()) == contacts_.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(uri, account);
        auto contact = std::make_shared<contact::Info>();
        contact->uri = uri.toStdString();
        contact->avatar = db_.getContactAttribute(uri.toStdString(), "photo");
        contact->registeredName = cm->registeredName().toStdString();
        contact->alias =  cm->bestName().toStdString();
        contact->isTrusted = false;
        contact->isPresent = cm->isPresent();
        contact->type = contact::Type::RING; // TODO

        contacts_[uri.toStdString()] = contact;

        // Add to database
        message::Info msg;
        msg.uid = uri.toStdString();
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        db_.addMessage(accountId_, msg);
        emit contactsChanged();
    }
}

void
ContactModel::slotContactsRemoved(const QString &accountID, const QString &uri, bool status)
{
    if (accountId_ != accountID.toStdString()) return;
    // TODO disconnect signal from LRC
    if (contacts_.find(uri.toStdString()) != contacts_.end()) {
        db_.clearHistory(accountId_, uri.toStdString(), true);
        contacts_.erase(uri.toStdString());
        emit contactsChanged();
    }
}

} // namespace lrc

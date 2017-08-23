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

// Lrc
#include "availableaccountmodel.h" // old
#include "contactmethod.h" // old
#include "dbus/callmanager.h" // old
#include "dbus/configurationmanager.h" // old


ContactModel::ContactModel(const std::shared_ptr<DatabaseManager> dbm, const Account* account, QObject* parent)
: dbm_(dbm), account_(account), QObject(parent)
{
    fillsWithContacts();
}

ContactModel::~ContactModel()
{
}

const Contact::Info&
ContactModel::addContact(const std::string& uri)
{
    auto avatar = dbm_->getAvatar("");
    auto registeredName = "feature missing"; // TODO add function in database
    auto alias = dbm_->getAlias("");
    auto isTrusted = false;
    auto type = Contact::Type::RING;

    auto contactInfo = std::make_shared<Contact::Info>(uri, avatar, registeredName, alias, isTrusted, type);

    contacts_[uri] = contactInfo;
}

void
ContactModel::removeContact(const std::string& uri)
{
    contacts_.erase(uri);
}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    unsigned int id = ConfigurationManager::instance().sendTextMessage(account_->id(), uri.c_str(), payloads);

    auto accountId = account_->id().toStdString();
    auto message = Message::Info(std::to_string(id), body, true, Message::Type::TEXT);

    dbm_->addMessage(accountId, message);
}

std::shared_ptr<Contact::Info>
ContactModel::getContact(const std::string& uri)
{
    return contacts_[uri];
}

const ContactsInfo&
ContactModel::getContacts() const
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
    if (account_->protocol() != Account::Protocol::RING) {
        qDebug() << "fillsWithContacts, account is not a RING account";
        return false;
    }

    auto contacts = account_->getContacts();

    // Clear the list
    contacts_.clear();

    auto type = Contact::Type::RING;

    // Add contacts to the list
    for (auto c : contacts) {
        auto uri = c->uri().toStdString();
        auto avatar = dbm_->getAvatar(uri);
        auto registeredName = c->registeredName().toStdString();
        auto alias = c->bestName().toStdString();
        auto isTrusted = false; // TODO: handle trust
        auto isPresent = c->isPresent();

        auto contact = std::shared_ptr<Contact::Info>(new Contact::Info(uri,
                                                                        avatar,
                                                                        registeredName,
                                                                        alias,
                                                                        isTrusted,
                                                                        type));

        contacts_[uri] = contact;
    }

    return true;
}

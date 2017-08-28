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
#include "dbus/presencemanager.h" // old


ContactModel::ContactModel(const std::shared_ptr<DatabaseManager> dbm, const Account* account, QObject* parent)
: dbm_(dbm), account_(account), QObject(parent)
{
    fillsWithContacts();

    connect(&PresenceManager::instance(),
            SIGNAL(newBuddyNotification(QString,QString,bool,QString)),
            this,
            SLOT(slotNewBuddySubscription(QString,QString,bool,QString)));
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
    auto isPresent = false;
    auto type = Contact::Type::RING;

    auto contactInfo = std::make_shared<Contact::Info>(uri, avatar, registeredName, alias, isTrusted, isPresent, type);

    contacts_[uri] = contactInfo;

    ConfigurationManager::instance().addContact(account_->id(),
    QString(uri.c_str()));

    lrc::message::Info msg;
    msg.uid = uri.c_str();
    msg.body = "";
    msg.timestamp = std::time(nullptr);
    msg.isOutgoing = true;
    msg.type = lrc::message::Type::CONTACT;
    msg.status = lrc::message::Status::SUCCEED;
    dbm_->addMessage(account_->id().toStdString(), msg);
}

void
ContactModel::slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status, const QString& message)
{
    if (accountId != account_->id()) return;
    if (contacts_.find(uri.toStdString()) != contacts_.end()) {
        contacts_[uri.toStdString()]->isPresent_ = status;
    }
}

bool
ContactModel::isAContact(const std::string& uri) const
{
    auto i = std::find_if(contacts_.begin(), contacts_.end(),
    [uri](const std::pair<std::string, std::shared_ptr<Contact::Info>>& contact) {
        return contact.second->uri_ == uri;
    });
    return (i != contacts_.end());
}

void
ContactModel::removeContact(const std::string& uri)
{
    ConfigurationManager::instance().removeContact(account_->id(), QString(uri.c_str()), false);
    contacts_.erase(uri);
}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    unsigned int id = ConfigurationManager::instance().sendTextMessage(account_->id(), uri.c_str(), payloads);

    auto accountId = account_->id().toStdString();
    lrc::message::Info msg;
    msg.uid = std::to_string(id);
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.isOutgoing = true;
    msg.type = lrc::message::Type::TEXT;
    msg.status = lrc::message::Status::SENDING;

    dbm_->addMessage(accountId, msg);
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
                                                                        isPresent,
                                                                        type));

        contacts_[uri] = contact;
    }

    return true;
}

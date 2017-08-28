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

// Std
#include <stdexcept>

// Data
#include "api/message.h"

// Models and database
#include "callbackshandler.h"
#include "database.h"
#include "api/newaccountmodel.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

// Lrc
#include "availableaccountmodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

namespace lrc
{

namespace api
{

class ContactModelPimpl : public QObject
{
public:
    ContactModelPimpl(NewAccountModel& p,
                      const Database& d,
                      const CallbacksHandler& callbacksHandler,
                      const account::Info& info);

    ~ContactModelPimpl();

    /**
     * Fills with contacts from daemon
     * @return if the method succeeds
     */
    bool fillsWithContacts();
    /**
     * Send a text message to a contact
     * @param contactUri
     * @param body
     */
    void sendMessage(const std::string& uri, const std::string& body) const;
    /**
     * Update the presence status of a contact
     * @param contactUri
     * @param status
     */
    void setContactPresent(const std::string& uri, bool status);

    const Database& db;
    const CallbacksHandler& callbacksHandler;
    ContactInfoMap contacts;
    NewAccountModel& parent;
    const account::Info& owner;

public Q_SLOTS:
    // TODO remove this from here when LRC signals are added
    /**
     * @param contactUri
     * @param confirmed]
     */
    void slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed);
    /**
     * @param contactUri
     * @param banned
     */
    void slotContactsRemoved(const QString &accountID, const QString &uri, bool status);
};


ContactModel::ContactModel(NewAccountModel& parent,
                           const Database& database,
                           const CallbacksHandler& callbacksHandler,
                           const account::Info& info)
: pimpl_(std::make_unique<ContactModelPimpl>(parent, database, callbacksHandler, info))
, owner(info)
{
}

ContactModel::~ContactModel()
{

}

const ContactInfoMap&
ContactModel::getAllContacts() const
{
    return pimpl_->contacts;
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
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()), QString(contactUri.c_str()), banned);
}

void
ContactModel::nameLookup(const std::string& uri) const
{

}

void
ContactModel::addressLookup(const std::string& name) const
{

}

const contact::Info&
ContactModel::getContact(const std::string& contactUri)
{
    auto contact = pimpl_->contacts.find(contactUri);
    if (contact == pimpl_->contacts.end()) {
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);
    }
    return *contact->second.get();
}

ContactModelPimpl::ContactModelPimpl(NewAccountModel& parent,
                                     const Database& database,
                                     const CallbacksHandler& callbacksHandler,
                                     const account::Info& info)
: db(database)
, parent(parent)
, callbacksHandler(callbacksHandler)
, owner(info)
{
    fillsWithContacts();

    // connect the signals
    connect(&callbacksHandler, &CallbacksHandler::NewBuddySubscription,
    [&] (const std::string& contactUri) {
        auto iter = contacts.find(contactUri);

        if (iter != contacts.end())
            iter->second->isPresent = true;
    });
}

ContactModelPimpl::~ContactModelPimpl()
{
    // TODO
}

bool
ContactModelPimpl::fillsWithContacts()
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
    contacts.clear();

    // Add contacts
    auto contactsList = account->getContacts();
    for (auto c : contactsList) {
        auto contact = std::make_shared<contact::Info>();
        auto contactUri = c->uri().toStdString();
        contact->uri = contactUri;
        contact->avatar = db.getContactAttribute(contactUri, "photo");
        contact->registeredName = db.getContactAttribute(contactUri, "username");
        contact->alias = db.getContactAttribute(contactUri, "alias");
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

        contacts[contactUri] = contact;
    }

    return true;
}

void

ContactModelPimpl::setContactPresent(const std::string& contactUri, bool status)
{
    if (contacts.find(contactUri) != contacts.end()) {
        contacts[contactUri]->isPresent = status;
    }
}

void
ContactModelPimpl::slotContactsAdded(const QString &accountID, const QString &uri, bool confirmed)
{
    // TODO: For now, we only get contacts for RING accounts.
    // Moreover, we still use ContactMethod class.
    
    auto contactUri = uri.toStdString();
    
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    if (contacts.find(contactUri) == contacts.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        auto contact = std::make_shared<contact::Info>();
        contact->uri = contactUri;
        contact->avatar = db.getContactAttribute(contactUri, "photo");
        contact->registeredName = db.getContactAttribute(contactUri, "username");
        contact->alias = db.getContactAttribute(contactUri, "alias");
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

        contacts[contactUri] = contact;

        // Add to database
        message::Info msg;
        msg.contact = contactUri;
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        db.addMessage(owner.id, msg);
        //~ emit contactsChanged();
    }
}

void
ContactModelPimpl::slotContactsRemoved(const QString &accountID, const QString &uri, bool status)
{
    auto contactUri = uri.toStdString();

    // TODO handle banned contacts
    if (contacts.find(contactUri) != contacts.end()) {
        db.clearHistory(owner.id, contactUri, true);
        contacts.erase(contactUri);
        //~ emit contactsChanged();
    }
}

void
ContactModelPimpl::sendMessage(const std::string& contactUri, const std::string& body) const
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
    db.addMessage(owner.id, msg);
}

} // namespace api
} // namespace lrc

#include "api/moc_contactmodel.cpp"

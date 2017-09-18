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
#include <iostream>

// Data
#include "api/message.h"

// Models and database
#include "callbackshandler.h"
#include "database.h"
#include "api/newaccountmodel.h"

// Daemon
#include <account_const.h>

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

// Lrc
#include "availableaccountmodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"
#include "private/vcardutils.h"
#include "api/newcallmodel.h"

namespace lrc
{

namespace api
{

class ContactModelPimpl : public QObject
{
public:
    ContactModelPimpl(const ContactModel& linked,
                      const NewAccountModel& p,
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

    const ContactModel& linked;
    const Database& db;
    const CallbacksHandler& callbacksHandler;
    ContactInfoMap contacts;
    const NewAccountModel& parent;
    const account::Info& owner;

public Q_SLOTS:
    /**
     * @param contactUri
     * @param confirmed]
     */
    void slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed);
    /**
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned);

    void slotIncomingContactRequest(const std::string& accountId,
                                    const std::string& ringID,
                                    const std::string& payload);

    void slotIncomingCall(const std::string& callId, const std::string& fromId);

    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status,
                             const QString& address, const QString& name);
};


ContactModel::ContactModel(NewAccountModel& parent,
                           const Database& database,
                           const CallbacksHandler& callbacksHandler,
                           const account::Info& info)
: pimpl_(std::make_unique<ContactModelPimpl>(*this, parent, database, callbacksHandler, info))
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

bool
ContactModel::isPendingRequests() const
{
    auto i = std::find_if(pimpl_->contacts.begin(), pimpl_->contacts.end(),
    [](const auto& contact) {
      return contact.second->type == contact::Type::PENDING;
    });

    return (i != pimpl_->contacts.end());
}


void
ContactModel::addContact(const std::string& contactUri)
{
    auto contact = pimpl_->contacts.find(contactUri);
    if (contact != pimpl_->contacts.end()
        && contact->second->type == contact::Type::PENDING) {
        // Transform the contact into a Ring contact
        ConfigurationManager::instance().acceptTrustRequest(
            QString(owner.id.c_str()),
            QString(contactUri.c_str())
        );
        contact->second->type = contact::Type::RING;

        // Add to database
        message::Info msg;
        msg.contact = contactUri;
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        pimpl_->db.addMessage(owner.id, msg);
        emit modelUpdated();
    } else {
        if (owner.contact.type == contact::Type::SIP) {
            pimpl_->db.addSIPContact(contactUri);
            auto contact = std::make_shared<contact::Info>();
            contact->uri = contactUri;
            contact->alias = contactUri;
            contact->isTrusted = true;
            contact->isPresent = false;
            contact->type = contact::Type::SIP;
            pimpl_->contacts.emplace(std::make_pair(contactUri, contact));
            // Add to database
            message::Info msg;
            msg.contact = contactUri;
            msg.body = "";
            msg.timestamp = std::time(nullptr);
            msg.type = message::Type::CONTACT;
            msg.status = message::Status::SUCCEED;
            pimpl_->db.addMessage(owner.id, msg);
            emit modelUpdated();
        } else {
            ConfigurationManager::instance().addContact(QString(owner.id.c_str()),
            QString(contactUri.c_str()));
        }
    }
}

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    // TODO link to ban model
    auto contact = pimpl_->contacts.find(contactUri);
    if (!banned && contact != pimpl_->contacts.end()
        && contact->second->type == contact::Type::PENDING) {
        ConfigurationManager::instance().discardTrustRequest(
            QString(owner.id.c_str()),
            QString(contactUri.c_str())
        );
        pimpl_->contacts.erase(contactUri);
        emit modelUpdated();
    } else {
        if (owner.contact.type == contact::Type::SIP) {
            pimpl_->db.clearHistory(owner.id, contactUri, true);
            pimpl_->contacts.erase(contactUri);
            emit modelUpdated();
        } else {
            ConfigurationManager::instance().removeContact(QString(owner.id.c_str()),
            QString(contactUri.c_str()), banned);
        }
    }
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
    if (contactUri.empty()) return temporaryContact;
    auto contact = pimpl_->contacts.find(contactUri);
    if (contact == pimpl_->contacts.end()) {
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);
    }
    return *contact->second.get();
}

ContactModelPimpl::ContactModelPimpl(const ContactModel& linked,
                                     const NewAccountModel& parent,
                                     const Database& database,
                                     const CallbacksHandler& callbacksHandler,
                                     const account::Info& info)
: linked(linked)
, db(database)
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

    connect(&callbacksHandler, &CallbacksHandler::contactAdded, this, &ContactModelPimpl::slotContactAdded);
    connect(&callbacksHandler, &CallbacksHandler::contactRemoved, this, &ContactModelPimpl::slotContactRemoved);
    connect(&callbacksHandler, &CallbacksHandler::incomingContactRequest, this, &ContactModelPimpl::slotIncomingContactRequest);
    connect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
            this, &ContactModelPimpl::slotIncomingCall);
    connect(&NameDirectory::instance(), &NameDirectory::registeredNameFound,
            this, &ContactModelPimpl::registeredNameFound);

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
    // Clear current contacts
    contacts.clear();

    if (owner.contact.type == contact::Type::SIP) {
        for (auto contactUri: db.getSIPContacts()) {
            auto contact = std::make_shared<contact::Info>();
            contact->uri = contactUri;
            contact->alias = contactUri;
            contact->isTrusted = true;
            contact->isPresent = false;
            contact->type = contact::Type::SIP;
            contacts.emplace(std::make_pair(contactUri, contact));
        }
        return true;
    }

    // Add contacts
    auto contactsList = account->getContacts();
    for (auto c : contactsList) {
        auto contact = std::make_shared<contact::Info>();
        auto contactUri = c->uri().toStdString();
        contact->uri = contactUri;
        contact->avatar = db.getContactAttribute(contactUri, "photo");
        contact->registeredName = c->registeredName().toStdString();// db.getContactAttribute(contactUri, "username");
        contact->alias = c->bestName().toStdString();// db.getContactAttribute(contactUri, "alias");
        contact->isTrusted = c->isConfirmed();
        contact->isPresent = c->isPresent();
        switch (owner.contact.type)
        {
        case contact::Type::RING:
            contact->type = contact::Type::RING;
            break;
        case contact::Type::SIP:
            contact->type = contact::Type::SIP;
            break;
        case contact::Type::INVALID:
        default:
            contact->type = contact::Type::INVALID;
            break;
        }

        contacts.emplace(std::make_pair(contactUri, contact));
    }

    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(account->id())};
    for (const auto& tr_info : pending_tr) {
       auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();
       auto ringID = tr_info[DRing::Account::TrustRequest::FROM];

       auto c = PhoneDirectoryModel::instance().getNumber(ringID, account);
       const auto vCard = VCardUtils::toHashMap(payload);
       const auto alias = vCard["FN"];
       const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

       auto contact = std::make_shared<contact::Info>();
       auto contactUri = c->uri().toStdString();
       contact->uri = contactUri;
       contact->avatar = photo.toStdString();
       contact->registeredName = c->registeredName().toStdString();// db.getContactAttribute(contactUri, "username");
       contact->alias = alias.toStdString();
       contact->isTrusted = c->isConfirmed();
       contact->isPresent = c->isPresent();
       contact->type = contact::Type::PENDING;

       contacts.emplace(std::make_pair(contactUri, contact));
       // TODO add to db
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
ContactModelPimpl::slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed)
{
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    if (contacts.find(contactUri) == contacts.end()) {
        auto contact = std::make_shared<contact::Info>();
        contact->uri = contactUri;
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        contact->avatar = db.getContactAttribute(contactUri, "photo");
        contact->registeredName = cm->registeredName().toStdString();
        contact->alias = cm->bestName().toStdString();
        contact->isTrusted = confirmed;
        contact->isPresent = cm->isPresent();
        contact->type = contact::Type::RING;

        contacts[contactUri] = contact;

        // Add to database
        message::Info msg;
        msg.contact = contactUri;
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        db.addMessage(owner.id, msg);
        emit linked.modelUpdated();
    }
}

void
ContactModelPimpl::slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned)
{
    // TODO handle banned contacts
    if (contacts.find(contactUri) != contacts.end()) {
        db.clearHistory(owner.id, contactUri, true);
        contacts.erase(contactUri);
        emit linked.modelUpdated();
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

void
ContactModelPimpl::slotIncomingContactRequest(const std::string& accountId,
                                              const std::string& ringID,
                                              const std::string& payload)
{
    if (owner.id != accountId) return;
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    if (contacts.find(ringID) == contacts.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(ringID.c_str()), account);
        auto contact = std::make_shared<contact::Info>();
        contact->uri = ringID;
        contact->avatar = db.getContactAttribute(ringID, "photo"); // TODO
        contact->registeredName = cm->registeredName().toStdString();// db.getContactAttribute(contactUri, "username");
        contact->alias = cm->bestName().toStdString();// db.getContactAttribute(contactUri, "alias");
        contact->isTrusted = false;
        contact->isPresent = false;
        contact->type = contact::Type::PENDING;

        contacts[ringID] = contact;
        emit linked.modelUpdated();
    }
}

void
ContactModelPimpl::slotIncomingCall(const std::string& fromId, const std::string& callId)
{
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    auto contact = std::make_shared<contact::Info>();
    if (contacts.find(fromId) == contacts.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(fromId.c_str()), account);
        contact->uri = fromId;
        contact->avatar = db.getContactAttribute(fromId, "photo"); // TODO
        contact->registeredName = cm->registeredName().toStdString();// db.getContactAttribute(contactUri, "username");
        contact->alias = cm->bestName().toStdString();// db.getContactAttribute(contactUri, "alias");
        contact->isTrusted = false;
        contact->isPresent = false;
        contact->type = contact::Type::PENDING;
        contacts[fromId] = contact;
        emit linked.modelUpdated();
    } else {
        contact = contacts[fromId];
    }

    emit linked.incomingCallFromPending(fromId, callId);
}

void
ContactModelPimpl::registeredNameFound(const Account* account,
                                       NameDirectory::LookupStatus status,
                                       const QString& address,
                                       const QString& name)
{
    if (account->id().toStdString() != linked.owner.id) return;
    if (contacts.find(address.toStdString()) == contacts.end()) return;

    auto& contact = contacts[address.toStdString()];
    contact->registeredName = name.toStdString();
    // alias should contains the best name.
    if (contact->alias == address.toStdString()) {
        contact->alias = name.toStdString();
    }
    emit linked.modelUpdated();
}

} // namespace api
} // namespace lrc

#include "api/moc_contactmodel.cpp"

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

// [jn] : apres l'avoir rebase, verifier les try catch

// Std
#include <stdexcept>

// LRC
#include "callbackshandler.h"
#include "database.h"
#include "api/newaccountmodel.h"
#include "api/contact.h"
#include "api/message.h"
#include "api/account.h"

#include "availableaccountmodel.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

namespace lrc
{

using namespace api;

class ContactModelPimpl : public QObject
{
    Q_OBJECT
public:
    ContactModelPimpl(const ContactModel& linked,
                      Database& db,
                      const CallbacksHandler& callbacksHandler);

    ~ContactModelPimpl();
    /**
     * Fills with contacts from daemon
     * @return if the method succeeds
     */
    bool fillsWithContacts();
    /**
     * Send a text message to a contact over the Dht
     * @param contactUri
     * @param body
     */
    void sendDhtMessage(const std::string& uri, const std::string& body) const;
    /**
     * Update the presence status of a contact
     * @param contactUri
     * @param status
     */
    void setContactPresent(const std::string& uri, bool status);
    /**
     * Add a contact::Info to contacts.
     * @param cm ContactMethod.
     * @return contact::Info added to contacts
     * @todo this function rely on legacy code and has to be changed in the futur.
     */
    contact::Info addToContacts(ContactMethod* cm);
    const ContactModel& linked;
    Database& db;
    ContactModel::ContactInfoMap contacts;
    const CallbacksHandler& callbacksHandler;

public Q_SLOTS:
    /**
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed);
    /**
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned);
};

namespace authority
{
namespace daemon
{
    void addContact(const account::Info& owner, const std::string& uri);
    void addContactFromPending(const account::Info& owner, const std::string& uri);
} // namespace daemon

namespace database
{
    void addContact(Database& db, const std::string& contactUri);
} // namespace database
} // namespace authority

void
authority::daemon::addContact(const account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()), QString(contactUri.c_str()));
}

void
authority::daemon::addContactFromPending(const account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().acceptTrustRequest(QString(owner.id.c_str()), QString(contactUri.c_str()) );
}

void
authority::database::addContact(Database& db, const std::string& contactUri)
{
    // add contact to the database
    auto row = db.insertInto("contacts", {{":id", "ring_id"}, {":alias", "alias"}, {":type", "type"}},
                            {{":id", contactUri}, {":alias", contactUri}, {":type", "SIP"}});

    if (row == -1) {
        qDebug() << "contact not added to the database";
        return;
    }
}

using namespace authority;

ContactModel::ContactModel(const account::Info& owner, Database& database, const CallbacksHandler& callbacksHandler)
: QObject()
, owner(owner)
, pimpl_(std::make_unique<ContactModelPimpl>(*this, database, callbacksHandler))
{
}

ContactModel::~ContactModel()
{

}

const ContactModel::ContactInfoMap&
ContactModel::getAllContacts() const
{
    return pimpl_->contacts;
}

void
ContactModel::addContact(const std::string& contactUri)
{
    auto contact = std::make_shared<contact::Info>();
    contact->uri = contactUri;
    contact->alias = contactUri;
    contact->isTrusted = true;
    contact->isPresent = false;

    if(owner.profile.type == contact::Type::SIP) {
        database::addContact(pimpl_->db, contactUri);
        contact->type = contact::Type::SIP;
    } else { // == contact::Type::RING
        auto contactFound = pimpl_->contacts.find(contactUri);
        if(contactFound != pimpl_->contacts.end() && contactFound->second->type == contact::Type::PENDING) {
            daemon::addContactFromPending(owner, contactUri);
            contactFound->second->type = contact::Type::RING;
        } else {
            daemon::addContact(owner, contactUri);
            contact->type = contact::Type::RING;
        }
    }

    // add a message to the conversation
    message::Info msg;
    msg.contact = contactUri;
    msg.body = "Conversation started";
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::CONTACT;
    msg.status = message::Status::SUCCEED;
    //~ pimpl_->db.addMessage(owner.id, msg);

    pimpl_->contacts.emplace(std::make_pair(contactUri, contact));
    emit modelUpdated();
}

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    // add to daemon auth.
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()), QString(contactUri.c_str()), banned);
}

const contact::Info&
ContactModel::getContact(const std::string& contactUri) const
{
    auto contact = pimpl_->contacts.find(contactUri);
    if (contact == pimpl_->contacts.end())
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);

    return *contact->second.get();
}

ContactModelPimpl::ContactModelPimpl(const ContactModel& linked,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
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

}

ContactModelPimpl::~ContactModelPimpl()
{
}

bool
ContactModelPimpl::fillsWithContacts()
{
    // TODO: For now, we only get contacts for RING accounts.
    // In the future, we will directly get contacts from daemon (or SIP)
    // and avoid "account->getContacts();"
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
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
    for (auto c : contactsList)
        addToContacts(c);

    return true;
}

void

ContactModelPimpl::setContactPresent(const std::string& contactUri, bool status)
{
    if (contacts.find(contactUri) != contacts.end())
        contacts[contactUri]->isPresent = status;
}

void // [jn] maybe to rename to slotnewcontactadded
ContactModelPimpl::slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed)
{
    // TODO: For now, we only get contacts for RING accounts.
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
    }
    if (contacts.find(contactUri) == contacts.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        auto contactInfo = addToContacts(cm);
        contactInfo.isTrusted = confirmed;

        // Add to database
        message::Info msg;
        msg.contact = contactUri;
        msg.body = "";
        msg.timestamp = std::time(nullptr);
        msg.type = message::Type::CONTACT;
        msg.status = message::Status::SUCCEED;
        db.insertInto("conversations",
                      {{":contactId", "contact"}, {":accountId", "account"}, {":body", "body"},
                       {":timestamp", "timestamp"}, {":isUnread", "is_unread"}, {":isOutgoing", "is_outgoing"},
                       {":type", "type"}, {":status", "status"}},
                      {{":contactId", msg.contact}, {":accountId", linked.owner.id}, {":body", msg.body},
                       {":timestamp", std::to_string(msg.timestamp)}, {":isUnread", "1"}, {":isOutgoing", "true"},
                       {":type", TypeToString(msg.type)}, {":status", StatusToString(msg.status)}});

        emit linked.modelUpdated();
    }
}

void
ContactModelPimpl::slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned)
{
    // TODO handle banned contacts
    if (contacts.find(contactUri) != contacts.end()) {
        db.deleteFrom("conversations",
                      "contact=:contactId AND account=:accountId",
                      {{":contactId", contactUri},{":accountId", accountId}});
        contacts.erase(contactUri);
        emit linked.modelUpdated();
    }
}

void
ContactModelPimpl::sendDhtMessage(const std::string& contactUri, const std::string& body) const
{
    // Send message
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();
    ConfigurationManager::instance().sendTextMessage(QString(linked.owner.id.c_str()),
                                                     QString(contactUri.c_str()),
                                                     payloads);

    // Store it into the database
    message::Info msg;
    msg.contact = contactUri;
    msg.body = body;
    msg.timestamp = std::time(nullptr);
    msg.type = message::Type::TEXT;
    msg.status = message::Status::SENDING;
    db.insertInto("conversations",
                      {{":contactId", "contact"}, {":accountId", "account"}, {":body", "body"},
                       {":timestamp", "timestamp"}, {":isUnread", "is_unread"}, {":isOutgoing", "is_outgoing"},
                       {":type", "type"}, {":status", "status"}},
                      {{":contactId", msg.contact}, {":accountId", linked.owner.id}, {":body", msg.body},
                       {":timestamp", std::to_string(msg.timestamp)}, {":isUnread", "1"}, {":isOutgoing", "true"},
                       {":type", TypeToString(msg.type)}, {":status", StatusToString(msg.status)}});
}

// TODO: test !!
contact::Info
ContactModelPimpl::addToContacts(ContactMethod* cm)
{
    auto contactInfo = std::make_shared<contact::Info>();
    auto contactUri = cm->uri().toStdString();
    contactInfo->uri = contactUri;
    auto returnFromDb = db.select("photo, uri, alias",
                                  "profiles",
                                  "uri=:uri",
                                  {{":uri", contactUri}});
    // the query should return on row of three columns.
    if (returnFromDb.nbrOfCols == 3 and returnFromDb.payloads.size() == 3) {
        contactInfo->avatar = returnFromDb.payloads[0];
        contactInfo->registeredName = returnFromDb.payloads[1];
        contactInfo->alias = returnFromDb.payloads[2];
    } else {
        // profiles is not in db, must add it
        db.insertInto("profiles",
                      {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"},
                       {":type", "type"}, {":status", "status"}},
                      {{":uri", contactUri}, {":alias", cm->bestName().toStdString()}, {":photo", ""},
                       {":type", "RING"}, {":status", "TRUSTED"}}); // TODO type
       contactInfo->avatar = "";
       contactInfo->registeredName = cm->registeredName().toStdString();
       contactInfo->alias = cm->bestName().toStdString();
    }
    contactInfo->isPresent = cm->isPresent();
    switch (linked.owner.type)
    {
    case account::Type::RING:
        contactInfo->type = contact::Type::RING;
        break;
    case account::Type::SIP:
        contactInfo->type = contact::Type::SIP;
        break;
    case account::Type::INVALID:
    default:
        contactInfo->type = contact::Type::INVALID;
        break;
    }

    contacts[contactUri] = contactInfo;
    qDebug() << "#######" << contactUri.c_str();

    return *contactInfo;
}

} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

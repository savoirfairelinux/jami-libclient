/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author:  Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author:  Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include <algorithm>

// Daemon
#include <account_const.h>

// LRC
#include "api/account.h"
#include "api/contact.h"
#include "api/interaction.h"
#include "api/newcallmodel.h"
#include "callbackshandler.h"

#include "accountmodel.h"
#include "contactmethod.h"
#include "namedirectory.h"
#include "phonedirectorymodel.h"
#include "private/vcardutils.h"

#include "authority/daemon.h"
#include "authority/databasehelper.h"

// Dbus
#include "dbus/configurationmanager.h"

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
     * Fills with contacts based on database's requests
     * @return if the method succeeds
     */
    bool fillsWithSIPContacts();
    /**
     * Fills with contacts based on daemon's requests
     * @return if the method succeeds
     */
    bool fillsWithRINGContacts();
    /**
     * Update the presence status of a contact
     * @param contactUri
     * @param status
     */
    void setContactPresent(const std::string& uri, bool status);
    /**
     * Add a contact::Info to contacts.
     * @note: the cm must corresponds to a profile in the database.
     * @param cm ContactMethod.
     * @param type
     */
    void addToContacts(ContactMethod* cm, const profile::Type& type);

    // Helpers
    const ContactModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    // Containers
    ContactModel::ContactInfoMap contacts;

public Q_SLOTS:
    /**
     * Listen CallbacksHandler when a contact is added
     * @param accountId
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed);
    /**
     * Listen CallbacksHandler when a contact is removed
     * @param accountId
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned);
    /**
     * Listen CallbacksHandler when a registeredName is found
     * @param accountId account linked
     * @param uri of the contact found
     * @param registeredName of the contact found
     */
    void slotRegisteredNameFound(const std::string& accountId, const std::string& uri, const std::string& registeredName);
    /**
     * Listen CallbacksHandler when an incoming request arrives
     * @param accountId account linked
     * @param contactUri
     * @param payload VCard of the contact
     */
    void slotIncomingContactRequest(const std::string& accountId,
                                    const std::string& contactUri,
                                    const std::string& payload);
    /**
     * Listen from call Model when an incoming call arrives.
     * @param fromId
     * @param callId
     */
    void slotIncomingCall(const std::string& fromId, const std::string& callId);
    /**
     * Listen from callbacksHandler for new account interaction and add pending contact if not present
     * @param accountId
     * @param from
     * @param payloads
     */
    void slotNewAccountMessage(std::string& accountId,
                               std::string& from,
                               std::map<std::string,std::string> payloads);
};

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

bool
ContactModel::hasPendingRequests() const
{
    auto i = std::find_if(pimpl_->contacts.begin(), pimpl_->contacts.end(),
        [](const auto& c) {
          return c.second.profileInfo.type == profile::Type::PENDING;
        });

    return (i != pimpl_->contacts.end());
}

void
ContactModel::addContact(contact::Info contactInfo)
{
    if ((owner.profileInfo.type == profile::Type::SIP and contactInfo.profileInfo.type == profile::Type::RING)
        or (owner.profileInfo.type == profile::Type::RING and contactInfo.profileInfo.type == profile::Type::SIP)) {
            qDebug() << "ContactModel::addContact, types invalids.";
            return;
    }

    auto* account = AccountModel::instance().getById(owner.id.c_str());
    QMap<QString, QString> details = ConfigurationManager::instance()
        .getContactDetails(owner.id.c_str(), contactInfo.profileInfo.uri.c_str());
    // check if it's already a contact for the daemon
    if (!details.empty()) {
        contactInfo.profileInfo.type = owner.profileInfo.type;
    }

    switch (contactInfo.profileInfo.type) {
    case profile::Type::TEMPORARY:
        // NOTE: do not set profile::Type::RING, this has to be done when the daemon has emited contactAdded
        if (account) account->sendContactRequest(URI(contactInfo.profileInfo.uri.c_str()));
        break;
    case profile::Type::PENDING:
        daemon::addContactFromPending(owner, contactInfo.profileInfo.uri);
        // NOTE: Don't know why, but the first function don't add a daemon contact sometimes.
        daemon::addContact(owner, contactInfo.profileInfo.uri);
        break;
    case profile::Type::RING:
    case profile::Type::SIP:
        break;
    case profile::Type::INVALID:
    default:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
        return;
    }

    database::getOrInsertProfile(pimpl_->db, contactInfo.profileInfo.uri,
                                 contactInfo.profileInfo.alias,
                                 contactInfo.profileInfo.avatar,
                                 to_string(owner.profileInfo.type));
    pimpl_->contacts[contactInfo.profileInfo.uri].profileInfo = contactInfo.profileInfo;
    emit contactAdded(contactInfo.profileInfo.uri);
}

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    auto contact = pimpl_->contacts.find(contactUri);
    if (!banned && contact != pimpl_->contacts.end()
        && contact->second.profileInfo.type == profile::Type::PENDING) {
        // Discard the pending request and remove profile from db if necessary
        daemon::discardFromPending(owner, contactUri);
        pimpl_->contacts.erase(contactUri);
        database::removeContact(pimpl_->db, owner.profileInfo.uri, contactUri);
        emit contactRemoved(contactUri);
    } else {
        if (owner.profileInfo.type == profile::Type::SIP) {
            // Remove contact from db
            pimpl_->contacts.erase(contactUri);
            database::removeContact(pimpl_->db, owner.profileInfo.uri, contactUri);
            emit contactRemoved(contactUri);
        } else {
            // NOTE: this method is async, slotContactRemoved will be call and
            // then the model will be updated.
            daemon::removeContact(owner, contactUri, banned);
        }
    }
}

const contact::Info
ContactModel::getContact(const std::string& contactUri) const
{
    return pimpl_->contacts.at(contactUri);
}

const std::string
ContactModel::getContactProfileId(const std::string& contactUri) const
{
    return database::getProfileId(pimpl_->db, contactUri);
}

void
ContactModel::searchContact(const std::string& query)
{
    if (owner.profileInfo.type == profile::Type::SIP) {
        // We don't need to search anything for SIP contacts.
        // NOTE: there is no registeredName for SIP contacts
        if (pimpl_->contacts[""].profileInfo.alias == query) {
            // contact already present, remove the temporaryContact
            profile::Info profileInfo = {"", "", "", profile::Type::TEMPORARY};
            pimpl_->contacts[""] = {profileInfo, "", false, false};
        } else {
            profile::Info profileInfo = {query, "", query, profile::Type::TEMPORARY};
            pimpl_->contacts[""] = {profileInfo, "", false, false};
        }
        emit modelUpdated();
        return;
    }
    // Default searching item.
    profile::Info profileInfo = {"", "", "Searching… " + query, profile::Type::TEMPORARY};
    pimpl_->contacts[""] = {profileInfo, query, false, false};
    emit modelUpdated();

    // Query Name Server
    auto uri = URI(QString(query.c_str()));
    if (uri.full().startsWith("ring:")) {
        profile::Info profileInfo = {query, "", query, profile::Type::TEMPORARY};
        pimpl_->contacts[""] = {profileInfo, query, false, false};
        emit modelUpdated();
        return;
    }
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (!account) return;
    account->lookupName(QString(query.c_str()));
}

void
ContactModel::sendDhtMessage(const std::string& contactUri, const std::string& body) const
{
    // Send interaction
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();
    ConfigurationManager::instance().sendTextMessage(QString(owner.id.c_str()),
                                                     QString(contactUri.c_str()),
                                                     payloads);
    // NOTE: ConversationModel should store the interaction into the database
}


ContactModelPimpl::ContactModelPimpl(const ContactModel& linked,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
{
    // Init contacts map
    if (linked.owner.profileInfo.type == profile::Type::SIP)
        fillsWithSIPContacts();
    else
        fillsWithRINGContacts();

    // connect the signals
    connect(&callbacksHandler, &CallbacksHandler::newBuddySubscription,
        [&] (const std::string& contactUri) {
            auto iter = contacts.find(contactUri);

            if (iter != contacts.end())
                iter->second.isPresent = true;
        });
    connect(&callbacksHandler, &CallbacksHandler::contactAdded,
            this, &ContactModelPimpl::slotContactAdded);
    connect(&callbacksHandler, &CallbacksHandler::contactRemoved,
            this, &ContactModelPimpl::slotContactRemoved);
    connect(&callbacksHandler, &CallbacksHandler::incomingContactRequest,
            this, &ContactModelPimpl::slotIncomingContactRequest);
    connect(&callbacksHandler, &CallbacksHandler::registeredNameFound,
            this, &ContactModelPimpl::slotRegisteredNameFound);
    connect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
            this, &ContactModelPimpl::slotIncomingCall);
    connect(&callbacksHandler, &lrc::CallbacksHandler::newAccountMessage,
            this, &ContactModelPimpl::slotNewAccountMessage);

}

ContactModelPimpl::~ContactModelPimpl()
{
}

bool
ContactModelPimpl::fillsWithSIPContacts()
{
    auto accountProfileId = database::getProfileId(db, linked.owner.profileInfo.uri);
    auto conversationsForAccount = database::getConversationsForProfile(db, accountProfileId);
    for (const auto& c : conversationsForAccount) {
        auto otherParticipants = database::getPeerParticipantsForConversation(db, accountProfileId, c);
        for (const auto& participant: otherParticipants) {
            // for each conversations get the other profile id
            auto contactInfo = database::buildContactFromProfileId(db, participant);
            contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
        }
    }
    return true;
}

bool
ContactModelPimpl::fillsWithRINGContacts() {
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::fillsWithContacts(), nullptr";
        return false;
    }

    // Add contacts from daemon
    for (auto* c : account->getContacts()) {
        addToContacts(c, linked.owner.profileInfo.type);
    }

    // Add pending contacts
    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(account->id())};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();

        auto contactUri = tr_info[DRing::Account::TrustRequest::FROM];
        auto* cm = PhoneDirectoryModel::instance().getNumber(contactUri, account);

        const auto vCard = VCardUtils::toHashMap(payload);
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        lrc::api::profile::Info profileInfo = {contactUri.toStdString(),
                                               photo.toStdString(),
                                               alias.toStdString(),
                                               profile::Type::PENDING};

        contact::Info contactInfo = {profileInfo, cm->bestName().toStdString(), false, false};

        contacts.emplace(contactUri.toStdString(), contactInfo);
        database::getOrInsertProfile(db, contactUri.toStdString(),
                                    alias.toStdString(), photo.toStdString(),
                                    profile::to_string(profile::Type::RING));
    }

    return true;
}

void
ContactModelPimpl::setContactPresent(const std::string& contactUri, bool status)
{
    if (contacts.find(contactUri) != contacts.end())
        contacts[contactUri].isPresent = status;
}

void
ContactModelPimpl::slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed)
{
    Q_UNUSED(confirmed)
    if (accountId != linked.owner.id) return;
    auto* account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
        return;
    }
    auto* cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
    addToContacts(cm, linked.owner.profileInfo.type);
    emit linked.contactAdded(contactUri);
}

void
ContactModelPimpl::slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned)
{
    if (accountId != linked.owner.id) return;
    Q_UNUSED(banned)
    if (contacts.find(contactUri) != contacts.end()) {
        database::removeContact(db, linked.owner.profileInfo.uri, contactUri);
        contacts.erase(contactUri);
        emit linked.contactRemoved(contactUri);
    }
}

void
ContactModelPimpl::addToContacts(ContactMethod* cm, const profile::Type& type)
{
    if (!cm) return;
    auto contactUri = cm->uri().toStdString();
    auto contactId = database::getProfileId(db, contactUri);
    if (contactId.empty()) {
        contactId = database::getOrInsertProfile(db, contactUri,
                                                 "",
                                                 "",
                                                 to_string(linked.owner.profileInfo.type));
    }
    auto contactInfo = database::buildContactFromProfileId(db, contactId);
    contactInfo.registeredName = cm->bestName().toStdString();
    contactInfo.isPresent = cm->isPresent();
    contactInfo.profileInfo.type = type; // Because PENDING should not be stored in the database
    if (contacts.find(contactInfo.profileInfo.uri) == contacts.end())
        contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
    else
        contacts[contactInfo.profileInfo.uri] = contactInfo;
}

void
ContactModelPimpl::slotRegisteredNameFound(const std::string& accountId,
                                           const std::string& uri,
                                           const std::string& registeredName)
{
    if (accountId != linked.owner.id) return;

    auto& temporaryContact = contacts[""];
    if (contacts.find(uri) == contacts.end()) {
        // contact not present, update the temporaryContact
        lrc::api::profile::Info profileInfo = {uri, "", registeredName, profile::Type::TEMPORARY};
        temporaryContact = {profileInfo, registeredName, false, false};
    } else {
        // Update contact
        contacts[uri].registeredName = registeredName;
        if (temporaryContact.registeredName == uri || temporaryContact.registeredName == registeredName) {
            // contact already present, remove the temporaryContact
            lrc::api::profile::Info profileInfo = {"", "", "", profile::Type::TEMPORARY};
            temporaryContact = {profileInfo, "", false, false};
        }
    }
    emit linked.modelUpdated();

}

void
ContactModelPimpl::slotIncomingContactRequest(const std::string& accountId,
                                              const std::string& contactUri,
                                              const std::string& payload)
{
    if (linked.owner.id != accountId)
        return;

    auto* account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingContactRequest(), nullptr";
        return;
    }

    if (contacts.find(contactUri) == contacts.end()) {
        auto* cm = PhoneDirectoryModel::instance().getNumber(URI(contactUri.c_str()), account);
        const auto vCard = VCardUtils::toHashMap(payload.c_str());
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        auto profileInfo = profile::Info({contactUri, photo.toStdString(), alias.toStdString(), profile::Type::PENDING});
        auto contactInfo = contact::Info({profileInfo, cm->bestName().toStdString(), cm->isConfirmed(), cm->isPresent()});
        contacts.emplace(contactUri, contactInfo);
        database::getOrInsertProfile(db, contactUri, alias.toStdString(),
                                     photo.toStdString(),
                                     profile::to_string(profile::Type::RING));
        emit linked.contactAdded(contactUri);
    }
}

void
ContactModelPimpl::slotIncomingCall(const std::string& fromId, const std::string& callId)
{
    auto* account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingCall(), nullptr";
        return;
    }

    if (contacts.find(fromId) == contacts.end()) {
        // Contact not found, load profile from database.
        // The conversation model will create an entry and link the incomingCall.
        auto* cm = PhoneDirectoryModel::instance().getNumber(QString(fromId.c_str()), account);
        auto type = (linked.owner.profileInfo.type == profile::Type::RING) ? profile::Type::PENDING : profile::Type::SIP;
        addToContacts(cm, type);
        emit linked.contactAdded(fromId);
        emit linked.incomingCallFromPending(fromId, callId);
    } else {
        // Already a contact, just emit signal
        emit linked.incomingCallFromPending(fromId, callId);
    }
}

void
ContactModelPimpl::slotNewAccountMessage(std::string& accountId,
                                         std::string& from,
                                         std::map<std::string,std::string> payloads)
{
    if (accountId != linked.owner.id) return;
    auto* account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingCall(), nullptr";
        return;
    }

    if (contacts.find(from) == contacts.end()) {
        // Contact not found, load profile from database.
        // The conversation model will create an entry and link the incomingCall.
        auto* cm = PhoneDirectoryModel::instance().getNumber(QString(from.c_str()), account);
        addToContacts(cm, profile::Type::PENDING);
    }
    emit linked.newAccountMessage(accountId, from, payloads);
}

} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

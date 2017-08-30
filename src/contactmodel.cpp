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
     * @param: type
     * @return contact::Info added to contacts
     * @TODO this function rely on legacy code and has to be changed in the futur.
     */
    void addToContacts(ContactMethod* cm, const profile::Type& type);

    // Helpers
    const ContactModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    // Containers
    ContactModel::ContactInfoMap contacts;
    //contact::Info temporaryContact;

    // Searching avatar in base 64
    const std::string searchingAvatar = "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";

public Q_SLOTS:
    /**
     * Listen CallbacksHandler when a contact is added
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed);
    /**
     * Listen CallbacksHandler when a contact is removed
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

    switch (contactInfo.profileInfo.type) {
    case profile::Type::RING:
    case profile::Type::TEMPORARY:
    case profile::Type::PENDING:
        // Already a RING contact, daemon::addToContacts will do nothing
        // but we need the profile in the database.
        if (contactInfo.profileInfo.type == profile::Type::PENDING) {
            daemon::addContactFromPending(owner, contactInfo.profileInfo.uri);
            contactInfo.profileInfo.type = api::profile::Type::RING;
        } else // NOTE: do not set profile::Type::RING, this has to be done when the daemon has emited contactAdded
            daemon::addContact(owner, contactInfo);
        // we need to add the profile into the database.
        database::getOrInsertProfile(pimpl_->db, contactInfo.profileInfo.uri, contactInfo.profileInfo.alias,
        contactInfo.profileInfo.avatar, TypeToString(contactInfo.profileInfo.type));
        break;
    case profile::Type::SIP:
        database::getOrInsertProfile(pimpl_->db, contactInfo.profileInfo.uri, contactInfo.profileInfo.alias,
        contactInfo.profileInfo.avatar, TypeToString(profile::Type::SIP));
        contactInfo.profileInfo.type = api::profile::Type::SIP;
        break;
    case profile::Type::INVALID:
    default:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
        break;
    }

    if (pimpl_->contacts.find(contactInfo.profileInfo.uri) == pimpl_->contacts.end())
        pimpl_->contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
    else
        pimpl_->contacts[contactInfo.profileInfo.uri] = contactInfo;
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
    auto contactInfo = pimpl_->contacts.find(contactUri);
    if (contactInfo == pimpl_->contacts.end())
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);

    return contactInfo->second;
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
    profile::Info profileInfo = {"", pimpl_->searchingAvatar, "Searching… " + query, profile::Type::TEMPORARY};
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
    // get conversations with this profile
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
    auto contactsList = account->getContacts();
    for (auto c : contactsList) {
        addToContacts(c, linked.owner.profileInfo.type);
    }

    // Add pending contacts
    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(account->id())};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();

        auto contactUri = tr_info[DRing::Account::TrustRequest::FROM];
        auto cm = PhoneDirectoryModel::instance().getNumber(contactUri, account);

        const auto vCard = VCardUtils::toHashMap(payload);
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        lrc::api::profile::Info profileInfo = {contactUri.toStdString(),
                                               photo.toStdString(),
                                               alias.toStdString(),
                                               profile::Type::PENDING};

        contact::Info contactInfo = {profileInfo, cm->registeredName().toStdString(), false, false};

        contacts.emplace(contactUri.toStdString(), contactInfo);
        database::getOrInsertProfile(db, contactUri.toStdString(), alias.toStdString(), photo.toStdString());
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
    if (accountId != linked.owner.id) return;
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotContactsAdded(), nullptr";
        return;
    }
    if (contacts.find(contactUri) == contacts.end()) {
        // Doesn't exists, add contact
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        addToContacts(cm, linked.owner.profileInfo.type);
        // Other models should be warned
        emit linked.contactAdded(contactUri);
    }
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
    auto contactUri = cm->uri().toStdString();
    auto contactId = database::getProfileId(db, contactUri);
    if (contactId.empty()) {
        contactId = database::getOrInsertProfile(db, contactUri,
                                                 cm->bestName().toStdString(),
                                                 "",
                                                 TypeToString(type));
    }
    auto contactInfo = database::buildContactFromProfileId(db, contactId);
    contactInfo.isPresent = cm->isPresent();
    contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
}

void
ContactModelPimpl::slotRegisteredNameFound(const std::string& accountId,
                                           const std::string& uri,
                                           const std::string& registeredName)
{
    if (accountId != linked.owner.id) return;
    if (contacts.find(uri) == contacts.end()) {
        // contact not present in contacts, update the temporaryContact
        lrc::api::profile::Info profileInfo = {uri, "", registeredName, profile::Type::TEMPORARY};
        contacts[""] = {profileInfo, registeredName, false, false};
    } else {
        // Update contact
        contacts[uri].registeredName = registeredName;
        contacts[uri].profileInfo.alias = registeredName;
        if (contacts[""].registeredName == uri
        || contacts[""].registeredName == registeredName) {
            // contact already present, remove the temporaryContact
            lrc::api::profile::Info profileInfo = {"", searchingAvatar, "", profile::Type::TEMPORARY};
            contacts[""] = {profileInfo, "", false, false};
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

    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingContactRequest(), nullptr";
        return;
    }

    if (contacts.find(contactUri) == contacts.end()) {
        auto cm = PhoneDirectoryModel::instance().getNumber(URI(contactUri.c_str()), account);
        const auto vCard = VCardUtils::toHashMap(payload.c_str());
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        auto profileInfo = profile::Info({contactUri, photo.toStdString(), alias.toStdString(), profile::Type::PENDING});
        auto contactInfo = contact::Info({profileInfo, cm->registeredName().toStdString(), cm->isConfirmed(), cm->isPresent()});
        contacts.emplace(contactUri, contactInfo);
        database::getOrInsertProfile(db, contactUri, alias.toStdString(), photo.toStdString());
        emit linked.contactAdded(contactUri);
    }
}

void
ContactModelPimpl::slotIncomingCall(const std::string& fromId, const std::string& callId)
{
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingCall(), nullptr";
        return;
    }

    if (contacts.find(fromId) == contacts.end()) {
        // Contact not found, load profile from database.
        // The conversation model will create an entry and link the incomingCall.
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(fromId.c_str()), account);
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
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::slotIncomingCall(), nullptr";
        return;
    }

    if (contacts.find(from) == contacts.end()) {
        // Contact not found, load profile from database.
        // The conversation model will create an entry and link the incomingCall.
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(from.c_str()), account);
        addToContacts(cm, profile::Type::PENDING);
    }
    emit linked.newAccountMessage(accountId, from, payloads);
}

} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

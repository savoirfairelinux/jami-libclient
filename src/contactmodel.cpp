/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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
    auto& profile = contactInfo.profileInfo;

    if ((owner.profileInfo.type != profile.type) and
        (profile.type == profile::Type::RING or profile.type == profile::Type::SIP)) {
            qDebug() << "ContactModel::addContact, types invalids.";
            return;
    }

    MapStringString details = ConfigurationManager::instance().getContactDetails(
        owner.id.c_str(), contactInfo.profileInfo.uri.c_str());

    // copy type (if not RING or SIP)
    if (!details.empty())
        profile.type = owner.profileInfo.type;

    switch (profile.type) {
    case profile::Type::TEMPORARY:
        // NOTE: do not set profile::Type::RING, this has to be done when the daemon has emited contactAdded
        if (auto* account = AccountModel::instance().getById(owner.id.c_str()))
            account->sendContactRequest(URI(profile.uri.c_str()));
        break;
    case profile::Type::PENDING:
        daemon::addContactFromPending(owner, profile.uri);
        daemon::addContact(owner, profile.uri); // BUGS?: daemon::addContactFromPending not always add the contact
        break;
    case profile::Type::RING:
    case profile::Type::SIP:
        break;
    case profile::Type::INVALID:
    default:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
        return;
    }

    database::getOrInsertProfile(pimpl_->db,
                                 profile.uri, profile.alias, profile.avatar,
                                 to_string(owner.profileInfo.type));

    auto iter = pimpl_->contacts.find(contactInfo.profileInfo.uri);
    if (iter == pimpl_->contacts.end())
        pimpl_->contacts.emplace_hint(iter, contactInfo.profileInfo.uri, contactInfo);
    else
        iter->second.profileInfo = contactInfo.profileInfo;

    emit contactAdded(profile.uri);
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
    } else if (owner.profileInfo.type == profile::Type::SIP) {
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
    auto& temporaryContact = pimpl_->contacts[""];
    temporaryContact = {}; // reset in any case

    if (owner.profileInfo.type == profile::Type::SIP) {
        // We don't need to search anything for SIP contacts.
        // NOTE: there is no registeredName for SIP contacts

        // Reset temporary if contact exists, else save the query inside it
        auto iter = pimpl_->contacts.find(query);
        if (iter == pimpl_->contacts.end()) {
            profile::Info profileInfo;
            profileInfo.uri = query;
            profileInfo.alias = query;
            profileInfo.type = profile::Type::TEMPORARY;
            temporaryContact.profileInfo = profileInfo;
        }

        emit modelUpdated();
        return;
    }

    // query is a valid RingID?
    auto uri = URI(QString(query.c_str()));
    if (uri.full().startsWith("ring:")) {
        profile::Info profileInfo;
        profileInfo.uri = query;
        profileInfo.alias = query;
        profileInfo.type = profile::Type::TEMPORARY;
        temporaryContact.profileInfo = profileInfo;
        emit modelUpdated();
        return;
    }

    // Default searching
    profile::Info profileInfo;
    profileInfo.alias = "Searching… " + query;
    profileInfo.type = profile::Type::TEMPORARY;
    temporaryContact.profileInfo = profileInfo;
    temporaryContact.registeredName = query;
    emit modelUpdated();

    // Query Name Server
    if (auto* account = AccountModel::instance().getById(owner.id.c_str()))
        account->lookupName(QString(query.c_str()));
}

uint64_t
ContactModel::sendDhtMessage(const std::string& contactUri, const std::string& body) const
{
    // Send interaction
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();
    auto msgId = ConfigurationManager::instance().sendTextMessage(QString(owner.id.c_str()),
                                                     QString(contactUri.c_str()),
                                                     payloads);
    // NOTE: ConversationModel should store the interaction into the database
    return msgId;
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
        [this] (const std::string& contactUri, bool status) {
            auto iter = contacts.find(contactUri);
            if (iter != contacts.end()) {
                iter->second.isPresent = status;
                emit this->linked.modelUpdated();
            }
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

        lrc::api::profile::Info profileInfo;
        profileInfo.uri = contactUri.toStdString();
        profileInfo.avatar = photo.toStdString();
        profileInfo.alias = alias.toStdString();
        profileInfo.type = profile::Type::PENDING;

        contact::Info contactInfo;
        contactInfo.profileInfo = profileInfo;
        contactInfo.registeredName = cm->bestName().toStdString();

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
    auto it = contacts.find(contactUri);
    if (it != contacts.end()) {
        it->second.isPresent = status;
        emit linked.modelUpdated();
    }
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
    Q_UNUSED(banned)
    if (accountId != linked.owner.id) return;
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
        contactId = database::getOrInsertProfile(db, contactUri, "", "",
                                                 to_string(linked.owner.profileInfo.type));
    }
    auto contactInfo = database::buildContactFromProfileId(db, contactId);
    contactInfo.registeredName = cm->bestName().toStdString();
    contactInfo.isPresent = cm->isPresent();
    contactInfo.profileInfo.type = type; // Because PENDING should not be stored in the database
    auto iter = contacts.find(contactInfo.profileInfo.uri);
    if (iter != contacts.end())
        iter->second = contactInfo;
    else
        contacts.emplace_hint(iter, contactInfo.profileInfo.uri, contactInfo);
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

        auto profileInfo = profile::Info {contactUri, photo.toStdString(), alias.toStdString(), profile::Type::PENDING};
        auto contactInfo = contact::Info {profileInfo, cm->bestName().toStdString(), cm->isConfirmed(), cm->isPresent()};
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

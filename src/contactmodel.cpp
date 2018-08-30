/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
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

// Std
#include <algorithm>
#include <mutex>

// Daemon
#include <account_const.h>

// LRC
#include "api/contactmodel.h"
#include "api/account.h"
#include "api/contact.h"
#include "api/interaction.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"
#include "api/conversationmodel.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"

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
                      const CallbacksHandler& callbacksHandler,
                      const BehaviorController& behaviorController);

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
     * Add a contact::Info to contacts.
     * @note: the contactId must corresponds to a profile in the database.
     * @param contactId
     * @param type
     * @param banned whether contact is banned or not
     */
    void addToContacts(ContactMethod* cm, const profile::Type& type, bool banned = false);
    /**
     * Helpers for searchContact. Search for a given RING or SIP contact.
     */
    void searchRingContact(const URI& query);
    void searchSipContact(const URI& query);
    /**
     * Update temporary item to display a given message about a given uri.
     */
    void updateTemporaryMessage(const std::string& mes, const std::string& uri);

    // Helpers
    const BehaviorController& behaviorController;
    const ContactModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    // Containers
    ContactModel::ContactInfoMap contacts;
    std::list<std::string> bannedContacts;
    std::mutex contactsMtx_;
    std::mutex bannedContactsMtx_;

public Q_SLOTS:
    /**
     * Listen CallbacksHandler when a presence update occurs
     * @param contactUri
     * @param status
     */
    void slotNewBuddySubscription(const std::string& uri, bool status);

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
     * @param status (0 = SUCCESS, 1 = Not found, 2 = Network error)
     * @param uri of the contact found
     * @param registeredName of the contact found
     */
    void slotRegisteredNameFound(const std::string& accountId, int status, const std::string& uri, const std::string& registeredName);

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
     * Listen from callModel when an incoming call arrives.
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

    /**
     * Listen from callbacksHandler to know when a file transfer interaction is incoming
     * @param dringId Daemon's ID for incoming transfer
     * @param transferInfo DataTransferInfo structure from daemon
     */
    void slotNewAccountTransfer(long long dringId, datatransfer::Info info);
};

using namespace authority;

ContactModel::ContactModel(const account::Info& owner, Database& database, const CallbacksHandler& callbacksHandler, const BehaviorController& behaviorController)
: QObject()
, owner(owner)
, pimpl_(std::make_unique<ContactModelPimpl>(*this, database, callbacksHandler, behaviorController))
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
    return pendingRequestCount() > 0;
}

int
ContactModel::pendingRequestCount() const
{
    std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
    int pendingRequestCount = 0;
    std::for_each(pimpl_->contacts.begin(), pimpl_->contacts.end(),
        [&pendingRequestCount] (const auto& c) {
            if (!c.second.isBanned)
                pendingRequestCount += static_cast<int>(c.second.profileInfo.type == profile::Type::PENDING);
        });
    return pendingRequestCount;
}

void
ContactModel::addContact(contact::Info contactInfo)
{
    auto& profile = contactInfo.profileInfo;

    // If passed contact is a banned contact, call the daemon to unban it
    auto it = std::find(pimpl_->bannedContacts.begin(), pimpl_->bannedContacts.end(), profile.uri);
    if (it != pimpl_->bannedContacts.end()) {
        qDebug("Unban-ing contact %s", profile.uri.c_str());
        ConfigurationManager::instance().addContact(owner.id.c_str(), profile.uri.c_str());
        // bannedContacts will be updated in slotContactAdded
        return;
    }

    if ((owner.profileInfo.type != profile.type) and
        (profile.type == profile::Type::RING or profile.type == profile::Type::SIP)) {
            qDebug() << "ContactModel::addContact, types invalids.";
            return;
    }

    MapStringString details = ConfigurationManager::instance().getContactDetails(
        owner.id.c_str(), contactInfo.profileInfo.uri.c_str());

    // if contactInfo is already a contact for the daemon, type should be equals to RING
    // if the user add a temporary item for a SIP account, should be directly transformed
    if (!details.empty()
        || (profile.type == profile::Type::TEMPORARY
        && owner.profileInfo.type == profile::Type::SIP))
            profile.type = owner.profileInfo.type;

    QByteArray vCard = owner.accountModel->accountVCard(owner.id).c_str();
    switch (profile.type) {
    case profile::Type::TEMPORARY:
        ConfigurationManager::instance().addContact(owner.id.c_str(), profile.uri.c_str());
        ConfigurationManager::instance().sendTrustRequest(owner.id.c_str(), profile.uri.c_str(), vCard);
        break;
    case profile::Type::PENDING:
        if (daemon::addContactFromPending(owner, profile.uri)) {
            emit pendingContactAccepted(profile.uri);
        } else {
            return;
        }
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

    {
        std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
        auto iter = pimpl_->contacts.find(contactInfo.profileInfo.uri);
        if (iter == pimpl_->contacts.end())
            pimpl_->contacts.emplace_hint(iter, contactInfo.profileInfo.uri, contactInfo);
        else {
            // On non-DBus platform, contactInfo.profileInfo.type may be wrong as the contact
            // may be trusted already. We must use Profile::Type from pimpl_->contacts
            // and not from contactInfo so we cannot revert a contact back to PENDING.
            contactInfo.profileInfo.type = iter->second.profileInfo.type;
            iter->second.profileInfo = contactInfo.profileInfo;
        }
    }
    if (profile.type == profile::Type::TEMPORARY)
        return;
    emit contactAdded(profile.uri);
}

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    bool emitContactRemoved = false;
    {
        std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
        auto contact = pimpl_->contacts.find(contactUri);
        if (!banned && contact != pimpl_->contacts.end()
            && contact->second.profileInfo.type == profile::Type::PENDING) {
            // Discard the pending request and remove profile from db if necessary
            if(!daemon::discardFromPending(owner, contactUri)) {
                qDebug() << "Discard request for account " << owner.id.c_str() << " failed (" << contactUri.c_str() << ")";
                return;
            }
            pimpl_->contacts.erase(contactUri);
            database::removeContact(pimpl_->db, owner.profileInfo.uri, contactUri);
            emitContactRemoved = true;
        }
        else if (owner.profileInfo.type == profile::Type::SIP) {
            // Remove contact from db
            pimpl_->contacts.erase(contactUri);
            database::removeContact(pimpl_->db, owner.profileInfo.uri, contactUri);
            emitContactRemoved = true;
        }
    }
    if (emitContactRemoved) {
        emit contactRemoved(contactUri);
    } else {
        // NOTE: this method is asynchronous, the model will be updated
        // in slotContactRemoved
        daemon::removeContact(owner, contactUri, banned);
    }
}

const contact::Info
ContactModel::getContact(const std::string& contactUri) const
{
    std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
    return pimpl_->contacts.at(contactUri);
}

const std::list<std::string>&
ContactModel::getBannedContacts() const
{
    return pimpl_->bannedContacts;
}

const std::string
ContactModel::getContactProfileId(const std::string& contactUri) const
{
    return database::getProfileId(pimpl_->db, contactUri);
}

void
ContactModel::searchContact(const std::string& query)
{
    // always reset temporary contact
    pimpl_->contacts[""] = {};

    auto uri = URI(QString(query.c_str()));

    auto uriScheme = uri.schemeType();
    if (uri.schemeType() == URI::SchemeType::NONE) {
        // uri has no scheme, default to current account scheme
        if (owner.profileInfo.type == profile::Type::SIP) {
            uriScheme = URI::SchemeType::SIP;
        } else if (owner.profileInfo.type == profile::Type::RING) {
            uriScheme = URI::SchemeType::RING;
        }
    }

    if (uriScheme == URI::SchemeType::SIP && owner.profileInfo.type == profile::Type::SIP) {
        pimpl_->searchSipContact(uri);
    } else if (uriScheme == URI::SchemeType::RING && owner.profileInfo.type == profile::Type::RING) {
        pimpl_->searchRingContact(uri);
    } else {
        pimpl_->updateTemporaryMessage(tr("Bad URI scheme").toStdString(), uri.full().toStdString());
    }
}

void
ContactModelPimpl::updateTemporaryMessage(const std::string& mes, const std::string& uri)
{
    std::lock_guard<std::mutex> lk(contactsMtx_);
    auto& temporaryContact = contacts[""];
    temporaryContact.profileInfo.alias = mes;
    temporaryContact.profileInfo.type = profile::Type::TEMPORARY;
    temporaryContact.registeredName = uri;
}

void
ContactModelPimpl::searchRingContact(const URI& query)
{
    if (query.isEmpty()) {
        return;
    }

    std::string uriID = query.format(URI::Section::USER_INFO | URI::Section::HOSTNAME | URI::Section::PORT).toStdString();
    if (query.protocolHint() == URI::ProtocolHint::RING) {
        // no lookup, this is a ring infoHash
        auto& temporaryContact = contacts[""];
        temporaryContact.profileInfo.uri = uriID;
        temporaryContact.profileInfo.alias = uriID;
        temporaryContact.profileInfo.type = profile::Type::TEMPORARY;
    } else {
        updateTemporaryMessage(tr("Searching…").toStdString(), uriID);

        // Default searching
        if (auto* account = AccountModel::instance().getById(linked.owner.id.c_str())) {
            account->lookupName(QString(uriID.c_str()));
        }
    }
}

void
ContactModelPimpl::searchSipContact(const URI& query)
{
    if (query.isEmpty()) {
        return;
    }

    std::string uriID = query.format(URI::Section::USER_INFO | URI::Section::HOSTNAME | URI::Section::PORT).toStdString();
    auto& temporaryContact = contacts[""];

    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(uriID) == contacts.end()) {
            temporaryContact.profileInfo.uri = uriID;
            temporaryContact.profileInfo.alias = uriID;
            temporaryContact.profileInfo.type = profile::Type::TEMPORARY;
        }
    }
    emit linked.modelUpdated(uriID);
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
                                     const CallbacksHandler& callbacksHandler,
                                     const BehaviorController& behaviorController)
: linked(linked)
, db(db)
, behaviorController(behaviorController)
, callbacksHandler(callbacksHandler)
{
    // Init contacts map
    if (linked.owner.profileInfo.type == profile::Type::SIP)
        fillsWithSIPContacts();
    else
        fillsWithRINGContacts();

    // connect the signals
    connect(&callbacksHandler, &CallbacksHandler::newBuddySubscription,
            this, &ContactModelPimpl::slotNewBuddySubscription);
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
    connect(&callbacksHandler, &CallbacksHandler::transferStatusCreated,
            this, &ContactModelPimpl::slotNewAccountTransfer);
}

ContactModelPimpl::~ContactModelPimpl()
{
    disconnect(&callbacksHandler, &CallbacksHandler::newBuddySubscription,
               	this, &ContactModelPimpl::slotNewBuddySubscription);
    disconnect(&callbacksHandler, &CallbacksHandler::contactAdded,
               this, &ContactModelPimpl::slotContactAdded);
    disconnect(&callbacksHandler, &CallbacksHandler::contactRemoved,
               this, &ContactModelPimpl::slotContactRemoved);
    disconnect(&callbacksHandler, &CallbacksHandler::incomingContactRequest,
               this, &ContactModelPimpl::slotIncomingContactRequest);
    disconnect(&callbacksHandler, &CallbacksHandler::registeredNameFound,
               this, &ContactModelPimpl::slotRegisteredNameFound);
    disconnect(&*linked.owner.callModel, &NewCallModel::newIncomingCall,
               this, &ContactModelPimpl::slotIncomingCall);
    disconnect(&callbacksHandler, &lrc::CallbacksHandler::newAccountMessage,
               this, &ContactModelPimpl::slotNewAccountMessage);
    disconnect(&callbacksHandler, &CallbacksHandler::transferStatusCreated,
               this, &ContactModelPimpl::slotNewAccountTransfer);
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
            {
                std::lock_guard<std::mutex> lk(contactsMtx_);
                contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
            }
        }
    }

    return true;
}

bool
ContactModelPimpl::fillsWithRINGContacts() {
    // Add contacts from daemon
    const VectorMapStringString& contacts_vector = ConfigurationManager::instance().getContacts(linked.owner.id.c_str());
    for (auto contact_info : contacts_vector) {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        bool banned = contact_info["banned"] == "true" ? true : false;
        addToContacts(contact_info["id"].toStdString(), linked.owner.profileInfo.type, banned);
    }

    // Add pending contacts
    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(linked.owner.id.c_str())};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();

        auto contactUri = tr_info[DRing::Account::TrustRequest::FROM];

        const auto vCard = VCardUtils::toHashMap(payload);
        const auto alias = vCard["FN"];
        const auto photo = (vCard.find("PHOTO;ENCODING=BASE64;TYPE=PNG") == vCard.end()) ?
        vCard["PHOTO;ENCODING=BASE64;TYPE=JPEG"] : vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        lrc::api::profile::Info profileInfo;
        profileInfo.uri = contactUri.toStdString();
        profileInfo.avatar = photo.toStdString();
        profileInfo.alias = alias.toStdString();
        profileInfo.type = profile::Type::PENDING;

        contact::Info contactInfo;
        contactInfo.profileInfo = profileInfo;
        contactInfo.registeredName = "";
        contactInfo.isBanned = false;

        {
            std::lock_guard<std::mutex> lk(contactsMtx_);
            contacts.emplace(contactUri.toStdString(), contactInfo);
        }

        database::getOrInsertProfile(db, contactUri.toStdString(),
                                    alias.toStdString(), photo.toStdString(),
                                    profile::to_string(profile::Type::RING));
    }

    return true;
}

void
ContactModelPimpl::slotNewBuddySubscription(const std::string& contactUri, bool status)
{
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        auto it = contacts.find(contactUri);
        if (it != contacts.end()) {
            it->second.isPresent = status;
        } else
            return;
    }
    emit linked.modelUpdated(contactUri, false);
}

void
ContactModelPimpl::slotContactAdded(const std::string& accountId, const std::string& contactUri, bool confirmed)
{
    Q_UNUSED(confirmed)
    if (accountId != linked.owner.id) return;
    auto contact = contacts.find(contactUri);

    if (contact->second.profileInfo.type == profile::Type::PENDING) {
        emit behaviorController.trustRequestTreated(linked.owner.id, contactUri);
    }

    bool isBanned = false;

    {
        // Always get contactsMtx_ lock before bannedContactsMtx_.
        std::lock_guard<std::mutex> lk(contactsMtx_);

        {
            // Check whether contact is banned or not
            std::lock_guard<std::mutex> lk(bannedContactsMtx_);
            auto it = std::find(bannedContacts.begin(), bannedContacts.end(), contact->second.profileInfo.uri);

            isBanned = (it != bannedContacts.end());

            // If contact is banned, do not re-add it, simply update its flag and the banned contacts list
            if (isBanned) {
                bannedContacts.erase(it);
            }

            addToContacts(contactUri, linked.owner.profileInfo.type, false);
        }
    }

    if (isBanned) {
        // Update the smartlist
        linked.owner.conversationModel->refreshFilter();
        emit linked.bannedStatusChanged(contactUri, false);
    } else {
        emit linked.contactAdded(contactUri);
    }
}

void
ContactModelPimpl::slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned)
{
    if (accountId != linked.owner.id)
        return;

    {
        // Always get contactsMtx_ lock before bannedContactsMtx_.
        std::lock_guard<std::mutex> lk(contactsMtx_);

        auto contact = contacts.find(contactUri);
        if (contact == contacts.end()) return;

        if (contact->second.profileInfo.type == profile::Type::PENDING) {
            emit behaviorController.trustRequestTreated(linked.owner.id, contactUri);
        }

        if (banned) {
            contact->second.isBanned = true;

            // Update bannedContacts index
            bannedContacts.emplace_back(contact->second.profileInfo.uri);
        } else {
            if (contact->second.isBanned) {
                // Contact was banned, update bannedContacts
                std::lock_guard<std::mutex> lk(bannedContactsMtx_);
                auto it = std::find(bannedContacts.begin(), bannedContacts.end(), contact->second.profileInfo.uri);
                if (it == bannedContacts.end()) {
                    // should not happen
                    qDebug("ContactModel::slotContactsRemoved(): Contact is banned but not present in bannedContacts. This is most likely the result of an earlier bug.");
                } else {
                    bannedContacts.erase(it);
                }
            }
            database::removeContact(db, linked.owner.profileInfo.uri, contactUri);
            contacts.erase(contactUri);
        }
    }

    if (banned) {
        // Update the smartlist
        linked.owner.conversationModel->refreshFilter();
        emit linked.bannedStatusChanged(contactUri, true);
    } else {
        emit linked.contactRemoved(contactUri);
    }
}

void
ContactModelPimpl::addToContacts(const std::string& contactId, const profile::Type& type, bool banned)
{
    auto contactInfo = database::buildContactFromProfileId(db, contactId);
    contactInfo.isBanned = banned;
    contactInfo.profileInfo.type = type; // PENDING should not be stored in the database

    // lookup address in case of RING contact
    if (type == profile::Type::RING) {
        ConfigurationManager::instance().lookupAddress(QString::fromStdString(linked.owner.id),
                                                       "", QString::fromStdString(contactId));
    }

    auto contactInfo = database::buildContactFromProfileId(db, contactId);
    contactInfo.registeredName = cm->registeredName().toStdString();
    contactInfo.isBanned = banned;
    contactInfo.isPresent = cm->isPresent();
    contactInfo.profileInfo.type = type; // Because PENDING should not be stored in the database
    auto iter = contacts.find(contactInfo.profileInfo.uri);
    if (iter != contacts.end()) {
        auto info = iter->second;
        contactInfo.registeredName = info.registeredName;
        iter->second = contactInfo;
    } else
        contacts.emplace_hint(it, contactInfo.profileInfo.uri, contactInfo);

    if (banned) {
        bannedContacts.emplace_back(contactId);
    }
}

void
ContactModelPimpl::slotRegisteredNameFound(const std::string& accountId,
                                           int status,
                                           const std::string& uri,
                                           const std::string& registeredName)
{
    if (accountId != linked.owner.id) return;

    auto& temporaryContact = contacts[""];
    if (status == 0 /* SUCCESS */) {
        std::lock_guard<std::mutex> lk(contactsMtx_);

        if (contacts.find(uri) != contacts.end()) {
            // update contact and remove temporary item
            contacts[uri].registeredName = registeredName;
            temporaryContact = {};
        } else {
            if (temporaryContact.registeredName != uri && temporaryContact.registeredName != registeredName) {
                // we are notified that a previous lookup ended
                return;
            }

            // update temporary item
            lrc::api::profile::Info profileInfo = {uri, "", "", profile::Type::TEMPORARY};
            temporaryContact = {profileInfo, registeredName, false, false};
        }
    } else {
        if (temporaryContact.registeredName != uri && temporaryContact.registeredName != registeredName) {
            // we are notified that a previous lookup ended
            return;
        }

        switch (status) {
        case 1 /* INVALID */:
            updateTemporaryMessage(tr("Invalid ringID").toStdString(), registeredName);
            break;
        case 2 /* NOT FOUND */:
            updateTemporaryMessage(tr("Not found").toStdString(), registeredName);
            break;
        case 3 /* ERROR */:
            updateTemporaryMessage(tr("Couldn't lookup…").toStdString(), registeredName);
            break;
        }
    }

    emit linked.modelUpdated(uri);
}

void
ContactModelPimpl::slotIncomingContactRequest(const std::string& accountId,
                                              const std::string& contactUri,
                                              const std::string& payload)
{
    if (linked.owner.id != accountId)
        return;

    auto emitTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(contactUri) == contacts.end()) {
            const auto vCard = VCardUtils::toHashMap(payload.c_str());
            const auto alias = vCard["FN"];
            const auto photo = (vCard.find("PHOTO;ENCODING=BASE64;TYPE=PNG") == vCard.end()) ?
            vCard["PHOTO;ENCODING=BASE64;TYPE=JPEG"] : vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

            auto profileInfo = profile::Info {contactUri, photo.toStdString(), alias.toStdString(), profile::Type::PENDING};
            auto contactInfo = contact::Info {profileInfo, "", false, false, false};
            contacts.emplace(contactUri, contactInfo);
            emitTrust = true;
            database::getOrInsertProfile(db, contactUri, alias.toStdString(),
                                         photo.toStdString(),
                                         profile::to_string(profile::Type::RING));
        }
    }

    if (emitTrust) {
        emit linked.contactAdded(contactUri);
        emit behaviorController.newTrustRequest(linked.owner.id, contactUri);
    }
}

void
ContactModelPimpl::slotIncomingCall(const std::string& fromId, const std::string& callId)
{
    bool emitContactAdded = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(fromId) == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.
            auto type = (linked.owner.profileInfo.type == profile::Type::RING) ? profile::Type::PENDING : profile::Type::SIP;
            addToContacts(fromId, type, false);
            emitContactAdded = true;
        }
    }
    if (emitContactAdded) {
        emit linked.contactAdded(fromId);
        if (linked.owner.profileInfo.type == profile::Type::RING) {
            emit behaviorController.newTrustRequest(linked.owner.id, fromId);
        }
    }

    emit linked.incomingCallFromPending(fromId, callId);
}

void
ContactModelPimpl::slotNewAccountMessage(std::string& accountId,
                                         std::string& from,
                                         std::map<std::string,std::string> payloads)
{
    if (accountId != linked.owner.id) return;

    auto emitNewTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(from) == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.
            addToContacts(from, profile::Type::PENDING, false);
            emitNewTrust = true;
        }
    }
    if (emitNewTrust) {
        emit behaviorController.newTrustRequest(linked.owner.id, from);
    }
    emit linked.newAccountMessage(accountId, from, payloads);
}

void
ContactModelPimpl::slotNewAccountTransfer(long long dringId, datatransfer::Info info)
{
    if (info.accountId != linked.owner.id) return;

    bool emitNewTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(info.peerUri) == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.
            addToContacts(info.peerUri, profile::Type::PENDING, false);
            emitNewTrust = true;
        }
    }
    if (emitNewTrust) {
        emit behaviorController.newTrustRequest(linked.owner.id, info.peerUri);
    }

    emit linked.newAccountTransfer(dringId, info);
}

} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

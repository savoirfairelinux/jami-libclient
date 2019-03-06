/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
 *   Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>       *
 *   Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>         *
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
#include <mutex>

// Daemon
#include <account_const.h>

// LRC
#include "api/account.h"
#include "api/contact.h"
#include "api/interaction.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"
#include "api/conversationmodel.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"
#include "uri.h"
#include "vcard.h"

#include "authority/daemon.h"
#include "authority/storagehelper.h"

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
                      const CallbacksHandler& callbacksHandler,
                      const BehaviorController& behaviorController);

    ~ContactModelPimpl();

    /**
     * Fills the contacts based on database's conversations
     * @return if the method succeeds
     */
    bool fillWithSIPContacts();

    /**
     * Fills the contacts based on database's conversations
     * @return if the method succeeds
     */
    bool fillWithJamiContacts();

    /**
     * Add a contact::Info to contacts.
     * @note: the contactId must corresponds to a profile in the database.
     * @param contactId
     * @param type
     * @param banned whether contact is banned or not
     */
    void addToContacts(const std::string& contactId, const profile::Type& type, bool banned = false);
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

ContactModel::ContactModel(const account::Info& owner,
                           Database& db,
                           const CallbacksHandler& callbacksHandler,
                           const BehaviorController& behaviorController)
: owner(owner)
, pimpl_(std::make_unique<ContactModelPimpl>(*this, db, callbacksHandler, behaviorController))
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
    case profile::Type::COUNT__:
    default:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
        return;
    }

    storage::createProfile(owner.id, profile, true);

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
            storage::removeContact(pimpl_->db, contactUri);
            emitContactRemoved = true;
        }
        else if (owner.profileInfo.type == profile::Type::SIP) {
            // Remove contact from db
            pimpl_->contacts.erase(contactUri);
            storage::removeContact(pimpl_->db, contactUri);
            emitContactRemoved = true;
        }
    }
    // hang up calls with the removed contact as peer
    try{
        auto callinfo = owner.callModel->getCallFromURI(contactUri, true);
        owner.callModel->hangUp(callinfo.id);
    } catch (std::out_of_range& e){}
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
        ConfigurationManager::instance().lookupName(QString::fromStdString(linked.owner.id), "", QString::fromStdString(uriID));
    }
    emit linked.modelUpdated(uriID);
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
        fillWithSIPContacts();
    else
        fillWithJamiContacts();

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
ContactModelPimpl::fillWithSIPContacts()
{
    auto conversationsForAccount = storage::getAllConversations(db);
    for (const auto& convId : conversationsForAccount) {
        auto otherParticipants = storage::getPeerParticipantsForConversation(db, convId);
        for (const auto& participant: otherParticipants) {
            // for each conversations get the other profile id
            auto contactInfo = storage::buildContactFromProfile(linked.owner.id,
                                                                participant,
                                                                profile::Type::SIP);
            {
                std::lock_guard<std::mutex> lk(contactsMtx_);
                contacts.emplace(contactInfo.profileInfo.uri, contactInfo);
            }
        }
    }

    return true;
}

bool
ContactModelPimpl::fillWithJamiContacts() {

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

        auto contactInfo = storage::buildContactFromProfile(linked.owner.id,
                                                            contactUri.toStdString(),
                                                            profile::Type::PENDING);

        const auto vCard = lrc::vCard::utils::toHashMap(payload);
        const auto alias = vCard["FN"];
        const auto photo = (vCard.find("PHOTO;ENCODING=BASE64;TYPE=PNG") != vCard.end()) ?
            vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"] :
            vCard["PHOTO;ENCODING=BASE64;TYPE=JPEG"];

        contactInfo.profileInfo.type = profile::Type::PENDING;
        if (!alias.isEmpty()) contactInfo.profileInfo.alias = alias.constData();
        if (!photo.isEmpty()) contactInfo.profileInfo.avatar = photo.constData();
        contactInfo.registeredName = "";
        contactInfo.isBanned = false;

        {
            std::lock_guard<std::mutex> lk(contactsMtx_);
            contacts.emplace(contactUri.toStdString(), contactInfo);
        }

        // create profile vcard for contact
        storage::updateProfile(linked.owner.id, contactInfo.profileInfo, true);
    }

    // Update presence
    // TODO fix this map. This is dumb for now. The map contains values as keys, and empty values.
    const VectorMapStringString& subscriptions {
        PresenceManager::instance().getSubscriptions(linked.owner.id.c_str())
    };
    for (const auto& subscription : subscriptions) {
        auto first = true;
        std::string uri = "";
        for (const auto& key : subscription) {
            if (first) {
                first = false;
                uri = key.toStdString();
            } else {
                {
                    std::lock_guard<std::mutex> lk(contactsMtx_);
                    auto it = contacts.find(uri);
                    if (it != contacts.end()) {
                        it->second.isPresent = key == "Online";
                        linked.modelUpdated(uri, false);
                    }
                }
                break;
            }
        }
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

        if (contact->second.profileInfo.type != profile::Type::SIP)
            PresenceManager::instance().subscribeBuddy(linked.owner.id.c_str(), contactUri.c_str(), false);

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
            storage::removeContact(db, contactUri);
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
ContactModelPimpl::addToContacts(const std::string& contactUri, const profile::Type& type, bool banned)
{
    // create a vcard if necessary
    profile::Info profileInfo{ contactUri, {}, {}, linked.owner.profileInfo.type };
    storage::createProfile(linked.owner.id, profileInfo, true);

    auto contactInfo = storage::buildContactFromProfile(linked.owner.id, contactUri, type);
    contactInfo.isBanned = banned;
    contactInfo.profileInfo.type = type; // PENDING should not be stored in the database

    // lookup address in case of RING contact
    if (type == profile::Type::RING) {
        ConfigurationManager::instance().lookupAddress(QString::fromStdString(linked.owner.id),
                                                       "", QString::fromStdString(contactUri));
        PresenceManager::instance().subscribeBuddy(linked.owner.id.c_str(), contactUri.c_str(), !banned);
    }

    contactInfo.profileInfo.type = type; // Because PENDING should not be stored in the database
    auto iter = contacts.find(contactInfo.profileInfo.uri);
    if (iter != contacts.end()) {
        auto info = iter->second;
        contactInfo.registeredName = info.registeredName;
        iter->second = contactInfo;
    } else
        contacts.emplace_hint(iter, contactInfo.profileInfo.uri, contactInfo);

    if (banned) {
        bannedContacts.emplace_back(contactUri);
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
            updateTemporaryMessage(tr("Invalid ID").toStdString(), registeredName);
            break;
        case 2 /* NOT FOUND */:
            updateTemporaryMessage(tr("Registered name not found").toStdString(), registeredName);
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
            const auto vCard = lrc::vCard::utils::toHashMap(payload.c_str());
            const auto alias = vCard["FN"];
            const auto photo = (vCard.find("PHOTO;ENCODING=BASE64;TYPE=PNG") == vCard.end()) ?
            vCard["PHOTO;ENCODING=BASE64;TYPE=JPEG"] : vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

            auto profileInfo = profile::Info {contactUri, photo.toStdString(), alias.toStdString(), profile::Type::PENDING};
            auto contactInfo = contact::Info {profileInfo, "", false, false, false};
            contacts.emplace(contactUri, contactInfo);
            emitTrust = true;
            storage::updateProfile(accountId, profileInfo, true);
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
            auto type = (linked.owner.profileInfo.type == profile::Type::RING)
                            ? profile::Type::PENDING
                            : profile::Type::SIP;
            addToContacts(from, type, false);
            emitNewTrust = (linked.owner.profileInfo.type == profile::Type::RING);
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
            auto type = (linked.owner.profileInfo.type == profile::Type::RING)
                            ? profile::Type::PENDING
                            : profile::Type::SIP;
            addToContacts(info.peerUri, type, false);
            emitNewTrust = (linked.owner.profileInfo.type == profile::Type::RING);
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

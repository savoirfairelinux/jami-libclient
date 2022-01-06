/****************************************************************************
 *    Copyright (C) 2017-2022 Savoir-faire Linux Inc.                       *
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

// LRC
#include "api/account.h"
#include "api/contact.h"
#include "api/conversationmodel.h"
#include "api/interaction.h"
#include "api/lrc.h"
#include "api/newaccountmodel.h"
#include "api/newcallmodel.h"
#include "callbackshandler.h"
#include "uri.h"
#include "vcard.h"
#include "typedefs.h"

#include "authority/daemon.h"
#include "authority/storagehelper.h"

// Dbus
#include "dbus/configurationmanager.h"
#include "dbus/presencemanager.h"

#include "account_const.h"

// Std
#include <algorithm>
#include <mutex>

namespace lrc {

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
     * @param displayName
     * @param banned whether contact is banned or not
     * @param conversationId linked swarm if one
     */
    void addToContacts(const QString& contactId,
                       const profile::Type& type,
                       const QString& displayName = "",
                       bool banned = false,
                       const QString& conversationId = "");
    /**
     * Helpers for searchContact. Search for a given classic or SIP contact.
     */
    void searchContact(const URI& query);
    void searchSipContact(const URI& query);

    /**
     * Update temporary item to display a given message about a given uri.
     */
    void updateTemporaryMessage(const QString& mes);

    /**
     * Check if equivalent uri exist in contact
     */
    QString sipUriReceivedFilter(const QString& uri);

    // Helpers
    const BehaviorController& behaviorController;
    const ContactModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    // Containers
    ContactModel::ContactInfoMap contacts;
    ContactModel::ContactInfoMap searchResult;
    QList<QString> bannedContacts;
    QString searchQuery;
    std::mutex contactsMtx_;
    std::mutex bannedContactsMtx_;
    QString searchStatus_ {};

public Q_SLOTS:
    /**
     * Listen CallbacksHandler when a presence update occurs
     * @param accountId
     * @param contactUri
     * @param status
     */
    void slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status);

    /**
     * Listen CallbacksHandler when a contact is added
     * @param accountId
     * @param contactUri
     * @param confirmed
     */
    void slotContactAdded(const QString& accountId, const QString& contactUri, bool confirmed);

    /**
     * Listen CallbacksHandler when a contact is removed
     * @param accountId
     * @param contactUri
     * @param banned
     */
    void slotContactRemoved(const QString& accountId, const QString& contactUri, bool banned);

    /**
     * Listen CallbacksHandler when a registeredName is found
     * @param accountId account linked
     * @param status (0 = SUCCESS, 1 = Not found, 2 = Network error)
     * @param uri of the contact found
     * @param registeredName of the contact found
     */
    void slotRegisteredNameFound(const QString& accountId,
                                 int status,
                                 const QString& uri,
                                 const QString& registeredName);

    /**
     * Listen CallbacksHandler when an incoming request arrives
     * @param accountId account linked
     * @param contactUri
     * @param payload VCard of the contact
     */
    void slotIncomingContactRequest(const QString& accountId,
                                    const QString& conversationId,
                                    const QString& contactUri,
                                    const QString& payload);
    /**
     * Listen from callModel when an incoming call arrives.
     * @param fromId
     * @param callId
     * @param displayName
     */
    void slotIncomingCall(const QString& fromId, const QString& callId, const QString& displayname);

    /**
     * Listen from callbacksHandler for new account interaction and add pending contact if not present
     * @param accountId
     * @param msgId
     * @param peerId
     * @param payloads
     */
    void slotNewAccountMessage(const QString& accountId,
                               const QString& peerId,
                               const QString& msgId,
                               const MapStringString& payloads);

    /**
     * Listen from callbacksHandler to know when a file transfer interaction is incoming
     * @param fileId Daemon's ID for incoming transfer
     * @param transferInfo DataTransferInfo structure from daemon
     */
    void slotNewAccountTransfer(const QString& fileId, datatransfer::Info info);

    /**
     * Listen from daemon to know when a VCard is received
     * @param accountId
     * @param peer
     * @param vCard
     */
    void slotProfileReceived(const QString& accountId, const QString& peer, const QString& vCard);

    /**
     * Listen from daemon to know when a user search completed
     * @param accountId
     * @param status
     * @param query
     * @param result
     */
    void slotUserSearchEnded(const QString& accountId,
                             int status,
                             const QString& query,
                             const VectorMapStringString& result);
};

using namespace authority;

ContactModel::ContactModel(const account::Info& owner,
                           Database& db,
                           const CallbacksHandler& callbacksHandler,
                           const BehaviorController& behaviorController)
    : owner(owner)
    , pimpl_(std::make_unique<ContactModelPimpl>(*this, db, callbacksHandler, behaviorController))
{}

ContactModel::~ContactModel() {}

const ContactModel::ContactInfoMap&
ContactModel::getAllContacts() const
{
    return pimpl_->contacts;
}

time_t
ContactModel::getAddedTs(const QString& contactUri) const
{
    MapStringString details = ConfigurationManager::instance().getContactDetails(owner.id,
                                                                                 contactUri);
    auto itAdded = details.find("added");
    if (itAdded == details.end())
        return 0;
    return itAdded.value().toUInt();
}

void
ContactModel::addContact(contact::Info contactInfo)
{
    auto& profile = contactInfo.profileInfo;

    // If passed contact is a banned contact, call the daemon to unban it
    auto it = std::find(pimpl_->bannedContacts.begin(), pimpl_->bannedContacts.end(), profile.uri);
    if (it != pimpl_->bannedContacts.end()) {
        qDebug() << QString("Unban-ing contact %1").arg(profile.uri);
        ConfigurationManager::instance().addContact(owner.id, profile.uri);
        // bannedContacts will be updated in slotContactAdded
        return;
    }

    if ((owner.profileInfo.type != profile.type)
        and (profile.type == profile::Type::JAMI or profile.type == profile::Type::SIP)) {
        qDebug() << "ContactModel::addContact, types invalid.";
        return;
    }

    MapStringString details = ConfigurationManager::instance()
                                  .getContactDetails(owner.id, contactInfo.profileInfo.uri);

    // if contactInfo is already a contact for the daemon, type should be equals to RING
    // if the user add a temporary item for a SIP account, should be directly transformed
    if (!details.empty()
        || (profile.type == profile::Type::TEMPORARY
            && owner.profileInfo.type == profile::Type::SIP))
        profile.type = owner.profileInfo.type;

    switch (profile.type) {
    case profile::Type::TEMPORARY: {
        // make a temporary contact available for UI elements, it will be upgraded to
        // its corresponding type after receiving contact added signal
        std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
        contactInfo.profileInfo.type = profile::Type::PENDING;
        pimpl_->contacts.insert(contactInfo.profileInfo.uri, contactInfo);
        ConfigurationManager::instance().addContact(owner.id, profile.uri);
        ConfigurationManager::instance()
            .sendTrustRequest(owner.id,
                              profile.uri,
                              owner.accountModel->accountVCard(owner.id).toUtf8());
        return;
    }
    case profile::Type::PENDING:
        if (daemon::addContactFromPending(owner, profile.uri)) {
            emit pendingContactAccepted(profile.uri);
        } else {
            return;
        }
        break;
    case profile::Type::JAMI:
    case profile::Type::SIP:
        break;
    case profile::Type::INVALID:
    case profile::Type::COUNT__:
    default:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
        return;
    }

    storage::createOrUpdateProfile(owner.id, profile, true);

    {
        std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
        auto iter = pimpl_->contacts.find(contactInfo.profileInfo.uri);
        if (iter == pimpl_->contacts.end())
            pimpl_->contacts.insert(iter, contactInfo.profileInfo.uri, contactInfo);
        else {
            // On non-DBus platform, contactInfo.profileInfo.type may be wrong as the contact
            // may be trusted already. We must use Profile::Type from pimpl_->contacts
            // and not from contactInfo so we cannot revert a contact back to PENDING.
            contactInfo.profileInfo.type = iter->profileInfo.type;
            iter->profileInfo = contactInfo.profileInfo;
        }
    }
    emit profileUpdated(profile.uri);
    if (profile.type == profile::Type::SIP)
        emit contactAdded(profile.uri);
}

void
ContactModel::addToContacts(const QString& contactUri)
{
    std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
    auto iter = pimpl_->contacts.find(contactUri);
    if (iter != pimpl_->contacts.end())
        return;

    auto contactInfo = storage::buildContactFromProfile(owner.id,
                                                        contactUri,
                                                        profile::Type::PENDING);
    pimpl_->contacts.insert(iter, contactUri, contactInfo);
    ConfigurationManager::instance().lookupAddress(owner.id, "", contactUri);
}

void
ContactModel::removeContact(const QString& contactUri, bool banned)
{
    bool emitContactRemoved = false;
    {
        std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
        auto contact = pimpl_->contacts.find(contactUri);
        if (!banned && contact != pimpl_->contacts.end()
            && contact->profileInfo.type == profile::Type::PENDING) {
            // Discard the pending request and remove profile from db if necessary
            if (!daemon::discardFromPending(owner, contactUri)) {
                qDebug() << "Discard request for account " << owner.id << " failed (" << contactUri
                         << ")";
                return;
            }
            pimpl_->contacts.remove(contactUri);
            storage::removeContactConversations(pimpl_->db, contactUri);
            storage::removeProfile(owner.id, contactUri);
            emitContactRemoved = true;
        } else if (owner.profileInfo.type == profile::Type::SIP) {
            // Remove contact from db
            pimpl_->contacts.remove(contactUri);
            storage::removeContactConversations(pimpl_->db, contactUri);
            storage::removeProfile(owner.id, contactUri);
            emitContactRemoved = true;
        }
    }
    // hang up calls with the removed contact as peer
    try {
        auto callinfo = owner.callModel->getCallFromURI(contactUri, true);
        owner.callModel->hangUp(callinfo.id);
    } catch (std::out_of_range& e) {
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
ContactModel::getContact(const QString& contactUri) const
{
    std::lock_guard<std::mutex> lk(pimpl_->contactsMtx_);
    if (pimpl_->contacts.contains(contactUri)) {
        return pimpl_->contacts.value(contactUri);
    } else if (pimpl_->searchResult.contains(contactUri)) {
        return pimpl_->searchResult.value(contactUri);
    }
    throw std::out_of_range("Contact out of range");
}

const QList<QString>&
ContactModel::getBannedContacts() const
{
    return pimpl_->bannedContacts;
}

ContactModel::ContactInfoMap
ContactModel::getSearchResults() const
{
    return pimpl_->searchResult;
}

void
ContactModel::searchContact(const QString& query)
{
    // always reset temporary contact
    pimpl_->searchResult.clear();

    auto uri = URI(query);
    QString uriID = uri.format(URI::Section::USER_INFO | URI::Section::HOSTNAME
                               | URI::Section::PORT);
    pimpl_->searchQuery = uriID;

    auto uriScheme = uri.schemeType();
    if (static_cast<int>(uriScheme) > 2 && owner.profileInfo.type == profile::Type::SIP) {
        // sip account do not care if schemeType is NONE, or UNRECOGNIZED (enum value > 2)
        uriScheme = URI::SchemeType::SIP;
    } else if (uriScheme == URI::SchemeType::NONE && owner.profileInfo.type == profile::Type::JAMI) {
        uriScheme = URI::SchemeType::RING;
    }

    if ((uriScheme == URI::SchemeType::SIP || uriScheme == URI::SchemeType::SIPS)
        && owner.profileInfo.type == profile::Type::SIP) {
        pimpl_->searchSipContact(uri);
    } else if (uriScheme == URI::SchemeType::RING && owner.profileInfo.type == profile::Type::JAMI) {
        pimpl_->searchContact(uri);
    } else {
        pimpl_->updateTemporaryMessage(tr("Bad URI scheme"));
    }
}

void
ContactModelPimpl::updateTemporaryMessage(const QString& mes)
{
    if (searchStatus_ != mes) {
        searchStatus_ = mes;
        linked.owner.conversationModel->updateSearchStatus(mes);
    }
}

void
ContactModelPimpl::searchContact(const URI& query)
{
    QString uriID = query.format(URI::Section::USER_INFO | URI::Section::HOSTNAME
                                 | URI::Section::PORT);
    if (query.isEmpty()) {
        // This will remove the temporary item
        emit linked.modelUpdated(uriID);
        updateTemporaryMessage("");
        return;
    }

    if (query.protocolHint() == URI::ProtocolHint::RING) {
        updateTemporaryMessage("");
        // no lookup, this is a ring infoHash
        for (auto& i : contacts) {
            if (i.profileInfo.uri == uriID) {
                return;
            }
        }
        auto& temporaryContact = searchResult[uriID];
        temporaryContact.profileInfo.uri = uriID;
        temporaryContact.profileInfo.alias = uriID;
        temporaryContact.profileInfo.type = profile::Type::TEMPORARY;
        emit linked.modelUpdated(uriID);
    } else {
        updateTemporaryMessage(tr("Searching…"));

        // If the username contains an @ it's an exact match
        bool isJamsAccount = !linked.owner.confProperties.managerUri.isEmpty();
        if (isJamsAccount and not query.hasHostname())
            ConfigurationManager::instance().searchUser(linked.owner.id, uriID);
        else
            ConfigurationManager::instance().lookupName(linked.owner.id, "", uriID);
    }
}

void
ContactModelPimpl::searchSipContact(const URI& query)
{
    QString uriID = query.format(URI::Section::USER_INFO | URI::Section::HOSTNAME
                                 | URI::Section::PORT);
    if (query.isEmpty()) {
        // This will remove the temporary item
        emit linked.modelUpdated(uriID);
        updateTemporaryMessage("");
        return;
    }

    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(uriID) == contacts.end()) {
            auto& temporaryContact = searchResult[query];

            temporaryContact.profileInfo.uri = uriID;
            temporaryContact.profileInfo.alias = uriID;
            temporaryContact.profileInfo.type = profile::Type::TEMPORARY;
        }
    }
    emit linked.modelUpdated(uriID);
}

uint64_t
ContactModel::sendDhtMessage(const QString& contactUri, const QString& body) const
{
    // Send interaction
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body;
    auto msgId = ConfigurationManager::instance().sendTextMessage(QString(owner.id),
                                                                  QString(contactUri),
                                                                  payloads);
    // NOTE: ConversationModel should store the interaction into the database
    return msgId;
}

const QString
ContactModel::bestNameForContact(const QString& contactUri) const
{
    try {
        auto contact = getContact(contactUri);
        auto alias = contact.profileInfo.alias.simplified();

        if (alias.isEmpty()) {
            return bestIdFromContactInfo(contact);
        }
        return alias;
    } catch (const std::out_of_range& e) {
        qDebug() << "ContactModel::bestNameForContact" << e.what();
    }

    return contactUri;
}

QString
ContactModel::avatar(const QString& uri) const
{
    return storage::avatar(owner.id, uri);
}

const QString
ContactModel::bestIdForContact(const QString& contactUri) const
{
    try {
        auto contact = getContact(contactUri);
        auto alias = contact.profileInfo.alias.simplified();
        if (alias.isEmpty()) {
            return {};
        }
        return bestIdFromContactInfo(contact);
    } catch (const std::out_of_range& e) {
        qDebug() << "ContactModel::bestIdForContact" << e.what();
    }

    return contactUri;
}

const QString
ContactModel::bestIdFromContactInfo(const contact::Info& contactInfo) const
{
    auto registeredName = contactInfo.registeredName.simplified();
    auto infoHash = contactInfo.profileInfo.uri.simplified();

    if (!registeredName.isEmpty()) {
        return registeredName;
    }
    return infoHash;
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
    connect(&callbacksHandler,
            &CallbacksHandler::newBuddySubscription,
            this,
            &ContactModelPimpl::slotNewBuddySubscription);
    connect(&callbacksHandler,
            &CallbacksHandler::contactAdded,
            this,
            &ContactModelPimpl::slotContactAdded);
    connect(&callbacksHandler,
            &CallbacksHandler::contactRemoved,
            this,
            &ContactModelPimpl::slotContactRemoved);
    connect(&callbacksHandler,
            &CallbacksHandler::incomingContactRequest,
            this,
            &ContactModelPimpl::slotIncomingContactRequest);
    connect(&callbacksHandler,
            &CallbacksHandler::registeredNameFound,
            this,
            &ContactModelPimpl::slotRegisteredNameFound);
    connect(&*linked.owner.callModel,
            &NewCallModel::newIncomingCall,
            this,
            &ContactModelPimpl::slotIncomingCall);
    connect(&callbacksHandler,
            &lrc::CallbacksHandler::newAccountMessage,
            this,
            &ContactModelPimpl::slotNewAccountMessage);
    connect(&callbacksHandler,
            &CallbacksHandler::transferStatusCreated,
            this,
            &ContactModelPimpl::slotNewAccountTransfer);
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::profileReceived,
            this,
            &ContactModelPimpl::slotProfileReceived);
    connect(&ConfigurationManager::instance(),
            &ConfigurationManagerInterface::userSearchEnded,
            this,
            &ContactModelPimpl::slotUserSearchEnded);
}

ContactModelPimpl::~ContactModelPimpl()
{
    disconnect(&callbacksHandler,
               &CallbacksHandler::newBuddySubscription,
               this,
               &ContactModelPimpl::slotNewBuddySubscription);
    disconnect(&callbacksHandler,
               &CallbacksHandler::contactAdded,
               this,
               &ContactModelPimpl::slotContactAdded);
    disconnect(&callbacksHandler,
               &CallbacksHandler::contactRemoved,
               this,
               &ContactModelPimpl::slotContactRemoved);
    disconnect(&callbacksHandler,
               &CallbacksHandler::incomingContactRequest,
               this,
               &ContactModelPimpl::slotIncomingContactRequest);
    disconnect(&callbacksHandler,
               &CallbacksHandler::registeredNameFound,
               this,
               &ContactModelPimpl::slotRegisteredNameFound);
    disconnect(&*linked.owner.callModel,
               &NewCallModel::newIncomingCall,
               this,
               &ContactModelPimpl::slotIncomingCall);
    disconnect(&callbacksHandler,
               &lrc::CallbacksHandler::newAccountMessage,
               this,
               &ContactModelPimpl::slotNewAccountMessage);
    disconnect(&callbacksHandler,
               &CallbacksHandler::transferStatusCreated,
               this,
               &ContactModelPimpl::slotNewAccountTransfer);
    disconnect(&ConfigurationManager::instance(),
               &ConfigurationManagerInterface::profileReceived,
               this,
               &ContactModelPimpl::slotProfileReceived);
    disconnect(&ConfigurationManager::instance(),
               &ConfigurationManagerInterface::userSearchEnded,
               this,
               &ContactModelPimpl::slotUserSearchEnded);
}

bool
ContactModelPimpl::fillWithSIPContacts()
{
    auto conversationsForAccount = storage::getAllConversations(db);
    for (const auto& convId : conversationsForAccount) {
        auto otherParticipants = storage::getPeerParticipantsForConversation(db, convId);
        for (const auto& participant : otherParticipants) {
            // for each conversations get the other profile id
            auto contactInfo = storage::buildContactFromProfile(linked.owner.id,
                                                                participant,
                                                                profile::Type::SIP);
            {
                std::lock_guard<std::mutex> lk(contactsMtx_);
                contacts.insert(contactInfo.profileInfo.uri, contactInfo);
            }
        }
    }

    return true;
}

bool
ContactModelPimpl::fillWithJamiContacts()
{
    // Add contacts from daemon
    const VectorMapStringString& contacts_vector = ConfigurationManager::instance().getContacts(
        linked.owner.id);
    for (auto contact_info : contacts_vector) {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        bool banned = contact_info["banned"] == "true" ? true : false;
        addToContacts(contact_info["id"],
                      linked.owner.profileInfo.type,
                      "",
                      banned,
                      contact_info["conversationId"]);
    }

    // Add pending contacts
    const VectorMapStringString& pending_tr {
        ConfigurationManager::instance().getTrustRequests(linked.owner.id)};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();
        auto contactUri = tr_info[DRing::Account::TrustRequest::FROM];
        auto convId = tr_info[DRing::Account::TrustRequest::CONVERSATIONID];
        if (!convId.isEmpty())
            continue; // This will be added via getConversationsRequests

        auto contactInfo = storage::buildContactFromProfile(linked.owner.id,
                                                            contactUri,
                                                            profile::Type::PENDING);

        const auto vCard = lrc::vCard::utils::toHashMap(payload);
        const auto alias = vCard["FN"];
        QByteArray photo;
        for (const auto& key : vCard.keys()) {
            if (key.contains("PHOTO") && lrc::api::Lrc::cacheAvatars.load())
                photo = vCard[key];
        }
        contactInfo.profileInfo.type = profile::Type::PENDING;
        if (!alias.isEmpty())
            contactInfo.profileInfo.alias = alias.constData();
        if (!photo.isEmpty())
            contactInfo.profileInfo.avatar = photo.constData();
        contactInfo.registeredName = "";
        contactInfo.isBanned = false;

        {
            std::lock_guard<std::mutex> lk(contactsMtx_);
            contacts.insert(contactUri, contactInfo);
        }

        // create profile vcard for contact
        storage::createOrUpdateProfile(linked.owner.id, contactInfo.profileInfo, true);
    }

    // Update presence
    // TODO fix this map. This is dumb for now. The map contains values as keys, and empty values.
    const VectorMapStringString& subscriptions {
        PresenceManager::instance().getSubscriptions(linked.owner.id)};
    for (const auto& subscription : subscriptions) {
        auto first = true;
        QString uri = "";
        for (const auto& key : subscription) {
            if (first) {
                first = false;
                uri = key;
            } else {
                {
                    std::lock_guard<std::mutex> lk(contactsMtx_);
                    auto it = contacts.find(uri);
                    if (it != contacts.end()) {
                        it->isPresent = key == "Online";
                        linked.modelUpdated(uri);
                    }
                }
                break;
            }
        }
    }
    return true;
}

void
ContactModelPimpl::slotNewBuddySubscription(const QString& accountId,
                                            const QString& contactUri,
                                            bool status)
{
    if (accountId != linked.owner.id)
        return;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        auto it = contacts.find(contactUri);
        if (it != contacts.end()) {
            it->isPresent = status;
        } else
            return;
    }
    emit linked.modelUpdated(contactUri);
}

void
ContactModelPimpl::slotContactAdded(const QString& accountId,
                                    const QString& contactUri,
                                    bool confirmed)
{
    if (accountId != linked.owner.id)
        return;
    auto contact = contacts.find(contactUri);
    if (contact != contacts.end()) {
        if (contact->profileInfo.type == profile::Type::PENDING) {
            emit behaviorController.trustRequestTreated(linked.owner.id, contactUri);
        } else if (contact->profileInfo.type == profile::Type::JAMI && !contact->isBanned
                   && confirmed) {
            // This means that the peer accepted the trust request. We don't need to re-add the
            // contact a second time (and this reset the presence to false).
            return;
        }
    }
    // for jams account we already have profile with avatar, use it to save to vCard
    bool isJamsAccount = !linked.owner.confProperties.managerUri.isEmpty();
    if (isJamsAccount) {
        auto result = searchResult.find(contactUri);
        if (result != searchResult.end()) {
            storage::createOrUpdateProfile(linked.owner.id, result->profileInfo, true);
        }
    }

    bool isBanned = false;

    {
        // Always get contactsMtx_ lock before bannedContactsMtx_.
        std::lock_guard<std::mutex> lk(contactsMtx_);

        {
            // Check whether contact is banned or not
            std::lock_guard<std::mutex> lk(bannedContactsMtx_);
            auto it = std::find(bannedContacts.begin(), bannedContacts.end(), contactUri);

            isBanned = (it != bannedContacts.end());

            // If contact is banned, do not re-add it, simply update its flag and the banned contacts list
            if (isBanned) {
                bannedContacts.erase(it);
            }

            MapStringString details = ConfigurationManager::instance()
                                          .getContactDetails(linked.owner.id, contactUri);
            addToContacts(contactUri,
                          linked.owner.profileInfo.type,
                          "",
                          false,
                          details["conversationId"]);
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
ContactModelPimpl::slotContactRemoved(const QString& accountId,
                                      const QString& contactUri,
                                      bool banned)
{
    if (accountId != linked.owner.id)
        return;

    {
        // Always get contactsMtx_ lock before bannedContactsMtx_.
        std::lock_guard<std::mutex> lk(contactsMtx_);

        auto contact = contacts.find(contactUri);
        if (contact == contacts.end())
            return;

        if (contact->profileInfo.type == profile::Type::PENDING) {
            emit behaviorController.trustRequestTreated(linked.owner.id, contactUri);
        }

        if (contact->profileInfo.type != profile::Type::SIP)
            PresenceManager::instance().subscribeBuddy(linked.owner.id, contactUri, false);

        if (banned) {
            contact->isBanned = true;
            // Update bannedContacts index
            bannedContacts.append(contact->profileInfo.uri);
        } else {
            if (contact->isBanned) {
                // Contact was banned, update bannedContacts
                std::lock_guard<std::mutex> lk(bannedContactsMtx_);
                auto it = std::find(bannedContacts.begin(),
                                    bannedContacts.end(),
                                    contact->profileInfo.uri);
                if (it == bannedContacts.end()) {
                    // should not happen
                    qDebug("ContactModel::slotContactsRemoved(): Contact is banned but not present "
                           "in bannedContacts. This is most likely the result of an earlier bug.");
                } else {
                    bannedContacts.erase(it);
                }
            }
            storage::removeContactConversations(db, contactUri);
            storage::removeProfile(linked.owner.id, contactUri);
            contacts.remove(contactUri);
        }
    }

    // Update the smartlist
    linked.owner.conversationModel->refreshFilter();
    if (banned) {
        emit linked.bannedStatusChanged(contactUri, true);
    }
    emit linked.contactRemoved(contactUri);
}

void
ContactModelPimpl::addToContacts(const QString& contactUri,
                                 const profile::Type& type,
                                 const QString& displayName,
                                 bool banned,
                                 const QString& conversationId)
{
    // create a vcard if necessary
    profile::Info profileInfo {contactUri, {}, displayName, linked.owner.profileInfo.type};
    auto contactInfo = storage::buildContactFromProfile(linked.owner.id, contactUri, type);
    auto updateProfile = false;
    if (!profileInfo.alias.isEmpty() && contactInfo.profileInfo.alias != profileInfo.alias) {
        updateProfile = true;
        contactInfo.profileInfo.alias = profileInfo.alias;
    }
    auto oldAvatar = lrc::api::Lrc::cacheAvatars.load()? contactInfo.profileInfo.avatar : storage::avatar(linked.owner.id, contactUri);
    if (!profileInfo.avatar.isEmpty() && oldAvatar != profileInfo.avatar) {
        updateProfile = true;
        contactInfo.profileInfo.avatar = profileInfo.avatar;
    }
    if (updateProfile)
        storage::vcard::setProfile(linked.owner.id, contactInfo.profileInfo, true);

    contactInfo.isBanned = banned;
    contactInfo.conversationId = conversationId;
    if (!lrc::api::Lrc::cacheAvatars.load())
        contactInfo.profileInfo.avatar.clear();

    // lookup address in case of RING contact
    if (type == profile::Type::JAMI) {
        ConfigurationManager::instance().lookupAddress(linked.owner.id, "", contactUri);
        PresenceManager::instance().subscribeBuddy(linked.owner.id, contactUri, !banned);
    } else {
        contactInfo.profileInfo.alias = displayName;
    }

    contactInfo.profileInfo.type = type; // Because PENDING should not be stored in the database
    auto iter = contacts.find(contactInfo.profileInfo.uri);
    if (iter != contacts.end()) {
        auto info = iter.value();
        contactInfo.registeredName = info.registeredName;
        iter.value() = contactInfo;
    } else
        contacts.insert(iter, contactInfo.profileInfo.uri, contactInfo);

    if (banned) {
        bannedContacts.append(contactUri);
    }
}

void
ContactModelPimpl::slotRegisteredNameFound(const QString& accountId,
                                           int status,
                                           const QString& uri,
                                           const QString& registeredName)
{
    if (accountId != linked.owner.id)
        return;

    if (status == 0 /* SUCCESS */) {
        std::lock_guard<std::mutex> lk(contactsMtx_);

        if (contacts.find(uri) != contacts.end()) {
            // update contact and remove temporary item
            contacts[uri].registeredName = registeredName;
            searchResult.clear();
        } else {
            if ((searchQuery != uri && searchQuery != registeredName) || searchQuery.isEmpty()) {
                // we are notified that a previous lookup ended
                return;
            }
            auto& temporaryContact = searchResult[uri];
            lrc::api::profile::Info profileInfo = {uri, "", "", profile::Type::TEMPORARY};
            temporaryContact = {profileInfo, registeredName, false, false};
        }
    } else {
        {
            std::lock_guard<std::mutex> lk(contactsMtx_);
            if (contacts.find(uri) != contacts.end()) {
                // it was lookup for contact
                return;
            }
        }
        if ((searchQuery != uri && searchQuery != registeredName) || searchQuery.isEmpty()) {
            // we are notified that a previous lookup ended
            return;
        }
        switch (status) {
        case 1 /* INVALID */:
            updateTemporaryMessage(tr("Invalid ID"));
            break;
        case 2 /* NOT FOUND */:
            updateTemporaryMessage(tr("Username not found"));
            break;
        case 3 /* ERROR */:
            updateTemporaryMessage(tr("Couldn't lookup…"));
            break;
        }
        return;
    }
    updateTemporaryMessage("");
    emit linked.modelUpdated(uri);
}

void
ContactModelPimpl::slotIncomingContactRequest(const QString& accountId,
                                              const QString& conversationId,
                                              const QString& contactUri,
                                              const QString& payload)
{
    if (linked.owner.id != accountId)
        return;

    auto emitTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(contactUri) == contacts.end()) {
            const auto vCard = lrc::vCard::utils::toHashMap(payload.toUtf8());
            const auto alias = vCard["FN"];
            QByteArray photo;
            for (const auto& key : vCard.keys()) {
                if (key.contains("PHOTO"))
                    photo = vCard[key];
            }
            auto profileInfo = profile::Info {contactUri, photo, alias, profile::Type::PENDING};
            auto contactInfo = contact::Info {profileInfo, "", false, false, false};
            contacts.insert(contactUri, contactInfo);
            emitTrust = true;
            storage::createOrUpdateProfile(accountId, profileInfo, true);
            ConfigurationManager::instance().lookupAddress(linked.owner.id, "", contactUri);
        }
    }
    emit linked.incomingContactRequest(contactUri);
    if (emitTrust) {
        emit behaviorController.newTrustRequest(linked.owner.id, conversationId, contactUri);
    }
}

void
ContactModelPimpl::slotIncomingCall(const QString& fromId,
                                    const QString& callId,
                                    const QString& displayname)
{
    bool emitContactAdded = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        auto it = contacts.find(fromId);
        if (it == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.
            auto type = (linked.owner.profileInfo.type == profile::Type::JAMI)
                            ? profile::Type::PENDING
                            : profile::Type::SIP;
            addToContacts(fromId, type, displayname, false);
            emitContactAdded = true;
        } else {
            // Update the display name
            if (!displayname.isEmpty()) {
                it->profileInfo.alias = displayname;
                storage::createOrUpdateProfile(linked.owner.id, it->profileInfo, true);
            }
        }
    }
    if (emitContactAdded) {
        if (linked.owner.profileInfo.type == profile::Type::SIP)
            emit linked.contactAdded(fromId);
        else if (linked.owner.profileInfo.type == profile::Type::JAMI)
            emit behaviorController.newTrustRequest(linked.owner.id, "", fromId);
    } else
        emit linked.profileUpdated(fromId);

    emit linked.incomingCall(fromId, callId);
}

void
ContactModelPimpl::slotNewAccountMessage(const QString& accountId,
                                         const QString& peerId,
                                         const QString& msgId,
                                         const MapStringString& payloads)
{
    if (accountId != linked.owner.id)
        return;

    QString peerId2(peerId);

    auto emitNewTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        if (contacts.find(peerId) == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.

            if (linked.owner.profileInfo.type == profile::Type::SIP) {
                QString potentialContact = sipUriReceivedFilter(peerId);
                if (potentialContact.isEmpty()) {
                    addToContacts(peerId, profile::Type::SIP, "", false);
                } else {
                    // equivalent uri exist, use that uri
                    peerId2 = potentialContact;
                }
            } else {
                addToContacts(peerId, profile::Type::PENDING, "", false);
                emitNewTrust = true;
            }
        }
    }
    if (emitNewTrust) {
        emit behaviorController.newTrustRequest(linked.owner.id, "", peerId);
    }
    emit linked.newAccountMessage(accountId, peerId2, msgId, payloads);
}

QString
ContactModelPimpl::sipUriReceivedFilter(const QString& uri)
{
    // this function serves when the uri is not found in the contact list
    // return "" means need to add new contact, else means equivalent uri exist
    std::string uriCopy = uri.toStdString();

    auto pos = uriCopy.find("@");
    auto ownerHostName = linked.owner.confProperties.hostname.toStdString();

    if (pos != std::string::npos) {
        // "@" is found, separate username and hostname
        std::string hostName = uriCopy.substr(pos + 1);
        uriCopy.erase(uriCopy.begin() + pos, uriCopy.end());
        std::string remoteUser = std::move(uriCopy);

        if (hostName.compare(ownerHostName) == 0) {
            auto remoteUserQStr = QString::fromStdString(remoteUser);
            if (contacts.find(remoteUserQStr) != contacts.end()) {
                return remoteUserQStr;
            }
            if (remoteUser.at(0) == '+') {
                // "+" - country dial-in codes
                // maximum 3 digits
                for (int i = 2; i <= 4; i++) {
                    QString tempUserName = QString::fromStdString(remoteUser.substr(i));
                    if (contacts.find(tempUserName) != contacts.end()) {
                        return tempUserName;
                    }
                }
                return "";
            } else {
                // if not "+"  from incoming
                // sub "+" char from contacts to see if user exit
                for (auto& contactUri : contacts.keys()) {
                    if (!contactUri.isEmpty()) {
                        for (int j = 2; j <= 4; j++) {
                            if (QString(contactUri).remove(0, j) == remoteUserQStr) {
                                return contactUri;
                            }
                        }
                    }
                }
                return "";
            }
        }
        // different hostname means not a phone number
        // no need to check country dial-in codes
        return "";
    }
    // "@" is not found -> not possible since all response uri has one
    return "";
}

void
ContactModelPimpl::slotNewAccountTransfer(const QString& fileId, datatransfer::Info info)
{
    if (info.accountId != linked.owner.id)
        return;

    bool emitNewTrust = false;
    {
        std::lock_guard<std::mutex> lk(contactsMtx_);
        // Note: just add a contact for compatibility (so not for swarm).
        if (info.conversationId.isEmpty() && !info.peerUri.isEmpty()
            && contacts.find(info.peerUri) == contacts.end()) {
            // Contact not found, load profile from database.
            // The conversation model will create an entry and link the incomingCall.
            auto type = (linked.owner.profileInfo.type == profile::Type::JAMI)
                            ? profile::Type::PENDING
                            : profile::Type::SIP;
            addToContacts(info.peerUri, type, "", false);
            emitNewTrust = (linked.owner.profileInfo.type == profile::Type::JAMI);
        }
    }
    if (emitNewTrust) {
        emit behaviorController.newTrustRequest(linked.owner.id, "", info.peerUri);
    }

    emit linked.newAccountTransfer(fileId, info);
}

void
ContactModelPimpl::slotProfileReceived(const QString& accountId,
                                       const QString& peer,
                                       const QString& path)
{
    if (accountId != linked.owner.id)
        return;

    QFile vCardFile(path);
    if (!vCardFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&vCardFile);

    auto vCard = in.readAll();

    profile::Info profileInfo;
    profileInfo.uri = peer;
    profileInfo.type = profile::Type::JAMI;

    for (auto& e : QString(vCard).split("\n"))
        if (e.contains("PHOTO"))
            profileInfo.avatar = e.split(":")[1];
        else if (e.contains("FN"))
            profileInfo.alias = e.split(":")[1];

    if (peer == linked.owner.profileInfo.uri) {
        if (linked.owner.profileInfo.avatar.isEmpty() && !profileInfo.avatar.isEmpty()) {
            auto dest = storage::getPath() + accountId + "/profile.vcf";
            QFile oldvCard(dest);
            if (oldvCard.exists())
                oldvCard.remove();
            vCardFile.rename(dest);
            linked.owner.accountModel->setAlias(linked.owner.id, profileInfo.alias);
            linked.owner.accountModel->setAvatar(linked.owner.id, profileInfo.avatar);
            emit linked.profileUpdated(peer);
        }
        return;
    }
    vCardFile.remove();

    contact::Info contactInfo;
    contactInfo.profileInfo = profileInfo;

    linked.owner.contactModel->addContact(contactInfo);
    if (!lrc::api::Lrc::cacheAvatars.load())
        contactInfo.profileInfo.avatar.clear(); // Do not store after update
}

void
ContactModelPimpl::slotUserSearchEnded(const QString& accountId,
                                       int status,
                                       const QString& query,
                                       const VectorMapStringString& result)
{
    if (searchQuery != query)
        return;
    if (accountId != linked.owner.id)
        return;
    searchResult.clear();
    switch (status) {
    case 0: /* SUCCESS */
        for (auto& resultInfo : result) {
            if (contacts.find(resultInfo.value("id")) != contacts.end()) {
                continue;
            }
            profile::Info profileInfo;
            profileInfo.uri = resultInfo.value("id");
            profileInfo.type = profile::Type::TEMPORARY;
            profileInfo.avatar = resultInfo.value("profilePicture");
            profileInfo.alias = resultInfo.value("firstName") + " " + resultInfo.value("lastName");
            contact::Info contactInfo;
            contactInfo.profileInfo = profileInfo;
            contactInfo.registeredName = resultInfo.value("username");
            searchResult.insert(profileInfo.uri, contactInfo);
        }
        updateTemporaryMessage("");
        break;
    case 3: /* ERROR */
        updateTemporaryMessage("could not find contact matching search");
        break;
    default:
        break;
    }
    emit linked.modelUpdated(query);
}

} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

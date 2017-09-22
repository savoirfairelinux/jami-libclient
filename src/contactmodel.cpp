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
#include <algorithm>

// Daemon
#include <account_const.h>

// LRC
#include "api/account.h"
#include "api/contact.h"
#include "api/message.h"
#include "api/newcallmodel.h"
#include "callbackshandler.h"
#include "database.h"

#include "accountmodel.h"
#include "contactmethod.h"
#include "namedirectory.h"
#include "phonedirectorymodel.h"
#include "private/vcardutils.h"

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
     * @note: the cm must corresponds to a profile in the database.
     * @param cm ContactMethod.
     * @return contact::Info added to contacts
     * @todo this function rely on legacy code and has to be changed in the futur.
     */
    void addToContacts(ContactMethod* cm);

    // Helpers
    const ContactModel& linked;
    Database& db;
    const CallbacksHandler& callbacksHandler;

    // Containers
    ContactModel::ContactInfoMap contacts;
    //contact::Info temporaryContact;

    // Searching avatar in base 64
    const std::string searchingAvatar = "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";

    // Database Helpers
    // TODO: move in another class.
    /**
     * @note the account must be in the database or it will fails
     * @return the id in the database for the current profile
     */
    std::string getAccountProfileId() const;
    /**
     * @param contactUri
     * @param alias
     * @param avatar
     * @return the id in the database for the current profile
     */
    std::string getOrInsertContact(const std::string& contactUri,
                                   const std::string& alias = "",
                                   const std::string& avatar = "") const;
    /**
     * @param accountProfile the id of the account in the database
     * @param contactProfile the id of the contact in the database
     * @return conversations id for conversations between account and contact
     */
    std::vector<std::string> getConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const;
    /**
     * Start a conversation between account and contact. Creates an entry in the conversations table
     * and an entry in the interactions table.
     * @param accountProfile the id of the account in the database
     * @param contactProfile the id of the contact in the database
     * @return conversation_id of the new conversation.
     */
    std::string beginConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const;
    /**
     * Remove a conversation between current account and a contact. Remove corresponding entries in
     * the conversations table and profiles if the profile is linked to no conversations.
     * @param contactUri
     */
    void removeContactFromDb(const std::string& contactUri) const;

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
};

namespace authority
{
namespace daemon
{
    void addContact(const account::Info& owner, const std::string& uri);
    void addToContacts(const account::Info& owner, contact::Info& contactInfo);
    void removeContact(const account::Info& owner, const std::string& uri, bool banned);
    void addContactFromPending(const account::Info& owner, const std::string& uri);
    void discardFromPending(const account::Info& owner, const std::string& uri);
} // namespace daemon

namespace database
{
    void addContact(ContactModelPimpl& model, const std::string& contactUri);
    const std::string addToProfiles(Database& db, const contact::Info& contactInfo);
} // namespace database
} // namespace authority

void
authority::daemon::addContact(const account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()), QString(contactUri.c_str()));
}

void
authority::daemon::addToContacts(const account::Info& owner, contact::Info& contactInfo)
{
    ConfigurationManager::instance().addContact(QString(owner.id.c_str()), QString(contactInfo.uri.c_str()));
    // [jn] voir si cette fonction retourne un status
    contactInfo.type = contact::Type::RING;
}

void
authority::daemon::removeContact(const account::Info& owner, const std::string& contactUri, bool banned)
{
    ConfigurationManager::instance().removeContact(QString(owner.id.c_str()),
    QString(contactUri.c_str()), banned);
}

void
authority::daemon::addContactFromPending(const account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().acceptTrustRequest(QString(owner.id.c_str()), QString(contactUri.c_str()) );
}

void
authority::daemon::discardFromPending(const account::Info& owner, const std::string& contactUri)
{
    ConfigurationManager::instance().discardTrustRequest(
        QString(owner.id.c_str()),
        QString(contactUri.c_str())
    );
}

const std::string
authority::database::addToProfiles(Database& db, const contact::Info& contactInfo)
{
    // Check if profile is already present.
    auto profile = db.select("id", "profiles", "uri=:uri", {{":uri", contactInfo.uri}});

    if (profile.payloads.empty()) {
        // Doesn't exists, add contact to the database
        auto row = db.insertInto("profiles",
                                 {{":uri", "uri"}, {":alias", "alias"},
                                  {":photo", "photo"}, {":type", "type"},
                                 {":status", "status"}},
                                 {{":uri", contactInfo.uri}, {":alias", contactInfo.alias},
                                  {":photo", contactInfo.avatar}, {":type", TypeToString(contactInfo.type)},
                                 {":status", "TRUSTED"}});

        if (row == -1) {
            qDebug() << "contact not added to the database";
            return "";
        }
        return std::to_string(row);

    } else {
        // Exists, update and retrieve it.
        if (!contactInfo.avatar.empty() || !contactInfo.alias.empty()) {
            db.update("profiles",
                      "alias=:alias, photo=:photo",
                      {{":alias", contactInfo.alias}, {":photo", contactInfo.avatar}},
                      "uri=:uri",
                      {{":uri", contactInfo.uri}});
        }
    return profile.payloads[0];
    }
}

void
authority::database::addContact(ContactModelPimpl& model, const std::string& contactUri)
{
    // Get profile for contact
    auto row = model.getOrInsertContact(contactUri);
    if (row.empty()) {
        qDebug() << "database::addContact, no profile for contact. abort";
        return;
    }
    // Get profile of the account linked
    auto accountProfileId = model.getAccountProfileId();
    // Get if conversation exists
    auto common = model.getConversationsBetween(accountProfileId, row);
    if (common.empty()) {
        // conversations doesn't exists, start it.
        model.beginConversationsBetween(accountProfileId, row);
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

bool
ContactModel::hasPendingRequests() const
{
    auto i = std::find_if(pimpl_->contacts.begin(), pimpl_->contacts.end(),
    [](const auto& contact) {
      return contact.second.type == contact::Type::PENDING;
    });

    return (i != pimpl_->contacts.end());
}

void
ContactModel::addContact(contact::Info contactInfo)
{
    if ((owner.profile.type == contact::Type::SIP and contactInfo.type == contact::Type::RING)
        or (owner.profile.type == contact::Type::RING and contactInfo.type == contact::Type::SIP)) {
            qDebug() << "ContactModel::addContact, types invalids.";
            return; // [jn] throw
        }

    switch (contactInfo.type) {
    case contact::Type::RING: // si c'est un type ring, il doit déjà être dans le daemon.
    case contact::Type::PENDING:
        daemon::addToContacts(owner, contactInfo); // cette fonction modifera le type de pending a RING, si ring ne pas l'ajouter
        // we need to add the profile too
    case contact::Type::SIP:
        database::addToProfiles(pimpl_->db, contactInfo);
        break;
    break;
    case contact::Type::TEMPORARY:
        qDebug() << "TODO FOR TEMPORARY";
    case contact::Type::INVALID:
        qDebug() << "ContactModel::addContact, cannot add contact with invalid type.";
    break;
    }

    pimpl_->contacts.emplace(contactInfo.uri, contactInfo);
    emit modelUpdated();
}

//~ void // to remove
//~ ContactModel::addContact(const std::string& contactUri)
//~ {
    //~ auto contactInfo = contact::Info({contactUri, "", contactUri, contactUri,
                                     //~ true, false, contact::Type::INVALID});

    //~ if(owner.profile.type == contact::Type::SIP) {
        //~ // SIP contact are owned by the database
        //~ database::addContact(*this->pimpl_, contactUri);
        //~ contactInfo.type = contact::Type::SIP;
        //~ // Here the contact is directly added, so we can add the contact in the map
        //~ pimpl_->contacts.emplace(contactUri, contactInfo);
        //~ emit modelUpdated();
    //~ } else {
        //~ // Ring contacts are owned by the daemon
        //~ auto contactFound = pimpl_->contacts.find(contactUri);
        //~ if(contactFound != pimpl_->contacts.end()
        //~ && contactFound->second.type == contact::Type::PENDING) {
            //~ // If it's a pending request, accept the request
            //~ daemon::addContactFromPending(owner, contactUri);
            //~ contactFound->second.type = contact::Type::RING;
            //~ pimpl_->getOrInsertContact(contactUri, contactInfo.alias, contactInfo.avatar);
            //~ emit modelUpdated();
        //~ } else {
            //~ // if the contact is already found, we don't need to add it.
            //~ if(contactFound != pimpl_->contacts.end()) {
                //~ qDebug() << "Add contact, but not pending, abort.";
                //~ return;
            //~ }
            //~ // HACK: The daemon can say this URI is a contact (but don't return it with getAllContacts)
            //~ // So, test if the daemon considers this uri as a contact
            //~ QMap<QString, QString> details = ConfigurationManager::instance()
            //~ .getContactDetails(QString(owner.id.c_str()), QString(contactUri.c_str()));
            //~ if (details.empty()) {
                //~ // It's not a contact for the daemon, add it.
                //~ // NOTE: this method is async, slotContactAdded will be call and then
                //~ // the model will be updated.
                //~ daemon::addContact(owner, contactUri);
                //~ contactInfo.type = contact::Type::RING;
            //~ } else {
                //~ // It's a contact for the daemon, directly add it.
                //~ // TODO: investigate and remove this hack.
                //~ auto account = AccountModel::instance().getById(owner.id.c_str());
                //~ // Get details.
                //~ auto cm = PhoneDirectoryModel::instance().getNumber(contactUri.c_str(), account);
                //~ auto contactUri = cm->uri().toStdString();
                //~ contactInfo.uri = contactUri;
                //~ contactInfo.registeredName = cm->registeredName().toStdString();
                //~ contactInfo.alias = cm->bestName().toStdString();
                //~ contactInfo.isTrusted = cm->isConfirmed();
                //~ contactInfo.isPresent = cm->isPresent();
                //~ contactInfo.type = contact::Type::RING;

                //~ pimpl_->contacts.emplace(contactUri, contactInfo);
                //~ pimpl_->getOrInsertContact(contactUri, contactInfo.alias, contactInfo.avatar);
                //~ emit modelUpdated();
            //~ }
        //~ }
    //~ }
//~ }

void
ContactModel::removeContact(const std::string& contactUri, bool banned)
{
    auto contact = pimpl_->contacts.find(contactUri);
    if (!banned && contact != pimpl_->contacts.end()
        && contact->second.type == contact::Type::PENDING) {
        // Discard the pending request and remove profile from db if necessary
        daemon::discardFromPending(owner, contactUri);
        pimpl_->contacts.erase(contactUri);
        pimpl_->removeContactFromDb(contactUri);
        emit modelUpdated();
    } else {
        if (owner.profile.type == contact::Type::SIP) {
            // Remove contact from db
            pimpl_->contacts.erase(contactUri);
            pimpl_->removeContactFromDb(contactUri);
            emit modelUpdated();
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
    // NOTE: "" is the id of the temporaryContact
    //~ if (contactUri.empty())
        //~ return pimpl_->temporaryContact;

    auto contactInfo = pimpl_->contacts.find(contactUri);
    if (contactInfo == pimpl_->contacts.end())
        throw std::out_of_range("ContactModel::getContact, can't find " + contactUri);

    return contactInfo->second;
}

void
ContactModel::searchContact(const std::string& query)
{
    if (owner.profile.type == contact::Type::SIP) {
        // We don,t need to search anything for SIP contacts.
        pimpl_->contacts[""].uri = query;
        // TODO default SIP avatar?
        pimpl_->contacts[""].avatar = "";
        pimpl_->contacts[""].registeredName = query; // [jn] there is no registered name for sip
        pimpl_->contacts[""].alias = query;
        emit modelUpdated();
        return;
    }
    // Default searching item.
    pimpl_->contacts[""].uri = "";
    pimpl_->contacts[""].registeredName = query;
    pimpl_->contacts[""].alias = "Searching… " + query;
    pimpl_->contacts[""].avatar = pimpl_->searchingAvatar;
    emit modelUpdated();

    // Query Name Server
    auto uri = URI(QString(query.c_str()));
    auto account = AccountModel::instance().getById(owner.id.c_str());
    if (!account) return;
    account->lookupName(QString(query.c_str()));
}


ContactModelPimpl::ContactModelPimpl(const ContactModel& linked,
                                     Database& db,
                                     const CallbacksHandler& callbacksHandler)
: linked(linked)
, db(db)
, callbacksHandler(callbacksHandler)
{
    // Init contacts map
    if (linked.owner.profile.type == contact::Type::SIP)
        fillsWithSIPContacts();
    else
        fillsWithRINGContacts();

    // connect the signals
    connect(&callbacksHandler, &CallbacksHandler::NewBuddySubscription,
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

}

ContactModelPimpl::~ContactModelPimpl()
{
}

bool
ContactModelPimpl::fillsWithSIPContacts()
{
    auto accountProfileId = getAccountProfileId();
    // get conversations with this profile
    auto conversationsForAccount = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", accountProfileId}}).payloads;
    for (const auto& c : conversationsForAccount) {
        auto otherParticipants = db.select("participant_id",
                                          "conversations",
                                          "id=:id AND participant_id!=:participant_id",
                                          {{":id", c}, {":participant_id", accountProfileId}}).payloads;
        for (const auto& participant: otherParticipants) {
            // for each conversations get the other profile id
            auto returnFromDb = db.select("uri, alias, photo",
            "profiles",
            "id=:id",
            {{":id", participant}});
            if (returnFromDb.nbrOfCols == 3) {
                auto payloads = returnFromDb.payloads;
                for (auto i = 0; i < payloads.size(); i += 3) {
                    auto contact = contact::Info();
                    contact.uri = payloads[i];
                    contact.alias = payloads[i + 1];
                    contact.avatar = payloads[i + 2];
                    contact.isTrusted = true;
                    contact.isPresent = false;
                    contact.type = contact::Type::SIP;
                    contacts.emplace(payloads[i], contact);
                }
            }
        }
    }
    return true;
}

bool
ContactModelPimpl::fillsWithRINGContacts() {
    // NOTE: In the future, we will directly get contacts from daemon (or SIP)
    // and avoid "account->getContacts();"
    auto account = AccountModel::instance().getById(linked.owner.id.c_str());
    if (not account) {
        qDebug() << "ContactModel::fillsWithContacts(), nullptr";
        return false;
    }

    // Add daemon contacts
    auto contactsList = account->getContacts();
    for (auto c : contactsList)
        addToContacts(c);

    // Add pending contacts
    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(account->id())};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();
        auto ringId = tr_info[DRing::Account::TrustRequest::FROM];

        auto cm = PhoneDirectoryModel::instance().getNumber(ringId, account);
        const auto vCard = VCardUtils::toHashMap(payload);
        const auto alias = vCard["FN"];
        const auto photo = vCard["PHOTO;ENCODING=BASE64;TYPE=PNG"];

        auto contactUri = cm->uri().toStdString();
        auto contactInfo = contact::Info({contactUri, photo.toStdString(),
                                          cm->registeredName().toStdString(),
                                          alias.toStdString(),
                                          cm->isConfirmed(), cm->isPresent(),
                                          contact::Type::PENDING});
        contacts.emplace(contactUri, contactInfo);
        getOrInsertContact(contactUri, alias.toStdString(), photo.toStdString());
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
        // Doesn't exists, add contact to the database
        auto cm = PhoneDirectoryModel::instance().getNumber(QString(contactUri.c_str()), account);
        auto row = getOrInsertContact(contactUri, cm->bestName().toStdString(), "");
        // Get profile of the account linked
        auto accountProfileId = getAccountProfileId();
        // Get if conversation exists
        auto common = getConversationsBetween(accountProfileId, row);
        if (common.empty()) {
            // conversations doesn't exists, start it.
            beginConversationsBetween(accountProfileId, row);
        }
        addToContacts(cm);
        emit linked.modelUpdated();
    }
}

void
ContactModelPimpl::slotContactRemoved(const std::string& accountId, const std::string& contactUri, bool banned)
{
    if (accountId != linked.owner.id) return;
    Q_UNUSED(banned)
    if (contacts.find(contactUri) != contacts.end()) {
        removeContactFromDb(contactUri);
        contacts.erase(contactUri);
        emit linked.modelUpdated();
    }
}

// TODO: test !!
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
    auto row = getOrInsertContact(contactUri);
    auto accountProfileId = getAccountProfileId();
    auto conversations = getConversationsBetween(accountProfileId, row);
    if (conversations.empty()) {
        conversations.emplace_back(beginConversationsBetween(accountProfileId, row));
    }
    db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountProfileId}, {":author_id", accountProfileId},
                  {":conversation_id", conversations[0]}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", std::to_string(msg.timestamp)},
                  {":body", msg.body}, {":type", TypeToString(msg.type)},
                  {":status", StatusToString(msg.status)}});
}

void
ContactModelPimpl::addToContacts(ContactMethod* cm)
{
    auto contactUri = cm->uri().toStdString();
    auto returnFromDb = db.select("photo, uri, alias, type",
                                  "profiles",
                                  "uri=:uri",
                                  {{":uri", contactUri}});

    // the query should return on row of three columns.
    if (returnFromDb.nbrOfCols == 4 and returnFromDb.payloads.size() == 4) {
        auto avatar = returnFromDb.payloads[0];
        auto registeredName = returnFromDb.payloads[1];
        auto alias = returnFromDb.payloads[2];
        auto isPresent = cm->isPresent();
        auto type = contact::StringToType(returnFromDb.payloads[3]);

        auto contactInfo = contact::Info({contactUri, avatar, registeredName,
                                         alias, isPresent, true, type});
        contacts.emplace(contactUri, contactInfo);
    }
}

void
ContactModelPimpl::slotRegisteredNameFound(const std::string& accountId, const std::string& uri, const std::string& registeredName)
{
    if (accountId != linked.owner.id) return;
    if (contacts.find(uri) == contacts.end()) {
        contacts[""].uri = uri;
        contacts[""].registeredName = registeredName;
        contacts[""].alias = registeredName;
        contacts[""].avatar = "";
    } else {
        contacts[uri].registeredName = registeredName;
        contacts[uri].alias = registeredName;
        //~ if (temporaryContact.registeredName == uri
        //~ || temporaryContact.registeredName == registeredName) {
            //~ temporaryContact.uri = "";
            //~ temporaryContact.registeredName = "";
            //~ temporaryContact.alias = "";
            //~ temporaryContact.avatar = searchingAvatar;
        //~ }
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

        auto contactInfo = contact::Info({contactUri, photo.toStdString(),
                                          cm->registeredName().toStdString(),
                                          alias.toStdString(),
                                          cm->isConfirmed(), cm->isPresent(),
                                          contact::Type::PENDING});
        contacts.emplace(contactUri, contactInfo);
        getOrInsertContact(contactUri, alias.toStdString(), photo.toStdString());
        emit linked.modelUpdated();
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
        addToContacts(cm);
        emit linked.modelUpdated();
    } else {
        // Already a contact, just emit signal
        // TODO verify if usefull, callModel already emit a signal for this.
        emit linked.incomingCallFromPending(fromId, callId);
    }
}

// Database Helpers
std::string
ContactModelPimpl::getAccountProfileId() const
{
    return db.select("id", "profiles","uri=:uri", {{":uri", linked.owner.profile.uri}}).payloads[0];
}

std::string
ContactModelPimpl::getOrInsertContact(const std::string& contactUri,
                                      const std::string& alias,
                                      const std::string& avatar) const
{
    // Check if profile is already present.
    auto profileAlreadyExists = db.select("id",
                                                "profiles",
                                                "uri=:uri",
                                                {{":uri", contactUri}});
    if (profileAlreadyExists.payloads.empty()) {
        // Doesn't exists, add contact to the database
        auto type = TypeToString(linked.owner.profile.type);
        auto row = db.insertInto("profiles",
        {{":uri", "uri"}, {":alias", "alias"}, {":photo", "photo"}, {":type", "type"},
        {":status", "status"}},
        {{":uri", contactUri}, {":alias", alias}, {":photo", avatar}, {":type", type},
        {":status", "TRUSTED"}});

        if (row == -1) {
            qDebug() << "contact not added to the database";
            return "";
        }

        return std::to_string(row);
    } else {
        // Exists, update and retrieve it.
        if (!avatar.empty() || !alias.empty()) {
            db.update("profiles",
                      "alias=:alias, photo=:photo",
                      {{":alias", alias}, {":photo", avatar}},
                      "uri=:uri", {{":uri", contactUri}});
        }
        return profileAlreadyExists.payloads[0];
    }
}


std::vector<std::string>
ContactModelPimpl::getConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const
{
    auto conversationsForAccount = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", accountProfile}}).payloads;
    std::sort(conversationsForAccount.begin(), conversationsForAccount.end());
    auto conversationsForContact = db.select("id",
                                  "conversations",
                                  "participant_id=:participant_id",
                                  {{":participant_id", contactProfile}}).payloads;

    std::sort(conversationsForContact.begin(), conversationsForContact.end());
    std::vector<std::string> common;

    std::set_intersection(conversationsForAccount.begin(), conversationsForAccount.end(),
                          conversationsForContact.begin(), conversationsForContact.end(),
                          std::back_inserter(common));
    return common;
}

std::string
ContactModelPimpl::beginConversationsBetween(const std::string& accountProfile, const std::string& contactProfile) const
{
    // Add conversation between account and profile
    auto newConversationsId = db.select("IFNULL(MAX(id), 0) + 1",
                                        "conversations",
                                        "1=1",
                                        {}).payloads[0];
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", accountProfile}});
    db.insertInto("conversations",
                  {{":id", "id"}, {":participant_id", "participant_id"}},
                  {{":id", newConversationsId}, {":participant_id", contactProfile}});
    // Add "Conversation started" message
    db.insertInto("interactions",
                  {{":account_id", "account_id"}, {":author_id", "author_id"},
                  {":conversation_id", "conversation_id"}, {":device_id", "device_id"},
                  {":group_id", "group_id"}, {":timestamp", "timestamp"},
                  {":body", "body"}, {":type", "type"},
                  {":status", "status"}},
                  {{":account_id", accountProfile}, {":author_id", accountProfile},
                  {":conversation_id", newConversationsId}, {":device_id", "0"},
                  {":group_id", "0"}, {":timestamp", "0"},
                  {":body", "Conversation started"}, {":type", "CONTACT"},
                  {":status", "SUCCEED"}});
    return newConversationsId;
}

void
ContactModelPimpl::removeContactFromDb(const std::string& contactUri) const
{
    // Get profile for contact
    auto contactId = db.select("id", "profiles","uri=:uri", {{":uri", contactUri}}).payloads;
    if (contactId.empty()) return; // No profile
    // Get common conversations
    auto accountProfileId = getAccountProfileId();
    auto conversations = getConversationsBetween(accountProfileId, contactId[0]);
    // Remove conversations + interactions
    for (const auto& conversationId: conversations) {
        // Remove conversation
        db.deleteFrom("conversations", "id=:id", {{":id", conversationId}});
        // clear History
        db.deleteFrom("interactions", "conversation_id=:id", {{":id", conversationId}});
    }
    // Get conversations for this contact.
    conversations = db.select("id", "conversations","participant_id=:id", {{":id", contactId[0]}}).payloads;
    if (conversations.empty()) {
        // Delete profile
        db.deleteFrom("profiles", "id=:id", {{":id", contactId[0]}});
    }
}


} // namespace lrc

#include "api/moc_contactmodel.cpp"
#include "contactmodel.moc"

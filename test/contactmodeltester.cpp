/*
 *  Copyright (C) 2017-2019 Savoir-faire Linux Inc.
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */
#include "contactmodeltester.h"
#include "utils/waitforsignalhelper.h"

// std
#include <algorithm>

// Qt
#include <QString>
#include <QMetaType>

// Lrc
#include <api/newaccountmodel.h>
#include <api/contact.h>
#include <api/contactmodel.h>
#include <api/conversationmodel.h>
#include <dbus/configurationmanager.h>
#include <dbus/presencemanager.h>

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(ContactModelTester);

ContactModelTester::ContactModelTester()
: lrc_(new lrc::api::Lrc())
{
}

void
ContactModelTester::setUp()
{
    daemon_ = std::make_unique<Daemon>();
    daemon_->addAccount("Fred");
    daemon_->addAccount("Ada");
}

void
ContactModelTester::testBanUnbanContact()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);

    CPPUNIT_ASSERT_THROW(accInfoAda.contactModel->getContact(accInfo.profileInfo.uri), std::out_of_range);
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->searchContact(accInfo.profileInfo.uri);
        })
        .addSignal("modelUpdated", *accInfoAda.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"], 1);

    auto temporaryContact = accInfoAda.contactModel->getContact("");
    std::string uri = std::string(accInfo.profileInfo.uri);
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, uri);

    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfoAda.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"], 1);

    // Ban contact
    auto banContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->removeContact(uri, true);
        })
        .addSignal("filterChanged", *accInfoAda.conversationModel, SIGNAL(filterChanged()))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(banContactSigsCaught["filterChanged"], 1);

    auto contactInfo = accInfoAda.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // Re-ban contact, make sure it isn't a problem
    auto reBanContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->removeContact(uri, true);
        })
        .addSignal("filterChanged", *accInfoAda.conversationModel, SIGNAL(filterChanged()))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(reBanContactSigsCaught["filterChanged"], 0);

    contactInfo = accInfoAda.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // Unban contact, make sure it worked
    auto unbanContactSigsCaught = WaitForSignalHelper([&]() {
            contactInfo = accInfoAda.contactModel->getContact(uri);
            accInfoAda.contactModel->addContact(contactInfo);
        })
        .addSignal("filterChanged", *accInfoAda.conversationModel, SIGNAL(filterChanged()))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(unbanContactSigsCaught["filterChanged"], 1);

    contactInfo = accInfoAda.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, false);
}

void
ContactModelTester::receivesPendingRequest()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->hasPendingRequests(), false);

    auto accountIdAda = daemon_->getAccountId("Ada");
    QByteArray vCardAda = lrc_->getAccountModel().accountVCard(accountIdAda).c_str();

    // TODO some failures
    auto incomingTrustRequestSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().sendTrustRequest(accountIdAda.c_str(), accInfo.profileInfo.uri.c_str(), vCardAda);
        })
        .addSignal("contactAdded", *accInfo.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(10000);
    CPPUNIT_ASSERT_EQUAL(incomingTrustRequestSigsCaught["contactAdded"], 1);
    CPPUNIT_ASSERT(accInfo.contactModel->hasPendingRequests());

    QVector<QMap<QString, QString>> contactsFromDaemon = ConfigurationManager::instance().getContacts(accountId.c_str());
    auto contacts = accInfo.contactModel->getAllContacts();
    int lrcContactsNumber = contacts.size();
    int daemonContactsNumber = contactsFromDaemon.size();
    CPPUNIT_ASSERT_EQUAL(lrcContactsNumber, daemonContactsNumber + 1 /* Temporary */);

    // Test getContacts
    for (const auto& contactUri: contactsFromDaemon)
        CPPUNIT_ASSERT(contacts.find(contactUri["id"].toStdString()) != contacts.end());
}

void
ContactModelTester::testAddNewContact()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);

    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    QByteArray vCardAda = lrc_->getAccountModel().accountVCard(accountIdAda).c_str();

    // Fred is not in Ada contacts
    CPPUNIT_ASSERT_THROW(accInfoAda.contactModel->getContact(accInfo.profileInfo.uri), std::out_of_range);

    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->searchContact(accInfo.profileInfo.uri);
        })
        .addSignal("modelUpdated", *accInfoAda.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"], 1);
    auto temporaryContact = accInfoAda.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(accInfo.profileInfo.uri, temporaryContact.profileInfo.uri);

    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoAda.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfoAda.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(10000);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"], 1);
    // Fred is now a contact for Ada
    CPPUNIT_ASSERT_NO_THROW(accInfoAda.contactModel->getContact(accInfo.profileInfo.uri));
}

void
ContactModelTester::testAddRingURI()
{
    auto accountId = daemon_->getAccountId("Ada");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);

    std::string uri = "deadc0debabe133700000000deadc0debabe1337";

    // uri is not in Ada contacts
    CPPUNIT_ASSERT_THROW(accInfo.contactModel->getContact(uri), std::out_of_range);

    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo.contactModel->searchContact(uri);
        })
        .addSignal("modelUpdated", *accInfo.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"], 1);
    auto temporaryContact = accInfo.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(uri, temporaryContact.profileInfo.uri);

    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(10000);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"], 1);
    // uri is now a contact for Ada
    CPPUNIT_ASSERT_NO_THROW(accInfo.contactModel->getContact(uri));
}

void
ContactModelTester::testAddNewSIPContact()
{
    // TODO: fix me
    // mock is broken for SIP account (contacts are stored in the database, not in the daemon)

    /*auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // "sipcontact0" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfoSip.contactModel->getContact("sipcontact0"), std::out_of_range);
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->searchContact("sipcontact0");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(1000);
    //CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact0"));
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(1000);
    //CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);
    // "sipcontact0" should be "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact0"));*/
}

void
ContactModelTester::testAddAlreadyAddedContact()
{
    testAddNewContact();

    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);

    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    QByteArray vCardAda = lrc_->getAccountModel().accountVCard(accountIdAda).c_str();

    auto nbContactsAtBegin = accInfoAda.contactModel->getAllContacts().size();
    // "Fred" should be in "Ada" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoAda.contactModel->getContact(accInfo.profileInfo.uri));
    auto contact1 = accInfoAda.contactModel->getContact(accInfo.profileInfo.uri);
    accInfoAda.contactModel->addContact(contact1);

    // "Fred" should be in "Ada" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoAda.contactModel->getContact(accInfo.profileInfo.uri));
    auto nbContactsAtEnd = accInfoAda.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtBegin, nbContactsAtEnd);
}

void
ContactModelTester::testReceivesContactPresenceUpdate()
{
    testAddNewContact();
    auto accountId = daemon_->getAccountId("Ada");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdFred = daemon_->getAccountId("Fred");
    const auto& accInfoFred = lrc_->getAccountModel().getAccountInfo(accountIdFred);

    // "Ada" should be in "Fred" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo.contactModel->getContact(accInfoFred.profileInfo.uri));
    auto idx = 0;
    while (!accInfo.contactModel->getContact(accInfoFred.profileInfo.uri).isPresent && idx < 10) {
        auto presenceSigsCaught = WaitForSignalHelper([&]() {
        })
        .addSignal("conversationUpdated", *accInfo.conversationModel, SIGNAL(conversationUpdated(const std::string&)))
        .wait(10000);
    }
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->getContact(accInfoFred.profileInfo.uri).isPresent, true);

    // TODO: For now there is no way to remove the presence on the DHT, so don't test
    // If presence is removed.
}

void
ContactModelTester::testRmContact()
{
    testAddNewContact();
    auto accountId = daemon_->getAccountId("Ada");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdFred = daemon_->getAccountId("Fred");
    const auto& accInfoFred = lrc_->getAccountModel().getAccountInfo(accountIdFred);

    // "Fred" should be in "Ada" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo.contactModel->getContact(accInfoFred.profileInfo.uri));

    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo.contactModel->removeContact(accInfoFred.profileInfo.uri);
        })
        .addSignal("contactRemoved", *accInfo.contactModel, SIGNAL(contactRemoved(const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"], 1);
    int nbContactsAtEnd = accInfo.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo.contactModel->getContact(accInfoFred.profileInfo.uri), std::out_of_range);
}

void
ContactModelTester::testRmPendingContact()
{
    receivesPendingRequest();
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);

    CPPUNIT_ASSERT_NO_THROW(accInfo.contactModel->getContact(accInfoAda.profileInfo.uri));
    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo.contactModel->removeContact(accInfoAda.profileInfo.uri);
        })
        .addSignal("contactRemoved", *accInfo.contactModel, SIGNAL(contactRemoved(const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"], 1);
    int nbContactsAtEnd = accInfo.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo.contactModel->getContact(accInfoAda.profileInfo.uri), std::out_of_range);
}

void
ContactModelTester::testRmSIPContact()
{
    // TODO: fix me
    // mock is broken for SIP account (contacts are stored in the database, not in the daemon)

    /*auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->searchContact("sipcontact1");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact1"));
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);
    // "sipcontact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact1"));
    int nbContactsAtBegin = accInfoSip.contactModel->getAllContacts().size();
    // "sipcontact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact1"));
    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->removeContact("sipcontact1");
        })
        .addSignal("contactRemoved", *accInfo_.contactModel, SIGNAL(contactRemoved(const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"] > 0, true);
    int nbContactsAtEnd = accInfoSip.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    // "sipcontact1" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfoSip.contactModel->getContact("sipcontact1"), std::out_of_range);*/
}

void
ContactModelTester::testRmTemporaryContact()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    accInfo.contactModel->removeContact("");
    int nbContactsAtEnd = accInfo.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin);
}

void
ContactModelTester::testCountPendingRequests()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);

    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->pendingRequestCount(), 0);
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->hasPendingRequests(), false);

    receivesPendingRequest();

    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->pendingRequestCount(), 1);
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->hasPendingRequests(), true);
}

void
ContactModelTester::testCountPendingRequestsWithBlockedContact()
{

    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    int nbContactsAtBegin = accInfo.contactModel->getAllContacts().size();
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);

    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->pendingRequestCount(), 0);
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->hasPendingRequests(), false);

    receivesPendingRequest();
    CPPUNIT_ASSERT_NO_THROW(accInfo.contactModel->getContact(accInfoAda.profileInfo.uri));
    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo.contactModel->removeContact(accInfoAda.profileInfo.uri, true);
        })
        .addSignal("bannedStatusChanged", *accInfo.contactModel, SIGNAL(bannedStatusChanged(const std::string&, bool)))
        .wait(1000);

    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->pendingRequestCount(), 0);
    CPPUNIT_ASSERT_EQUAL(accInfo.contactModel->hasPendingRequests(), false);
}

void
ContactModelTester::tearDown()
{
    daemon_.reset();
}

} // namespace test
} // namespace ring

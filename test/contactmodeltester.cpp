/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
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

// std
#include <algorithm>

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"

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
, accInfo_(lrc_->getAccountModel().getAccountInfo("ring1"))
{

}

void
ContactModelTester::setUp()
{
}

void
ContactModelTester::testBanUnbanContact()
{
    // "bigbadjohn" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("bigbadjohn"), std::out_of_range);

    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->searchContact("bigbadjohn");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfo_.contactModel->getContact("");
    std::string uri = std::string("bigbadjohn");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, uri);

    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);

    // Ban contact
    auto banContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->removeContact(uri, true);
        })
        .addSignal("filterChanged", ConfigurationManager::instance(), SIGNAL(lrc::api::ConversationModel::filterChanged()))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(banContactSigsCaught["filterChanged"] > 0, true);

    auto contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // Re-ban contact, make sure it isn't a problem
    auto reBanContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->removeContact(uri, true);
        })
        .addSignal("filterChanged", ConfigurationManager::instance(), SIGNAL(lrc::api::ConversationModel::filterChanged()))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(reBanContactSigsCaught["filterChanged"] > 0, true);

    contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // Unban contact, make sure it worked
    auto unbanContactSigsCaught = WaitForSignalHelper([&]() {
            contactInfo = accInfo_.contactModel->getContact(uri);
            accInfo_.contactModel->addContact(contactInfo);
        })
        .addSignal("filterChanged", ConfigurationManager::instance(), SIGNAL(lrc::api::ConversationModel::filterChanged()))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(unbanContactSigsCaught["filterChanged"] > 0, true);

    contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, false);
}

void
ContactModelTester::testGetAllContactsForRINGAccount()
{
    auto contacts = accInfo_.contactModel->getAllContacts();
    auto contactsFromDaemon = ConfigurationManager::instance().getContacts("ring1");
    // getAllContacts must return all daemon contacts
    int lrcContactsNumber = contacts.size();
    int daemonContactsNumber = contactsFromDaemon.size();
    CPPUNIT_ASSERT_EQUAL(lrcContactsNumber, daemonContactsNumber);
    for (const auto& contactUri: contactsFromDaemon)
        CPPUNIT_ASSERT(contacts.find(contactUri["id"].toStdString()) != contacts.end());
}

void
ContactModelTester::testReceivesPendingRequest()
{
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->hasPendingRequests(), false);
    QByteArray payload = "FN:pending0\nPHOTO;ENCODING=BASE64;TYPE=PNG:";
    auto incomingTrustRequestSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitIncomingTrustRequest("ring1", "pending0", payload, 0);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(incomingTrustRequestSigsCaught["contactAdded"] > 0, true);
    CPPUNIT_ASSERT(accInfo_.contactModel->hasPendingRequests());
    auto contactsFromDaemon = ConfigurationManager::instance().getContacts("ring1");
    auto contacts = accInfo_.contactModel->getAllContacts();
    int lrcContactsNumber = contacts.size();
    int daemonContactsNumber = contactsFromDaemon.size();
    CPPUNIT_ASSERT_EQUAL(lrcContactsNumber, daemonContactsNumber + 1);
}

void
ContactModelTester::testAddNewRingContact()
{
    // "dummy" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("dummy"), std::out_of_range);
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->searchContact("dummy");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfo_.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(std::string("dummy"), temporaryContact.profileInfo.uri);
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("dummy"));
}

void
ContactModelTester::testAddRingURI()
{
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("f5a46751671918fe7210a3c31b9a9e4ce081429b"), std::out_of_range);
    auto nbContacts = accInfo_.contactModel->getAllContacts().size();
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->searchContact("ring:f5a46751671918fe7210a3c31b9a9e4ce081429b");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfo_.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("f5a46751671918fe7210a3c31b9a9e4ce081429b"));
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);
    // "f5a46751671918fe7210a3c31b9a9e4ce081429b" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("f5a46751671918fe7210a3c31b9a9e4ce081429b"));
    // We should only have added one contact.
    // TODO: broken!!!!!!!!!!!!!!
    //CPPUNIT_ASSERT_EQUAL((nbContacts + 1), accInfo_.contactModel->getAllContacts().size());
}

void
ContactModelTester::testAddNewSIPContact()
{
    auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // "sipcontact0" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfoSip.contactModel->getContact("sipcontact0"), std::out_of_range);
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->searchContact("sipcontact0");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact0"));
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(addContactSigsCaught["contactAdded"] > 0, true);
    // "sipcontact0" should be "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact0"));
}

void
ContactModelTester::testAddAlreadyAddedContact()
{
    auto nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    // "contact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("contact1"));
    auto contact1 = accInfo_.contactModel->getContact("contact1");
    accInfo_.contactModel->addContact(contact1);
    // "contact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("contact1"));
    auto nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtBegin, nbContactsAtEnd);
}

void
ContactModelTester::testReceivesContactPresenceUpdate()
{
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("contact1"));
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->getContact("contact1").isPresent, false);
    auto newBuddyNotificationSigsCaught = WaitForSignalHelper([&]() {
            PresenceManager::instance().emitNewBuddyNotification(QString::fromStdString(accInfo_.id), "contact1", true, QString());
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .addSignal("modelSorted", *accInfo_.conversationModel, SIGNAL(modelSorted()))
        .addSignal("conversationUpdated", *accInfo_.conversationModel, SIGNAL(conversationUpdated(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(newBuddyNotificationSigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(newBuddyNotificationSigsCaught["modelSorted"] == 0, true);
    CPPUNIT_ASSERT_EQUAL(newBuddyNotificationSigsCaught["conversationUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->getContact("contact1").isPresent, true);
}

void
ContactModelTester::testRmRingContact()
{
    int nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    // "contact2" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("contact2"));
    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->removeContact("contact2");
        })
        .addSignal("contactRemoved", *accInfo_.contactModel, SIGNAL(contactRemoved(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"] > 0, true);
    int nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("contact2"), std::out_of_range);
}

void
ContactModelTester::testRmPendingContact()
{
    int nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("pending0"));
    auto removeContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->removeContact("pending0");
        })
        .addSignal("contactRemoved", *accInfo_.contactModel, SIGNAL(contactRemoved(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"] > 0, true);
    int nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("pending0"), std::out_of_range);
}

void
ContactModelTester::testRmSIPContact()
{
    auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // Search and add the temporaryContact
    auto searchContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->searchContact("sipcontact1");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(searchContactSigsCaught["modelUpdated"] > 0, true);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact1"));
    auto addContactSigsCaught = WaitForSignalHelper([&]() {
            accInfoSip.contactModel->addContact(temporaryContact);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
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
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(removeContactSigsCaught["contactRemoved"] > 0, true);
    int nbContactsAtEnd = accInfoSip.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    // "sipcontact1" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfoSip.contactModel->getContact("sipcontact1"), std::out_of_range);
}

void
ContactModelTester::testRmTemporaryContact()
{
    int nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    accInfo_.contactModel->removeContact("");
    int nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin);
}

void
ContactModelTester::testCountPendingRequests()
{
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("pending0"));
    accInfo_.contactModel->removeContact("pending0");
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->hasPendingRequests(), false);
    QByteArray payload = "FN:pending0\nPHOTO;ENCODING=BASE64;TYPE=PNG:";
    auto incomingTrustRequestSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitIncomingTrustRequest("ring1", "pending0", payload, 0);
        })
        .addSignal("contactAdded", *accInfo_.contactModel, SIGNAL(contactAdded(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(incomingTrustRequestSigsCaught["contactAdded"] > 0, true);
    CPPUNIT_ASSERT(accInfo_.contactModel->hasPendingRequests());
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->pendingRequestCount(), 1);
}

void
ContactModelTester::testCountPendingRequestsWithBlockedContact()
{
    CPPUNIT_ASSERT(accInfo_.contactModel->hasPendingRequests());
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->pendingRequestCount(), 1);
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("pending0"));
    accInfo_.contactModel->removeContact("pending0", true);
    CPPUNIT_ASSERT(!accInfo_.contactModel->hasPendingRequests());
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->pendingRequestCount(), 0);
}

void
ContactModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

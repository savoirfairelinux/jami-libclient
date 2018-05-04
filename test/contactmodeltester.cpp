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
#include <dbus/configurationmanager.h>

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
    // TODO
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
    ConfigurationManager::instance().emitIncomingTrustRequest("ring1", "pending0", payload, 0);
    auto contactAdded = WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
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
    accInfo_.contactModel->searchContact("dummy");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    auto temporaryContact = accInfo_.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("dummy"));
    accInfo_.contactModel->addContact(temporaryContact);
    auto contactAdded = WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("dummy"));
}

void
ContactModelTester::testAddRingURI()
{
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("f5a46751671918fe7210a3c31b9a9e4ce081429b"), std::out_of_range);
    auto nbContacts = accInfo_.contactModel->getAllContacts().size();
    // Search and add the temporaryContact
    accInfo_.contactModel->searchContact("ring:f5a46751671918fe7210a3c31b9a9e4ce081429b");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    auto temporaryContact = accInfo_.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("f5a46751671918fe7210a3c31b9a9e4ce081429b"));
    accInfo_.contactModel->addContact(temporaryContact);
    auto contactAdded = WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
    // "f5a46751671918fe7210a3c31b9a9e4ce081429b" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("f5a46751671918fe7210a3c31b9a9e4ce081429b"));
    // We should only have added one contact.
    CPPUNIT_ASSERT_EQUAL((nbContacts + 1), accInfo_.contactModel->getAllContacts().size());
}

void
ContactModelTester::testAddNewSIPContact()
{
    auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // "sipcontact0" should not be in "ring1" contacts.
    CPPUNIT_ASSERT_THROW(accInfoSip.contactModel->getContact("sipcontact0"), std::out_of_range);
    // Search and add the temporaryContact
    accInfoSip.contactModel->searchContact("sipcontact0");
    WaitForSignalHelper(*accInfoSip.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact0"));
    accInfoSip.contactModel->addContact(temporaryContact);
    auto contactAdded = WaitForSignalHelper(*accInfoSip.contactModel,
        SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
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
ContactModelTester::testRmRingContact()
{
    int nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    // "contact2" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("contact2"));
    accInfo_.contactModel->removeContact("contact2");
    auto contactRemoved = WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(contactRemoved(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactRemoved);
    int nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("contact2"), std::out_of_range);
}

void
ContactModelTester::testRmPendingContact()
{
    int nbContactsAtBegin = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_NO_THROW(accInfo_.contactModel->getContact("pending0"));
    accInfo_.contactModel->removeContact("pending0");
    auto contactRemoved = WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(contactRemoved(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactRemoved);
    int nbContactsAtEnd = accInfo_.contactModel->getAllContacts().size();
    CPPUNIT_ASSERT_EQUAL(nbContactsAtEnd, nbContactsAtBegin - 1);
    CPPUNIT_ASSERT_THROW(accInfo_.contactModel->getContact("pending0"), std::out_of_range);
}

void
ContactModelTester::testRmSIPContact()
{
    auto& accInfoSip = lrc_->getAccountModel().getAccountInfo("sip0");
    // Search and add the temporaryContact
    accInfoSip.contactModel->searchContact("sipcontact1");
    WaitForSignalHelper(*accInfoSip.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    auto temporaryContact = accInfoSip.contactModel->getContact("");
    CPPUNIT_ASSERT_EQUAL(temporaryContact.profileInfo.uri, std::string("sipcontact1"));
    accInfoSip.contactModel->addContact(temporaryContact);
    auto contactAdded = WaitForSignalHelper(*accInfoSip.contactModel,
        SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
    // "sipcontact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact1"));
    int nbContactsAtBegin = accInfoSip.contactModel->getAllContacts().size();
    // "sipcontact1" should be in "ring1" contacts.
    CPPUNIT_ASSERT_NO_THROW(accInfoSip.contactModel->getContact("sipcontact1"));
    accInfoSip.contactModel->removeContact("sipcontact1");
    auto contactRemoved = WaitForSignalHelper(*accInfoSip.contactModel,
        SIGNAL(contactRemoved(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactRemoved);
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
    ConfigurationManager::instance().emitIncomingTrustRequest("ring1", "pending0", payload, 0);
    auto contactAdded = WaitForSignalHelper(*accInfo_.contactModel,
                                            SIGNAL(contactAdded(const std::string& contactUri))).wait(1000);
    CPPUNIT_ASSERT(contactAdded);
    CPPUNIT_ASSERT(accInfo_.contactModel->hasPendingRequests());
    CPPUNIT_ASSERT_EQUAL(accInfo_.contactModel->pendingRequestCount(), 1);
}

void
ContactModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

/*
 *  Copyright (C) 2017-2019 Savoir-faire Linux Inc.
 *
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

#pragma once

// cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// std
#include <memory>

// Qt
#include <QObject>

// lrc
#include <api/lrc.h>
#include <api/account.h>

// utils
#include "utils/daemon_connector.h"

namespace ring
{
namespace test
{

class ContactModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ContactModelTester);
    //CPPUNIT_TEST(testAddNewContact);
    //CPPUNIT_TEST(testAddRingURI);
    //CPPUNIT_TEST(testAddAlreadyAddedContact);
    //CPPUNIT_TEST(testRmTemporaryContact);
    //CPPUNIT_TEST(testRmContact);
    //CPPUNIT_TEST(testRmPendingContact);
    //CPPUNIT_TEST(testCountPendingRequests);
    CPPUNIT_TEST(testCountPendingRequestsWithBlockedContact);
    /*
    CPPUNIT_TEST(testAddNewSIPContact);
    CPPUNIT_TEST(testReceivesContactPresenceUpdate);
    CPPUNIT_TEST(testRmSIPContact);
    CPPUNIT_TEST(testBanUnbanContact);
    */
    CPPUNIT_TEST_SUITE_END();

public:
    ContactModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();

    /**
     * Generate a pending request from "Ada" for account "Fred".
     * The pending contact should be added.
     */
    void receivesPendingRequest();

    /**
     * Add "Fred" to "Ada" contacts.
     * A new contact should be added.
     */
    void testAddNewContact();

    /**
     * re-add "Fred" to "Ada" contacts.
     * No new contact should appears.
     */
    void testAddAlreadyAddedContact();

    /**
     * Test if someone try to add ring:xxxxxxxx works
     */
    void testAddRingURI();

    /**
     * Remove "Fred" from "Ada" contacts.
     * The contact should be removed.
     */
    void testRmContact();

    /**
     * remove "Ada" from "Fred" contacts.
     * The contact should be removed.
     */
    void testRmPendingContact();

    /**
     * Method automatically called after each test by CppUnit
     */
    void tearDown();




    /**
     * Add "sipcontact0" to "sip0" contacts.
     * A new sip contact should be added.
     */
    void testAddNewSIPContact();
    /**
     * receive a presence update.
     * modelSorted should not be emitted, but conversationUpdated should.
     */
    void testReceivesContactPresenceUpdate();
    /**
     * Add and remove "sipcontact1" form "sip0" contacts.
     * The contact should be removed.
     */
    void testRmSIPContact();
    /**
     * Search "dummy" and try to remove it. Should not change anything
     */
    void testRmTemporaryContact();
    /**
     * Count contact requests
     */
    void testCountPendingRequests();
    /**
     * Count contact requests when banned contacts exists
     */
    void testCountPendingRequestsWithBlockedContact();
    /**
     * Try to ban and unban contacts
     */
    void testBanUnbanContact();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
    std::unique_ptr<Daemon> daemon_;
};

} // namespace test
} // namespace ring

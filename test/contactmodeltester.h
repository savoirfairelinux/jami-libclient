/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
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
#include "api/lrc.h"
#include "api/account.h"

namespace ring
{
namespace test
{

class ContactModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ContactModelTester);
    CPPUNIT_TEST(testGetAllContactsForRINGAccount);
    CPPUNIT_TEST(testReceivesPendingRequest);
    CPPUNIT_TEST(testAddNewRingContact);
    CPPUNIT_TEST(testAddRingURI);
    CPPUNIT_TEST(testAddNewSIPContact);
    CPPUNIT_TEST(testAddAlreadyAddedContact);
    CPPUNIT_TEST(testRmRingContact);
    CPPUNIT_TEST(testRmPendingContact);
    CPPUNIT_TEST(testRmSIPContact);
    CPPUNIT_TEST(testRmTemporaryContact);
    CPPUNIT_TEST(testCountPendingRequests);
    CPPUNIT_TEST(testBanUnbanContact);
    CPPUNIT_TEST_SUITE_END();

public:
    ContactModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Get all contacts for account "ring1".
     * Contacts are defined in configurationmanager_mock.h
     */
    void testGetAllContactsForRINGAccount();
    /**
     * Generate a pending request from "pending0" for account "ring1".
     * The pending contact should be added.
     */
    void testReceivesPendingRequest();
    /**
     * Add "dummy" to "ring1" contacts.
     * A new ring contact should be added.
     */
    void testAddNewRingContact();
    /**
     * Test if someone try to add ring:xxxxxxxx works correctly and don't create 2 contacts
     */
    void testAddRingURI();
    /**
     * Add "sipcontact0" to "sip0" contacts.
     * A new sip contact should be added.
     */
    void testAddNewSIPContact();
    /**
     * re-add "contact1" to "ring1" contacts.
     * No new contact should appears.
     */
    void testAddAlreadyAddedContact();
    /**
     * Remove "dummy" from "ring1" contacts.
     * The contact should be removed.
     */
    void testRmRingContact();
    /**
     * remove "pending0" from "ring1" contacts.
     * The contact should be removed.
     */
    void testRmPendingContact();
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
     * Try to ban and unban contacts
     */
    void testBanUnbanContact();
    /**
     * Method automatically called after each test by CppUnit
     */
    void tearDown();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
    const lrc::api::account::Info& accInfo_;
};

} // namespace test
} // namespace ring

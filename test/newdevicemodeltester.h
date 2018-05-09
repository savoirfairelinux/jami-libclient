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

class NewDeviceModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NewDeviceModelTester);
    CPPUNIT_TEST(testGetAllDevices);
    CPPUNIT_TEST(testGetValidDevice);
    CPPUNIT_TEST(testGetInvalidDevice);
    CPPUNIT_TEST(testNewDeviceAdded);
    CPPUNIT_TEST(testRevokeDevice);
    CPPUNIT_TEST(testRevokeDeviceInvalidDevice);
    CPPUNIT_TEST(testRevokeDeviceInvalidPassword);
    CPPUNIT_TEST(testSetCurrentDeviceName);
    CPPUNIT_TEST_SUITE_END();

public:
    NewDeviceModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Retrieve all devices of an account
     */
    void testGetAllDevices();
    /**
     * Test getting an existing device
     */
    void testGetValidDevice();
    /**
     * Test getting a non existing device
     */
    void testGetInvalidDevice();
    /**
     * Test new device added
     */
    void testNewDeviceAdded();
    /**
     * Test to remove a device (valid device, valid password)
     */
    void testRevokeDevice();
    /**
     * Test to remove a device (invalid device, valid password)
     */
    void testRevokeDeviceInvalidDevice();
    /**
     * Test to remove a device (valid device, invalid password)
     */
    void testRevokeDeviceInvalidPassword();
    /**
     * Test to change the current device name
     */
    void testSetCurrentDeviceName();
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

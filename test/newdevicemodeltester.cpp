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
#include "newdevicemodeltester.h"

// std
#include <string>

// Qt
#include "utils/waitforsignalhelper.h"

// Lrc
#include <api/newaccountmodel.h>
#include <api/newdevicemodel.h>
#include <dbus/configurationmanager.h>


namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(NewDeviceModelTester);

NewDeviceModelTester::NewDeviceModelTester()
: lrc_(new lrc::api::Lrc())
, accInfo_(lrc_->getAccountModel().getAccountInfo("ring3"))
{

}

void
NewDeviceModelTester::setUp()
{

}

void
NewDeviceModelTester::testGetAllDevices()
{
    // See mocked ConfigurationManager::getKnownRingDevices() and getAccountDetails()
    auto devices = accInfo_.deviceModel->getAllDevices();
    // Here, we should have 2 devices (device0 pc) and (device1 tel)
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(devices.size()), 2);
    auto device0 = devices.front();
    CPPUNIT_ASSERT_EQUAL(device0.isCurrent, true);
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device0"));
    CPPUNIT_ASSERT_EQUAL(device0.name, std::string("pc"));
    auto device1 = devices.back();
    CPPUNIT_ASSERT_EQUAL(device1.isCurrent, false);
    CPPUNIT_ASSERT_EQUAL(device1.id, std::string("device1"));
    CPPUNIT_ASSERT_EQUAL(device1.name, std::string("tel"));
}

void
NewDeviceModelTester::testGetValidDevice()
{
    // device0 defined in mocked ConfigurationManager
    auto device0 = accInfo_.deviceModel->getDevice("device0");
    CPPUNIT_ASSERT_EQUAL(device0.isCurrent, true);
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device0"));
    CPPUNIT_ASSERT_EQUAL(device0.name, std::string("pc"));
}

void
NewDeviceModelTester::testGetInvalidDevice()
{
    // notADevice not defined in mocked ConfigurationManager
    auto device0 = accInfo_.deviceModel->getDevice("notADevice");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string(""));
}

void
NewDeviceModelTester::testNewDeviceAdded()
{
    // this will add a new device for ring3 (see mock)
    ConfigurationManager::instance().addNewDevice("ring3", "device2", "tv");
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceAdded(const std::string& id))).wait(1000);
    auto device0 = accInfo_.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.isCurrent, false);
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
    CPPUNIT_ASSERT_EQUAL(device0.name, std::string("tv"));
    // Revoke device for other tests
    // NOTE: should be removed when test will not depends from each others
    // See mock
    ConfigurationManager::instance().revokeDevice("ring3", "", "device2");
}

void
NewDeviceModelTester::testRevokeDevice()
{
    // this will add a new device for ring3 (see mock)
    ConfigurationManager::instance().addNewDevice("ring3", "device2", "tv");
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceAdded(const std::string& id))).wait(1000);
    // Then revoke device
    accInfo_.deviceModel->revokeDevice("device2", "");  // empty password = correct
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceRevoked(const std::string& id,
                            const lrc::api::NewDeviceModel::Status status)))
        .wait(1000);
    // Should not exists anymore
    auto device2 = accInfo_.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device2.id, std::string(""));
}

void
NewDeviceModelTester::testRevokeDeviceInvalidDevice()
{
    // this will add a new device for ring3 (see mock)
    ConfigurationManager::instance().addNewDevice("ring3", "device2", "tv");
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceAdded(const std::string& id))).wait(1000);
    // Then revoke device
    accInfo_.deviceModel->revokeDevice("device3", "");  // empty password = correct
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceRevoked(const std::string& id,
                            const lrc::api::NewDeviceModel::Status status)))
        .wait(1000);
    // device2 still exists
    auto device0 = accInfo_.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
    // Revoke device for other tests
    // NOTE: should be removed when test will not depends from each others
    // See mock
    ConfigurationManager::instance().revokeDevice("ring3", "", "device2");
}

void
NewDeviceModelTester::testRevokeDeviceInvalidPassword()
{
    // this will add a new device for ring3 (see mock)
    ConfigurationManager::instance().addNewDevice("ring3", "device2", "tv");
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceAdded(const std::string& id))).wait(1000);
    // Then revoke device
    accInfo_.deviceModel->revokeDevice("device2", "notAPass");  // !empty password = incorrect
    // Wait for deviceAdded
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceRevoked(const std::string& id,
                            const lrc::api::NewDeviceModel::Status status)))
        .wait(1000);
    // device2 still exists
    auto device0 = accInfo_.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
    // Revoke device for other tests
    // NOTE: should be removed when test will not depends from each others
    // See mock
    ConfigurationManager::instance().revokeDevice("ring3", "", "device2");
}

void
NewDeviceModelTester::testSetCurrentDeviceName()
{
    // Will change the name of device0
    accInfo_.deviceModel->setCurrentDeviceName("NewDeviceName");
    // Will call mocked ConfigurationManager::setAccountDetails()
    // Because known devices changed, NewDeviceModel::deviceUpdated will be emitted
    WaitForSignalHelper(*accInfo_.deviceModel,
        SIGNAL(deviceUpdated(const std::string& id))).wait(1000);
    // device0 should have a new name now.
    auto device0 = accInfo_.deviceModel->getDevice("device0");
    CPPUNIT_ASSERT_EQUAL(device0.isCurrent, true);
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device0"));
    CPPUNIT_ASSERT_EQUAL(device0.name, std::string("NewDeviceName"));
}

void
NewDeviceModelTester::tearDown()
{
    accInfo_.deviceModel->setCurrentDeviceName("pc");
}

} // namespace test
} // namespace ring

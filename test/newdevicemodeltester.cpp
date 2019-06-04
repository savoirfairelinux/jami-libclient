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
#include "newdevicemodeltester.h"

// daemon
#include <account_const.h>

// std
#include <string>

// Qt
#include "utils/daemon_connector.h"

// Lrc
#include <api/account.h>
#include <api/newaccountmodel.h>
#include <api/contactmodel.h>
#include <api/conversationmodel.h>
#include <api/newcodecmodel.h>
#include <api/newcallmodel.h>
#include <api/newdevicemodel.h>
#include <dbus/configurationmanager.h>


namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(NewDeviceModelTester);

NewDeviceModelTester::NewDeviceModelTester()
: lrc_(new lrc::api::Lrc())
{

}

void
NewDeviceModelTester::setUp()
{
    daemon_ = std::make_unique<Daemon>();
    daemon_->addAccount("Fred");
}

void
NewDeviceModelTester::testGetAllDevices()
{
    // We should get the same size between DBus and LRC
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto devices = accInfo.deviceModel->getAllDevices();
    const MapStringString accountDevices = ConfigurationManager::instance().getKnownRingDevices(accountId.c_str());
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(devices.size()), static_cast<int>(accountDevices.size()));

    const MapStringString aDetails = ConfigurationManager::instance().getAccountDetails(accountId.c_str());
    auto currentDeviceId = aDetails.value(DRing::Account::ConfProperties::RING_DEVICE_ID).toStdString();

    // Get preferred device should be the same
    auto hasDefaultDevice = false;
    for (const auto& device : devices) {
        if (device.isCurrent) {
            hasDefaultDevice = true;
            CPPUNIT_ASSERT_EQUAL(currentDeviceId, device.id);
        }
        auto deviceFound = false;
        for (const auto& aDevice : accountDevices.toStdMap()) {
            if (device.id == aDevice.first.toStdString()) {
                deviceFound = true;
                CPPUNIT_ASSERT_EQUAL(aDevice.second.toStdString(), device.name);
            }
        }
        CPPUNIT_ASSERT(deviceFound);
    }
    CPPUNIT_ASSERT(hasDefaultDevice);
}

void
NewDeviceModelTester::testGetInvalidDevice()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto device0 = accInfo.deviceModel->getDevice("notADevice");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string(""));
}

void
NewDeviceModelTester::testNewDeviceAdded()
{
    auto accountId = daemon_->getAccountId("Fred");
    daemon_->addNewDevice(accountId, "device2", "tv");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto device0 = accInfo.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.isCurrent, false);
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
    CPPUNIT_ASSERT_EQUAL(device0.name, std::string("tv"));
}

void
NewDeviceModelTester::testRevokeDevice()
{
    auto accountId = daemon_->getAccountId("Fred");
    daemon_->addNewDevice(accountId, "device2", "tv");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    accInfo.deviceModel->revokeDevice("device2", "" /* empty password */);
    auto device2 = accInfo.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device2.id, std::string(""));
}

void
NewDeviceModelTester::testRevokeDeviceInvalidDevice()
{
    auto accountId = daemon_->getAccountId("Fred");
    daemon_->addNewDevice(accountId, "device2", "tv");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    // Revoke a wrong device
    accInfo.deviceModel->revokeDevice("device3", "" /* empty password */);
    // device2 still exists
    auto device0 = accInfo.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
}

void
NewDeviceModelTester::testRevokeDeviceInvalidPassword()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    accInfo.deviceModel->setCurrentDeviceName("device2");
    // device2 still exists
    auto device0 = accInfo.deviceModel->getDevice("device2");
    CPPUNIT_ASSERT_EQUAL(device0.id, std::string("device2"));
}

void
NewDeviceModelTester::testSetCurrentDeviceName()
{
    auto accountId = daemon_->getAccountId("Fred");
    daemon_->addNewDevice(accountId, "device2", "tv");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    // Revoke with a wrong password
    accInfo.deviceModel->setCurrentDeviceName("NewDeviceName");

    auto devices = accInfo.deviceModel->getAllDevices();
    auto hasDefaultDevice = false;
    for (const auto& device : devices) {
        if (device.isCurrent) {
            hasDefaultDevice = true;
            CPPUNIT_ASSERT_EQUAL(device.name, std::string("NewDeviceName"));
        }
    }
    CPPUNIT_ASSERT(hasDefaultDevice);
}

void
NewDeviceModelTester::tearDown()
{
    daemon_.reset();
}

} // namespace test
} // namespace ring

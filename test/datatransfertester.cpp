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
#include "datatransfertester.h"

// std
#include <algorithm>

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"

// Lrc
#include <api/newaccountmodel.h>
#include <api/contact.h>
#include <api/conversationmodel.h>
#include <dbus/configurationmanager.h>
#include <api/datatransfer.h>
#include <api/datatransfermodel.h>
#include <datatransfer_interface.h>

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(DataTransferTester);

DataTransferTester::DataTransferTester()
: lrc_(new lrc::api::Lrc())
, accInfo_(lrc_->getAccountModel().getAccountInfo("ring0"))
{

}

void
DataTransferTester::setUp()
{

}

void
DataTransferTester::testReceivesMusic()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto intBaseSize = accInfo_.conversationModel->filteredConversation(0).interactions.size();

    lrc::api::datatransfer::Info info {
        "1", lrc::api::datatransfer::Status::on_connection, 0, 10 * 1024 * 1024, 0,
        "./", "glados.mp3", "ring0", firstConversation.participants[0]
    };
    ConfigurationManager::instance().setDataTransferInfo(1, info);
    ConfigurationManager::instance().emitDataTransferEvent(1, DRing::DataTransferEventCode::created);
    auto intFinalSize = accInfo_.conversationModel->filteredConversation(0).interactions.size();
    CPPUNIT_ASSERT_EQUAL(intFinalSize, intBaseSize + 1);
    // base conversation + file transfer
}

void
DataTransferTester::testReceivesImage5MbNoPref()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    lrc::api::datatransfer::Info info {
        "1", lrc::api::datatransfer::Status::on_connection, 0, 5 * 1024 * 1024, 0,
        "./", "glados.jpg", "ring0", firstConversation.participants[0]
    };
    ConfigurationManager::instance().setDataTransferInfo(2, info);

    auto dataTransferEventSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitDataTransferEvent(2, DRing::DataTransferEventCode::created);
            ConfigurationManager::instance().emitDataTransferEvent(2, DRing::DataTransferEventCode::wait_host_acceptance);
        })
        .addSignal("interactionStatusUpdated", *accInfo_.conversationModel, SIGNAL(interactionStatusUpdated(const std::string&, uint64_t, const api::interaction::Info&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(dataTransferEventSigsCaught["interactionStatusUpdated"], 1);

    auto lastIt = accInfo_.conversationModel->filteredConversation(0).interactions.rbegin()->second;
    CPPUNIT_ASSERT(lastIt.status == lrc::api::interaction::Status::TRANSFER_AWAITING_HOST);
}

void
DataTransferTester::testReceivesImage5Mb()
{
    lrc_->getDataTransferModel().downloadDirectory = "/";
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    lrc::api::datatransfer::Info info {
        "1", lrc::api::datatransfer::Status::on_connection, 0, 5 * 1024 * 1024, 0,
        "./", "glados.jpg", "ring0", firstConversation.participants[0]
    };
    ConfigurationManager::instance().setDataTransferInfo(3, info);

    auto dataTransferEventSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitDataTransferEvent(3, DRing::DataTransferEventCode::created);
            ConfigurationManager::instance().emitDataTransferEvent(3, DRing::DataTransferEventCode::wait_host_acceptance);
        })
        .addSignal("interactionStatusUpdated", *accInfo_.conversationModel, SIGNAL(interactionStatusUpdated(const std::string&, uint64_t, const api::interaction::Info&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(dataTransferEventSigsCaught["interactionStatusUpdated"], 1);

    auto lastIt = accInfo_.conversationModel->filteredConversation(0).interactions.rbegin()->second;
    CPPUNIT_ASSERT(lastIt.status == lrc::api::interaction::Status::TRANSFER_ACCEPTED);
}

void
DataTransferTester::testReceivesImage50Mb()
{
    lrc_->getDataTransferModel().downloadDirectory = "/";
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    lrc::api::datatransfer::Info info {
        "1", lrc::api::datatransfer::Status::on_connection, 0, 50 * 1024 * 1024, 0,
        "./", "glados.jpg", "ring0", firstConversation.participants[0]
    };
    ConfigurationManager::instance().setDataTransferInfo(3, info);

    auto dataTransferEventSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitDataTransferEvent(3, DRing::DataTransferEventCode::created);
            ConfigurationManager::instance().emitDataTransferEvent(3, DRing::DataTransferEventCode::wait_host_acceptance);
        })
        .addSignal("interactionStatusUpdated", *accInfo_.conversationModel, SIGNAL(interactionStatusUpdated(const std::string&, uint64_t, const api::interaction::Info&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(dataTransferEventSigsCaught["interactionStatusUpdated"], 1);

    auto lastIt = accInfo_.conversationModel->filteredConversation(0).interactions.rbegin()->second;
    CPPUNIT_ASSERT(lastIt.status == lrc::api::interaction::Status::TRANSFER_AWAITING_HOST);
}

void
DataTransferTester::tearDown()
{

}

} // namespace test
} // namespace ring

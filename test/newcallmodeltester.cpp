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
#include "newcallmodeltester.h"

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"

// Lrc
#include <api/newaccountmodel.h>
#include <api/newcallmodel.h>
#include <api/conversationmodel.h>
#include <api/call.h>
#include <dbus/callmanager.h>

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(NewCallModelTester);

NewCallModelTester::NewCallModelTester()
: lrc_(new lrc::api::Lrc())
{

}

void
NewCallModelTester::setUp()
{
    daemon_ = std::make_unique<Daemon>();
    daemon_->addAccount("Fred");
    daemon_->addAccount("Ada");
}

void
NewCallModelTester::testCreateAndGetCall()
{
    /*auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto peer = "ring:" + accInfoAda.profileInfo.uri;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // TODO daemon->waitForPresence("Ada");

    std::string callId;
    auto callSignals = WaitForSignalHelper([&]() {
        callId = accInfo.callModel->createCall(peer);
    })
    .addSignal("incomingCall", *accInfoAda.callModel,
        SIGNAL(newIncomingCall(const std::string&, const std::string&)))
    .wait(10000);
    CPPUNIT_ASSERT_EQUAL(callSignals["incomingCall"], 1);

    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo.callModel->hasCall(callId));
    auto& call = accInfo.callModel->getCallFromURI(peer);
    auto& callFromId = accInfo.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peerUri, std::string(peer));*/
}

void
NewCallModelTester::testCreateAndGetAudioOnlyCall()
{
    /*auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto peer = "ring:" + accInfoAda.profileInfo.uri;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // TODO daemon->waitForPresence("Ada");

    std::string callId;
    auto callSignals = WaitForSignalHelper([&]() {
        callId = accInfo.callModel->createCall(peer, true);
    })
    .addSignal("incomingCall", *accInfoAda.callModel,
        SIGNAL(newIncomingCall(const std::string&, const std::string&)))
    .wait(10000);
    CPPUNIT_ASSERT_EQUAL(callSignals["incomingCall"], 1);

    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo.callModel->hasCall(callId));
    auto& call = accInfo.callModel->getCallFromURI(peer);
    auto& callFromId = accInfo.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peerUri, std::string(peer));*/
}

void
NewCallModelTester::testAcceptHoldUnholdHangupCall()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto fredURI = "ring:" + accInfo.profileInfo.uri;
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto adaURI = "ring:" + accInfoAda.profileInfo.uri;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // TODO daemon->waitForPresence("Ada");

    // TODO daemon (waitForState)

    std::string callId;
    auto callSignals = WaitForSignalHelper([&]() {
        callId = accInfo.callModel->createCall(adaURI, true);
    })
    .addSignal("incomingCall", *accInfoAda.callModel,
        SIGNAL(newIncomingCall(const std::string&, const std::string&)))
    .wait(10000);
    CPPUNIT_ASSERT_EQUAL(callSignals["incomingCall"], 1);
    CPPUNIT_ASSERT(accInfo.callModel->hasCall(callId));

    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // TODO daemon->waitForPresence("Ada");

    auto callAda = accInfoAda.callModel->getCallFromURI(fredURI);
    // Test status
    callSignals = WaitForSignalHelper([&]() {
        accInfoAda.callModel->accept(callAda.id);
    })
    .addSignal("statusChanged", *accInfo.callModel,
        SIGNAL(callStatusChanged(const std::string&, int)))
    .wait(1000);
    bool is_calling = false;
    for (int i = 0; i < 10; ++i) {
        if (callAda.status == lrc::api::call::Status::IN_PROGRESS) {
            is_calling = true;
            break;
        }
        callSignals = WaitForSignalHelper([&]() {})
        .addSignal("statusChanged", *accInfo.callModel,
            SIGNAL(callStatusChanged(const std::string&, int)))
        .wait(1000);
        callAda = accInfoAda.callModel->getCallFromURI(fredURI);
    }
    CPPUNIT_ASSERT(is_calling);

    // Wait for pause
    callSignals = WaitForSignalHelper([&]() {
        accInfoAda.callModel->togglePause(callAda.id);
    })
    .addSignal("statusChanged", *accInfo.callModel,
        SIGNAL(callStatusChanged(const std::string&)))
    .wait(1000);
    bool is_paused = false;
    for (int i = 0; i < 10; ++i) {
        if (callAda.status == lrc::api::call::Status::PAUSED) {
            is_paused = true;
            break;
        }
        callSignals = WaitForSignalHelper([&]() {})
        .addSignal("statusChanged", *accInfo.callModel,
            SIGNAL(callStatusChanged(const std::string&, int)))
        .wait(1000);
        callAda = accInfoAda.callModel->getCallFromURI(fredURI);
    }
    CPPUNIT_ASSERT(is_paused);

    // Wait for unpause
    callSignals = WaitForSignalHelper([&]() {
        accInfoAda.callModel->togglePause(callAda.id);
    })
    .addSignal("statusChanged", *accInfo.callModel,
        SIGNAL(callStatusChanged(const std::string&)))
    .wait(1000);
    bool is_unpaused = false;
    for (int i = 0; i < 10; ++i) {
        if (callAda.status == lrc::api::call::Status::IN_PROGRESS) {
            is_unpaused = true;
            break;
        }
        callSignals = WaitForSignalHelper([&]() {})
        .addSignal("statusChanged", *accInfo.callModel,
            SIGNAL(callStatusChanged(const std::string&, int)))
        .wait(1000);
        callAda = accInfoAda.callModel->getCallFromURI(fredURI);
    }
    CPPUNIT_ASSERT(is_unpaused);

    // Wait for ended
    callSignals = WaitForSignalHelper([&]() {
        accInfoAda.callModel->hangUp(callAda.id);
    })
    .addSignal("statusChanged", *accInfo.callModel,
        SIGNAL(callStatusChanged(const std::string&)))
    .wait(1000);
    bool is_ended = false;
    for (int i = 0; i < 10; ++i) {
        if (callAda.status == lrc::api::call::Status::ENDED) {
            is_ended = true;
            break;
        }
        callSignals = WaitForSignalHelper([&]() {})
        .addSignal("statusChanged", *accInfo.callModel,
            SIGNAL(callStatusChanged(const std::string&, int)))
        .wait(1000);
        callAda = accInfoAda.callModel->getCallFromURI(fredURI);
    }
    CPPUNIT_ASSERT(is_ended);
}


void
NewCallModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

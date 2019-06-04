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
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto peer = accInfoAda.profileInfo.uri;

    auto callId = accInfo.callModel->createCall(peer);
    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo.callModel->hasCall(callId));
    auto& call = accInfo.callModel->getCallFromURI(peer);
    auto& callFromId = accInfo.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peer, std::string(peer));
}

void
NewCallModelTester::testCreateAndGetAudioOnlyCall()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto peer = accInfoAda.profileInfo.uri;

    auto callId = accInfo.callModel->createCall(peer, true);
    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo.callModel->hasCall(callId));
    auto& call = accInfo.callModel->getCallFromURI(peer);
    auto& callFromId = accInfo.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peer, std::string(peer));
}

void
NewCallModelTester::testAcceptHoldUnholdHangupCall()
{
    auto accountId = daemon_->getAccountId("Fred");
    const auto& accInfo = lrc_->getAccountModel().getAccountInfo(accountId);
    auto fredURI = accInfo.profileInfo.uri;
    auto accountIdAda = daemon_->getAccountId("Ada");
    const auto& accInfoAda = lrc_->getAccountModel().getAccountInfo(accountIdAda);
    auto adaURI = accInfoAda.profileInfo.uri;

    // Test if incoming call is detected
    std::string callId;
    auto incomingCallSigsCaught = WaitForSignalHelper([&]() {
            callId = accInfo.callModel->createCall(adaURI, true);
        })
        .addSignal("newIncomingCall", *accInfoAda.callModel, SIGNAL(newIncomingCall(const std::string&, const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(incomingCallSigsCaught["newIncomingCall"], 1);

    // Test status
    CPPUNIT_ASSERT(accInfoAda.callModel->hasCall(callId));
    accInfoAda.callModel->accept(callId);
    auto& call = accInfoAda.callModel->getCallFromURI(fredURI);
    CPPUNIT_ASSERT_EQUAL((int)call.status, (int)lrc::api::call::Status::IN_PROGRESS);
    accInfoAda.callModel->togglePause(callId);
    auto& callHold = accInfoAda.callModel->getCallFromURI(fredURI);
    CPPUNIT_ASSERT_EQUAL((int)callHold.status, (int)lrc::api::call::Status::PAUSED);
    accInfoAda.callModel->togglePause(callId);
    auto& callUnhold = accInfoAda.callModel->getCallFromURI(fredURI);
    CPPUNIT_ASSERT_EQUAL((int)callUnhold.status, (int)lrc::api::call::Status::IN_PROGRESS);
    accInfoAda.callModel->hangUp(callId);
    auto& callOver = accInfoAda.callModel->getCallFromURI(fredURI);
    CPPUNIT_ASSERT_EQUAL((int)callOver.status, (int)lrc::api::call::Status::ENDED);
}


void
NewCallModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

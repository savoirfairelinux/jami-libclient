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
, accInfo_(lrc_->getAccountModel().getAccountInfo("ring2"))
{

}

void
NewCallModelTester::setUp()
{

}

void
NewCallModelTester::testCreateAndGetCall()
{
    auto callId = accInfo_.callModel->createCall("ring:contact0");
    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo_.callModel->hasCall(callId));
    auto& call = accInfo_.callModel->getCallFromURI("ring:contact0");
    auto& callFromId = accInfo_.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peer, std::string("ring:contact0"));
}

void
NewCallModelTester::testCreateAndGetAudioOnlyCall()
{
    auto callId = accInfo_.callModel->createCall("ring:contact0", true);
    CPPUNIT_ASSERT(!callId.empty());
    CPPUNIT_ASSERT(accInfo_.callModel->hasCall(callId));
    auto& call = accInfo_.callModel->getCallFromURI("ring:contact0");
    auto& callFromId = accInfo_.callModel->getCall(call.id);
    CPPUNIT_ASSERT_EQUAL(callFromId.peer, std::string("ring:contact0"));
}

void
NewCallModelTester::testAcceptHoldUnholdHangupCall()
{
    std::string callId = "ring:contact1";

    auto incomingCallSigsCaught = WaitForSignalHelper([&]() {
            CallManager::instance().emitIncomingCall("ring2", callId.c_str(), "ring:contact1");
        })
        .addSignal("newIncomingCall", *accInfo_.callModel, SIGNAL(newIncomingCall(const std::string&, const std::string&)))
        .wait(1000);
    CPPUNIT_ASSERT_EQUAL(incomingCallSigsCaught["newIncomingCall"], 1);

    CPPUNIT_ASSERT(accInfo_.callModel->hasCall(callId));
    accInfo_.callModel->accept(callId);
    auto& call = accInfo_.callModel->getCallFromURI("ring:contact1");
    CPPUNIT_ASSERT_EQUAL((int)call.status, (int)lrc::api::call::Status::IN_PROGRESS);
    accInfo_.callModel->togglePause(callId);
    auto& callHold = accInfo_.callModel->getCallFromURI("ring:contact1");
    CPPUNIT_ASSERT_EQUAL((int)callHold.status, (int)lrc::api::call::Status::PAUSED);
    accInfo_.callModel->togglePause(callId);
    auto& callUnhold = accInfo_.callModel->getCallFromURI("ring:contact1");
    CPPUNIT_ASSERT_EQUAL((int)callUnhold.status, (int)lrc::api::call::Status::IN_PROGRESS);
    accInfo_.callModel->hangUp(callId);
    auto& callOver = accInfo_.callModel->getCallFromURI("ring:contact1");
    CPPUNIT_ASSERT_EQUAL((int)callOver.status, (int)lrc::api::call::Status::ENDED);
}


void
NewCallModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

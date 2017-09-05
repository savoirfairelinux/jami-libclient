/*
 *  Copyright (C) 2017 Savoir-faire Linux Inc.
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
#include "conversationmodeltester.h"

// std
#include <cstdio>

// Qt
#include <QString>

// Lrc
#include "dbus/configurationmanager.h"

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(ConversationModelTester);

ConversationModelTester::ConversationModelTester()
: lrc_(new lrc::Lrc()),
accInfo_(lrc_->getAccountModel().getAccountInfo("account0"))
{ }

void
ConversationModelTester::setUp()
{
    // TODO
}

void
ConversationModelTester::testAddValidConversation()
{
    // TODO test add valid contact

    // conversation::Info getConversation(const unsigned int row) const;}

void
ConversationModelTester::testAddInvalidConversation()
{
    // TODO test add invalid contact

}

void
ConversationModelTester::testRmConversation()
{

}

void
ConversationModelTester::testFilterAndGetConversations()
{

}

void
ConversationModelTester::testSendMessage()
{

}

void
ConversationModelTester::testPlaceCall()
{

}

void
ConversationModelTester::testCreateConference()
{

}

void
ConversationModelTester::testSelectConversation()
{

}

void
ConversationModelTester::testRmHistory()
{

}

void
ConversationModelTester::testRmContact()
{

}

void
ConversationModelTester::tearDown()
{
    // Remove ring.db
    std::remove(ringDB);
}

} // namespace test
} // namespace ring

/*
 *  Copyright (C) 2017 Savoir-faire Linux Inc.
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

// lrc
#include "lrc.h"

namespace ring
{
namespace test
{

class ConversationModelTester : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(ConversationModelTester);
    CPPUNIT_TEST(setUp);
    CPPUNIT_TEST(testAddValidConversation);
    CPPUNIT_TEST(testAddInvalidConversation);
    CPPUNIT_TEST(testRmConversation);
    CPPUNIT_TEST(testFilterAndGetConversations);
    CPPUNIT_TEST(testSendMessage);
    CPPUNIT_TEST(testPlaceCall);
    CPPUNIT_TEST(testCreateConference);
    CPPUNIT_TEST(testSelectConversation);
    CPPUNIT_TEST(testRmHistory);
    CPPUNIT_TEST(testRmContact);
    CPPUNIT_TEST(tearDown);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void testAddValidConversation();
    void testAddInvalidConversation();
    void testRmConversation();
    void testFilterAndGetConversations();
    void testSendMessage();
    void testPlaceCall();
    void testCreateConference();
    void testSelectConversation();
    void testRmHistory();
    void testRmContact();
    void tearDown();

protected:
    std::unique_ptr<lrc> lrc_;
    const lrc::account::Info& accInfo_;
};

} // namespace test
} // namespace ring

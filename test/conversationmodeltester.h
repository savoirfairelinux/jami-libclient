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

class ConversationModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConversationModelTester);
    CPPUNIT_TEST(testAddValidConversation);
    CPPUNIT_TEST(testAddInvalidConversation);
    CPPUNIT_TEST(testRmConversation);
    CPPUNIT_TEST(testFilterAndGetConversations);
    CPPUNIT_TEST(testSendMessageAndClearHistory);
    CPPUNIT_TEST(testSendMessagesAndClearInteraction);
    CPPUNIT_TEST(testRetryToSendTextInteraction);
    CPPUNIT_TEST(testRetryToSendFileInteraction);
    CPPUNIT_TEST(testRetryInvalidInteraction);
    CPPUNIT_TEST(testRetryIncomingInteraction);
    CPPUNIT_TEST(testRetryContactInteraction);
    CPPUNIT_TEST(testRetryCallInteraction);
    CPPUNIT_TEST(testReceiveMessageAndSetRead);
    CPPUNIT_TEST(testPlaceCall);
    CPPUNIT_TEST(testCreateConference);
    CPPUNIT_TEST(testPlaceAudioOnlyCall);
    CPPUNIT_TEST(testClearUnreadInteractions);
    CPPUNIT_TEST(testSendMessageToBannedContact);
    CPPUNIT_TEST(testFilterBannedContact);
    CPPUNIT_TEST(testPlaceCallWithBannedContact);
    CPPUNIT_TEST_SUITE_END();

public:
    ConversationModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Add a new conversation between account "ring0" and "dummy"
     */
    void testAddValidConversation();
    /**
     * Try to add a new conversation between account "ring0" and "notAContact"
     * but "notAContact" is not a ringId of another user, so we can't create a
     * new conversation with "notAContact".
     */
    void testAddInvalidConversation();
    /**
     * Remove the conversation between account "ring0" and "contact0"
     * NOTE: "contact0" is already a contact for "ring0",
     * cf. mock/configurationmanager_mock
     */
    void testRmConversation();
    /**
     * Test the behavior of setFilter(query) with different queries.
     */
    void testFilterAndGetConversations();
    /**
     * Send "Hello World!" to the first conversation and clear the history
     */
    void testSendMessageAndClearHistory();
    /**
     * Make sure it is not possible to send a message to a banned contact
     */
    void testSendMessageToBannedContact();
    /**
     * Make sure banned contacts only appear in perfect-match filter searches.
     */
    void testFilterBannedContact();
    /**
     * Send multiple messages to the first conversation and clear one interaction
     */
    void testSendMessagesAndClearInteraction();
    /**
     * Send an old failed outgoing text interaction
     */
    void testRetryToSendTextInteraction();
    /**
     * Send an old failed outgoing file interaction
     */
    void testRetryToSendFileInteraction();
    /**
     * Retry an unexistant interaction
     */
    void testRetryInvalidInteraction();
    /**
     * Retry an incoming interaction
     */
    void testRetryIncomingInteraction();
    /**
     * Retry a contact interaction
     */
    void testRetryContactInteraction();
    /**
     * Retry a call interaction
     */
    void testRetryCallInteraction();
    /**
     * Receives a message from a conversation and set this message READ
     */
    void testReceiveMessageAndSetRead();
    /**
     * Call the first conversation
     */
    void testPlaceCall();
    /**
     * Make sure it is not possible to call a banned contact
     */
    void testPlaceCallWithBannedContact();
    /**
     * Start and audio-only call with the first conversation
     */
    void testPlaceAudioOnlyCall();
    /**
     * Create a conference with the two first conversations
     */
    void testCreateConference();
    /**
     * Clear any unread text messages from a conversation
     */
    void testClearUnreadInteractions();
    /**
     * Method automatically called after each test by CppUnit
     */
    void tearDown();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
    const lrc::api::account::Info& accInfo_;

    // Helpers

    /**
     * Ban contact with passed uri
     */
    void banContact(std::string uri);
    /**
     * Return whether passed uri already maps to a contact or not
     */
    bool isAContact(std::string uri);
    /**
     * Add passed usename to contacts and return its uri
     */
    std::string addToContacts(std::string username);
    /**
     * Return whether a converation with passed contact uri exists or not
     */
    bool hasConversationWithContact(std::string uri);
};

} // namespace test
} // namespace ring

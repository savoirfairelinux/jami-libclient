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
#include <algorithm>

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
    accInfo_.removeConversation("dummy");
    // TODO wait for signal from daemon
}

void
ConversationModelTester::testAddValidConversation()
{
    // Dummy should not be in contacts
    auto contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("dummy") == contacts.end());
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i == conversations.end());

    accInfo_.addConversation("dummy");
    // TODO wait for signal from daemon
    contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("dummy") != contacts.end());
    conversations = accInfo_.conversationModel.getFilteredConversations();
    i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i != conversations.end());

void
ConversationModelTester::testAddInvalidConversation()
{
    // Dummy should not be in contacts
    auto contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("notAContact") == contacts.end());
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i == conversations.end());

    accInfo_.addConversation("notAContact");
    // TODO wait for signal from daemon
    contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("notAContact") == contacts.end());
    conversations = accInfo_.conversationModel.getFilteredConversations();
    i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i == conversations.end());

}

void
ConversationModelTester::testRmConversation()
{
    // Add conversation
    accInfo_.addConversation("dummy");
    // TODO wait for signal from daemon
    contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("dummy") != contacts.end());
    conversations = accInfo_.conversationModel.getFilteredConversations();
    i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i != conversations.end());

    // And remove it
    accInfo_.removeConversation("dummy");
    // TODO wait for signal from daemon
    auto contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.find("dummy") == contacts.end());
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [uid](const conversation::Info& conversation) {
      return conversation.uid == uid;
    });
    CPPUNIT_ASSERT(i == conversations.end());

}

void
ConversationModelTester::testFilterAndGetConversations()
{
    // If filter gives nothing, the getFilteredConversations should return a list with size == 0
    accInfo_.conversationModel.setFilter("YouShouldNotPass");
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    CPPUNIT_ASSERT_EQUAL(conversations.size(), 0);

    auto contacts = accInfo_.contactModel.getAllContacts();
    CPPUNIT_ASSERT(contacts.size() != 0); // the daemon should return some contacts
    auto contactUri = contacts.begin().first;
    accInfo_.conversationModel.setFilter(contactUri);
    conversations = accInfo_.conversationModel.getFilteredConversations();
    CPPUNIT_ASSERT_EQUAL(conversations.size(), 1); // We should see the contact
    auto firstConversation = accInfo_.conversationModel.getConversation(0);
    CPPUNIT_ASSERT_EQUAL(firstConversation.participants.front(), contactUri);

    auto nbContact = 0;
    for (const auto& contact: contacts) {
        if (contact.first.indexOf("contact") != -1)
            ++ nbContact;
    }
    accInfo_.conversationModel.setFilter("contact");
    conversations = accInfo_.conversationModel.getFilteredConversations();
    CPPUNIT_ASSERT_EQUAL(conversations.size(), nbContact);
    firstConversation = accInfo_.conversationModel.getConversation(0);
    CPPUNIT_ASSERT_EQUAL(firstConversation.participants.front(), contactUri);
}

void
ConversationModelTester::testSendMessageAndClearHistory()
{
    accInfo_.conversationModel.setFilter("");
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel.getConversation(0).uid;
    accInfo_.conversationModel.sendMessage(firstConversation, "Hello World!");
    // TODO wait for signal from database (messageAdded)
    // Then test clearHistory
    accInfo_.conversationModel.clearHistory(firstConversation);
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversation) {
            CPPUNIT_ASSERT_EQUAL(conversation.messages.size(), 1); // contains the CONTACT message.
        }
    }
}

void
ConversationModelTester::testPlaceCall()
{
    // TODO not implemented yet
}

void
ConversationModelTester::testCreateConference()
{
    // TODO not implemented yet
}

void
ConversationModelTester::testSelectConversation()
{
    accInfo_.conversationModel.setFilter("");
    auto conversations = accInfo_.conversationModel.getFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel.getConversation(0).uid;
    accInfo_.conversationModel.selectConversation(firstConversation);
    // TODO should emit openChatView
}

void
ConversationModelTester::tearDown()
{
    // Remove ring.db
    std::remove(ringDB);
}

} // namespace test
} // namespace ring

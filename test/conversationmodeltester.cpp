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
#include "conversationmodeltester.h"

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"

// Lrc
#include <api/newaccountmodel.h>
#include <api/conversationmodel.h>
#include <api/contact.h>
#include <api/contactmodel.h>
#include <api/newcallmodel.h>
#include <dbus/configurationmanager.h>
#include <namedirectory.h>

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(ConversationModelTester);

ConversationModelTester::ConversationModelTester()
: lrc_(new lrc::api::Lrc())
, accInfo_(lrc_->getAccountModel().getAccountInfo("ring0"))
{
    accInfo_.conversationModel->setFilter(lrc::api::profile::Type::RING);
}

void
ConversationModelTester::setUp()
{

}

void
ConversationModelTester::testAddValidConversation()
{
    // Dummy should not be in contacts
    auto contactId = accInfo_.contactModel->getContactProfileId("dummy");
    CPPUNIT_ASSERT(contactId.empty());
    // Search contact
    accInfo_.conversationModel->setFilter("dummy");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    // So, add dummy to contacts
    accInfo_.conversationModel->makePermanent("");
    auto contactAdded = WaitForSignalHelper(ConfigurationManager::instance(),
        SIGNAL(contactAdded(const QString&, const QString&, bool))).wait(1000);
    CPPUNIT_ASSERT_EQUAL(contactAdded, true);
    // Dummy should be in contacts
    contactId = accInfo_.contactModel->getContactProfileId("dummy");
    CPPUNIT_ASSERT(!contactId.empty());
    // So, a conversation should exists.
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         "dummy") != conversation.participants.end();
    });
    CPPUNIT_ASSERT(i != conversations.end());
}

void
ConversationModelTester::testAddInvalidConversation()
{
    // notAContact should not be in contacts
    auto contacts = accInfo_.contactModel->getAllContacts();
    CPPUNIT_ASSERT(contacts.find("notAContact") == contacts.end());
    // Search contact
    accInfo_.conversationModel->setFilter("notAContact");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    // Temporary item should contains "Searching...notAContact"
    // makePermanent should not do anything
    accInfo_.conversationModel->makePermanent("");
    WaitForSignalHelper(ConfigurationManager::instance(),
        SIGNAL(contactAdded(const QString&, const QString&, bool))).wait(1000);
    contacts = accInfo_.contactModel->getAllContacts();
    CPPUNIT_ASSERT(contacts.find("notAContact") == contacts.end());
}

void
ConversationModelTester::testRmConversation()
{
    // Dummy is already in conversations (added by testAddValidConversation)
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         "contact0") != conversation.participants.end();
    });
    CPPUNIT_ASSERT(i != conversations.end());
    accInfo_.conversationModel->removeConversation((*i).uid);
    auto conversationRemoved = WaitForSignalHelper(*accInfo_.conversationModel,
        SIGNAL(conversationRemoved(const std::string& uid))).wait(1000);
    CPPUNIT_ASSERT(conversationRemoved);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    i = std::find_if(conversations.begin(), conversations.end(),
    [](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         "contact0") != conversation.participants.end();
    });
    CPPUNIT_ASSERT(i == conversations.end());
}

void
ConversationModelTester::testFilterAndGetConversations()
{
    // If filter gives nothing, the allFilteredConversations should return a list with size == 1 (temporary item)
    accInfo_.conversationModel->setFilter("YouShouldNotPass");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT_EQUAL((int)conversations.size(), 1);
    // Count when filter exact name
    auto contacts = accInfo_.contactModel->getAllContacts();
    CPPUNIT_ASSERT(contacts.size() != 0); // the daemon should return some contacts
    auto contactUri = (*contacts.rbegin()).first;
    accInfo_.conversationModel->setFilter(contactUri);
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT_EQUAL((int)conversations.size(), 1); // We should see the contact
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT_EQUAL(firstConversation.participants.front(), contactUri);
    // Count all contacts
    auto nbContact = 0;
    for (const auto& contact: contacts)
        if (contact.first.find("contact") != std::string::npos)
            ++ nbContact;
    accInfo_.conversationModel->setFilter("contact");
    WaitForSignalHelper(*accInfo_.contactModel,
        SIGNAL(modelUpdated())).wait(1000);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT_EQUAL((int)conversations.size() - 1, nbContact);
}

void
ConversationModelTester::testSendMessageAndClearHistory()
{
    accInfo_.conversationModel->setFilter("");
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0).uid;
    accInfo_.conversationModel->sendMessage(firstConversation, "Hello World!");
    conversations = accInfo_.conversationModel->allFilteredConversations();
    auto conversationExists = false;
        for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversation) {
            conversationExists = true;
            // Should contains "Hello World!"
            CPPUNIT_ASSERT_EQUAL((int)conversation.interactions.size(), 1);
            CPPUNIT_ASSERT_EQUAL((*conversation.interactions.rbegin()).second.body, std::string("Hello World!"));
            break;
        }
    }
    CPPUNIT_ASSERT(conversationExists);
    auto unreadMessage = WaitForSignalHelper(*accInfo_.conversationModel,
        SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&))).wait(1000);
    CPPUNIT_ASSERT_EQUAL(unreadMessage, true);
    // Then test clearHistory
    accInfo_.conversationModel->clearHistory(firstConversation);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    conversationExists = false;
    for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversation) {
            conversationExists = true;
            CPPUNIT_ASSERT_EQUAL((int)conversation.interactions.size(), 0);
            break;
        }
    }
    CPPUNIT_ASSERT(conversationExists);
}

void
ConversationModelTester::testReceiveMessageAndSetRead()
{
    // Add a new message for the first conversation
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    QMap<QString, QString> payloads;
    payloads["text/plain"] = "This is not a message";
    ConfigurationManager::instance().emitIncomingAccountMessage(accInfo_.id.c_str(),
        firstConversation.participants.front().c_str(), payloads);
    auto unreadMessage = WaitForSignalHelper(*accInfo_.conversationModel,
        SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&))).wait(1000);
    CPPUNIT_ASSERT_EQUAL(unreadMessage, true);
    // This message should be unread
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto lastInteraction = *firstConversation.interactions.rbegin();
    CPPUNIT_ASSERT(lastInteraction.second.status == lrc::api::interaction::Status::UNREAD);
    accInfo_.conversationModel->setInteractionRead(firstConversation.uid, lastInteraction.first);
    // Now, the interaction should be READ
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT((*firstConversation.interactions.rbegin()).second.status == lrc::api::interaction::Status::READ);
}

void
ConversationModelTester::testPlaceCall()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT(firstConversation.callId.empty());
    accInfo_.conversationModel->placeCall(firstConversation.uid);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto newConv = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT_EQUAL(newConv.uid, firstConversation.uid);
    CPPUNIT_ASSERT(!newConv.callId.empty());
}

void
ConversationModelTester::testPlaceAudioOnlyCall()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT(firstConversation.callId.empty());
    accInfo_.conversationModel->placeAudioOnlyCall(firstConversation.uid);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto newConv = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT_EQUAL(newConv.uid, firstConversation.uid);
    CPPUNIT_ASSERT(!newConv.callId.empty());
}

void
ConversationModelTester::testCreateConference()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() > 1);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT(firstConversation.callId.empty());
    accInfo_.conversationModel->placeCall(firstConversation.uid);
    auto secondConversation = accInfo_.conversationModel->filteredConversation(1);
    CPPUNIT_ASSERT(secondConversation.callId.empty());
    accInfo_.conversationModel->placeCall(secondConversation.uid);

    // Calls should be dialogs
    conversations = accInfo_.conversationModel->allFilteredConversations();
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto firstCall = accInfo_.callModel->getCall(firstConversation.callId);
    CPPUNIT_ASSERT(firstCall.type == lrc::api::call::Type::DIALOG);
    secondConversation = accInfo_.conversationModel->filteredConversation(1);
    auto secondCall = accInfo_.callModel->getCall(secondConversation.callId);
    CPPUNIT_ASSERT(secondCall.type == lrc::api::call::Type::DIALOG);

    // Create conference
    accInfo_.conversationModel->joinConversations(firstConversation.uid, secondConversation.uid);
    auto callAddedToConf = WaitForSignalHelper(*accInfo_.callModel,
        SIGNAL(callAddedToConference(const std::string&, const std::string&))).wait(1000);
    CPPUNIT_ASSERT_EQUAL(callAddedToConf, true);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    firstCall = accInfo_.callModel->getCall(firstConversation.confId);
    CPPUNIT_ASSERT(firstCall.type == lrc::api::call::Type::CONFERENCE);
    secondConversation = accInfo_.conversationModel->filteredConversation(1);
    secondCall = accInfo_.callModel->getCall(secondConversation.confId);
    CPPUNIT_ASSERT(secondCall.type == lrc::api::call::Type::CONFERENCE);
    CPPUNIT_ASSERT(secondConversation.confId == firstConversation.confId);

}

void
ConversationModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

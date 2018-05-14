/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *  Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>
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
    CPPUNIT_ASSERT(!isAContact("dummy"));

    auto newContactUri = addToContacts("dummy");

    // Dummy should be in contacts
    CPPUNIT_ASSERT(isAContact("dummy"));

    // So, a conversation should exists.
    CPPUNIT_ASSERT(hasConversationWithContact("dummy"));
}

void
ConversationModelTester::testPlaceCallWithBannedContact()
{
    // badguy0 should not be in contacts
    CPPUNIT_ASSERT(!isAContact("badguy0"));

    // so, add him to contacts
    auto uri = addToContacts("badguy0");
    CPPUNIT_ASSERT(isAContact("badguy0"));

    // and ban him
    banContact(uri);
    auto contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // find conversation
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    auto conversation = std::find_if(conversations.begin(), conversations.end(),
    [&contactInfo](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         contactInfo.profileInfo.uri) != conversation.participants.end();
    });

    CPPUNIT_ASSERT(conversation != conversations.end());

    // now that badguy0 is banned, calling him should be forbidden
    auto baseInteractionsSize = conversation->interactions.size();
    accInfo_.conversationModel->placeCall(conversation->uid);

    // make sure call didn't succeed
    CPPUNIT_ASSERT(conversation->callId.empty());

    // unban badguy0
    unbanContact(uri);
    contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, false);

    // call again, should succeed
    accInfo_.conversationModel->placeCall(conversation->uid);

    // make sure call succeeded
    conversations = accInfo_.conversationModel->allFilteredConversations();
    conversation = std::find_if(conversations.begin(), conversations.end(),
    [&contactInfo](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         contactInfo.profileInfo.uri) != conversation.participants.end();
    });

    CPPUNIT_ASSERT(conversation != conversations.end());
    CPPUNIT_ASSERT(!conversation->callId.empty());
}

void
ConversationModelTester::testFilterBannedContact()
{
    // bannedContact should not be in contacts
    CPPUNIT_ASSERT(!isAContact("bannedContact"));
    CPPUNIT_ASSERT(!isAContact("bannedContacte"));
    CPPUNIT_ASSERT(!isAContact("bannedContac"));

    auto uri = addToContacts("bannedContact");

    // bannedContact now should be in contacts
    CPPUNIT_ASSERT(isAContact("bannedContact"));

    // ban bannedContact
    banContact(uri);
    auto contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // Make sure bannedContact doesn't appear is non-perfect-match filter searches
    // We expect 1 (temporary item)
    auto setFilter1SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContac");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter1SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(1, (int)accInfo_.conversationModel->allFilteredConversations().size());
    auto isTemporary = accInfo_.conversationModel->filteredConversation(0).participants.front() == "";
    CPPUNIT_ASSERT(isTemporary);
    auto setFilter2SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContacte");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter2SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(1, (int)accInfo_.conversationModel->allFilteredConversations().size());
    isTemporary = accInfo_.conversationModel->filteredConversation(0).participants.front() == "";
    CPPUNIT_ASSERT(isTemporary);

    // Make sure bannedContact appears in perfect-match filter searches
    // We expect 1 (bannedContact)
    auto setFilter3SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContact");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter3SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(1, (int)accInfo_.conversationModel->allFilteredConversations().size());
    isTemporary = accInfo_.conversationModel->filteredConversation(0).participants.front() == "";
    CPPUNIT_ASSERT(!isTemporary);

    // Unban bannedContact
    unbanContact(uri);
    contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, false);

    // Make sure bannedContact appears is non-perfect-match filter searches
    // We expect 2 (temporary item + bannedContact)
    auto setFilter4SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContac");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter4SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(2, (int)accInfo_.conversationModel->allFilteredConversations().size());

    // Here we expect 1 (temporary item)
    auto setFilter5SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContacte");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter5SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(1, (int)accInfo_.conversationModel->allFilteredConversations().size());

    // Make sure bannedContact appears in perfect-match filter searches
    // We expect 1 (bannedContact)
    auto setFilter6SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("bannedContact");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter6SigsCaught["modelUpdated"] > 0, true);
    CPPUNIT_ASSERT_EQUAL(1, (int)accInfo_.conversationModel->allFilteredConversations().size());

    isTemporary = accInfo_.conversationModel->filteredConversation(0).participants.front() == "";
    CPPUNIT_ASSERT(!isTemporary);
}

void
ConversationModelTester::testSendMessageToBannedContact()
{
    // badguy1 should not be in contacts
    CPPUNIT_ASSERT(!isAContact("badguy1"));

    auto uri = addToContacts("badguy1");

    // badguy1 should now be in contacts
    CPPUNIT_ASSERT(isAContact("badguy1"));

    // Ban badguy1
    banContact(uri);
    auto contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, true);

    // So, now that badguy is banned, sending a message should be forbidden
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    auto conversation = std::find_if(conversations.begin(), conversations.end(),
    [&contactInfo](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         contactInfo.profileInfo.uri) != conversation.participants.end();
    });

    CPPUNIT_ASSERT(conversation != conversations.end());

    // Try to send message to banned contact
    auto baseInteractionsSize = conversation->interactions.size();
    accInfo_.conversationModel->sendMessage(conversation->uid, "Hello banned !");
    // Make sure message didn't arrive (but contact added is already here)
    CPPUNIT_ASSERT_EQUAL((int)baseInteractionsSize, (int)conversation->interactions.size());

    // Unban badguy1
    unbanContact(uri);
    contactInfo = accInfo_.contactModel->getContact(uri);
    CPPUNIT_ASSERT_EQUAL(contactInfo.isBanned, false);

    // Now send message again, should succeed
    accInfo_.conversationModel->sendMessage(conversation->uid, "Hello unbanned !");

    // Make sure message arrived
    conversations = accInfo_.conversationModel->allFilteredConversations();
    conversation = std::find_if(conversations.begin(), conversations.end(),
    [&contactInfo](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         contactInfo.profileInfo.uri) != conversation.participants.end();
    });

    CPPUNIT_ASSERT(conversation != conversations.end());

    CPPUNIT_ASSERT_EQUAL((int)baseInteractionsSize + 1, (int)conversation->interactions.size());
}

void
ConversationModelTester::testAddInvalidConversation()
{
    // notAContact should not be in contacts
    CPPUNIT_ASSERT(!isAContact("notAContact"));

    // Search contact
    auto setFilterSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("notAContact");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilterSigsCaught["modelUpdated"] > 0, true);

    // Temporary item should contain "Searching...notAContact"
    // makePermanent should not do anything
    auto makePermanentSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->makePermanent("");
        })
        .addSignal("contactAdded", ConfigurationManager::instance(), SIGNAL(contactAdded(const QString&, const QString&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(makePermanentSigsCaught["contactAdded"] > 0, true);

    CPPUNIT_ASSERT(!isAContact("notAContact"));
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
    auto removeConversationSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->removeConversation((*i).uid);
        })
        .addSignal("conversationRemoved", *accInfo_.conversationModel, SIGNAL(conversationRemoved(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(removeConversationSigsCaught["conversationRemoved"] > 0, true);
    CPPUNIT_ASSERT(!hasConversationWithContact("contact0"));
}

void
ConversationModelTester::testFilterAndGetConversations()
{
    // If filter gives nothing, the allFilteredConversations should return a list with size == 1 (temporary item)
    auto setFilter1SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("YouShouldNotPass");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter1SigsCaught["modelUpdated"] > 0, true);

    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT_EQUAL((int)conversations.size(), 1);

    // Count when filter exact name
    auto contacts = accInfo_.contactModel->getAllContacts();
    CPPUNIT_ASSERT(contacts.size() != 0); // the daemon should return some contacts
    auto contactUri = (*contacts.rbegin()).first;
    auto setFilter2SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter(contactUri);
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter2SigsCaught["modelUpdated"] > 0, true);
    conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT_EQUAL((int)conversations.size(), 1); // We should see the contact
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT_EQUAL(firstConversation.participants.front(), contactUri);

    // Count all contacts
    auto nbContact = 0;
    for (const auto& contact: contacts) {
        if (contact.first.find("contact") != std::string::npos) {
            ++ nbContact;
        }
    }

    auto setFilter3SigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter("contact");
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilter3SigsCaught["modelUpdated"] > 0, true);

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

    auto sendMessageSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->sendMessage(firstConversation, "Hello World!");
        })
        .addSignal("newUnreadMessage", *accInfo_.conversationModel, SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(sendMessageSigsCaught["newUnreadMessage"] > 0, true);

    conversations = accInfo_.conversationModel->allFilteredConversations();
    auto conversationExists = false;
    for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversation) {
            conversationExists = true;
            // Should contains "Contact Added" + "Hello World!"
            CPPUNIT_ASSERT_EQUAL((int)conversation.interactions.size(), 2);
            CPPUNIT_ASSERT_EQUAL((*conversation.interactions.rbegin()).second.body, std::string("Hello World!"));
            break;
        }
    }
    CPPUNIT_ASSERT(conversationExists);

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
ConversationModelTester::testSendMessagesAndClearInteraction()
{
    accInfo_.conversationModel->setFilter("");
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto firstConversationUid = firstConversation.uid;

    // HACK reinit the conversation here (without these line, Hello World! will not be in interactions)
    // FIXME
    accInfo_.conversationModel->clearHistory(firstConversationUid);
    firstConversation = accInfo_.conversationModel->filteredConversation(0);

    // Send 3 messages (will be added to conversation.interactions)
    int baseInteractionsSize = firstConversation.interactions.size();
    accInfo_.conversationModel->sendMessage(firstConversationUid, "Hello World!");
    accInfo_.conversationModel->sendMessage(firstConversationUid, "It's been a long time");
    accInfo_.conversationModel->sendMessage(firstConversationUid, "How have you been?");

    conversations = accInfo_.conversationModel->allFilteredConversations();
    auto conversationExists = false;
    uint64_t secondInterId = {};
    for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversationUid) {
            conversationExists = true;
            CPPUNIT_ASSERT_EQUAL((int)conversation.interactions.size(), baseInteractionsSize + 3);
            auto it = conversation.interactions.begin();
            it++;
            secondInterId = it->first;
            break;
        }
    }
    CPPUNIT_ASSERT(conversationExists);

    auto clearInteractionFromConversationSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->clearInteractionFromConversation(firstConversationUid, secondInterId);
        })
        .addSignal("interactionRemoved", *accInfo_.conversationModel, SIGNAL(interactionRemoved(const std::string&, uint64_t)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(clearInteractionFromConversationSigsCaught["interactionRemoved"] > 0, true);

    conversations = accInfo_.conversationModel->allFilteredConversations();
    conversationExists = false;
    for (const auto& conversation: conversations) {
        if (conversation.uid == firstConversationUid) {
            conversationExists = true;
            // Second interaction should be removed
            CPPUNIT_ASSERT_EQUAL((int)conversation.interactions.size(), baseInteractionsSize + 2);
            for (const auto& interaction: conversation.interactions)
                CPPUNIT_ASSERT(interaction.first != secondInterId);
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

    auto incomingAccountMessageSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitIncomingAccountMessage(accInfo_.id.c_str(), firstConversation.participants.front().c_str(), payloads);
        })
        .addSignal("newUnreadMessage", *accInfo_.conversationModel, SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(incomingAccountMessageSigsCaught["newUnreadMessage"] > 0, true);

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
    // Get first conversation and make sure it is empty
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    CPPUNIT_ASSERT(firstConversation.callId.empty());

    // Place a call
    accInfo_.conversationModel->placeCall(firstConversation.uid);

    // Get first conversation again and make sure it isn't empty anymore (call suceeded)
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
    auto joinConversationsSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->joinConversations(firstConversation.uid, secondConversation.uid);
        })
        .addSignal("callAddedToConference", *accInfo_.conversationModel, SIGNAL(callAddedToConference(const std::string&, const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(joinConversationsSigsCaught["callAddedToConference"] > 0, true);

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
ConversationModelTester::testClearUnreadInteractions()
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    CPPUNIT_ASSERT(conversations.size() != 0);
    auto firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto sender = firstConversation.participants.front().c_str();

    QMap<QString, QString> payloads;

    // Send a first message
    payloads["text/plain"] = "This is not a message";
    auto incomingAccountMessageSigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitIncomingAccountMessage(accInfo_.id.c_str(), sender, payloads);
        })
        .addSignal("newUnreadMessage", *accInfo_.conversationModel, SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(incomingAccountMessageSigsCaught["newUnreadMessage"] > 0, true);

    // Send a second message
    auto incomingAccountMessage2SigsCaught = WaitForSignalHelper([&]() {
            ConfigurationManager::instance().emitIncomingAccountMessage(accInfo_.id.c_str(), sender, payloads);
        })
        .addSignal("newUnreadMessage", *accInfo_.conversationModel, SIGNAL(newUnreadMessage(const std::string&, uint64_t, const interaction::Info&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(incomingAccountMessage2SigsCaught["newUnreadMessage"] > 0, true);

    // Make sure both messages are unread
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    auto interactions = firstConversation.interactions.rbegin();
    CPPUNIT_ASSERT(interactions->second.status == lrc::api::interaction::Status::UNREAD);
    CPPUNIT_ASSERT((++interactions)->second.status == lrc::api::interaction::Status::UNREAD);

    // Clear conversation of unread interactions
    auto clearUnreadInteractionsSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->clearUnreadInteractions(firstConversation.uid);
        })
        .addSignal("conversationUpdated", *accInfo_.conversationModel, SIGNAL(conversationUpdated(const std::string&)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(clearUnreadInteractionsSigsCaught["conversationUpdated"] > 0, true);

    // Now make sure both messages are read
    firstConversation = accInfo_.conversationModel->filteredConversation(0);
    interactions = firstConversation.interactions.rbegin();
    CPPUNIT_ASSERT(interactions->second.status == lrc::api::interaction::Status::READ);
    CPPUNIT_ASSERT((++interactions)->second.status == lrc::api::interaction::Status::READ);
}

void
ConversationModelTester::tearDown()
{
}

bool
ConversationModelTester::hasConversationWithContact(const std::string& uri)
{
    auto conversations = accInfo_.conversationModel->allFilteredConversations();
    auto i = std::find_if(conversations.begin(), conversations.end(),
    [&uri](const lrc::api::conversation::Info& conversation) {
        return std::find(conversation.participants.begin(),
                         conversation.participants.end(),
                         uri) != conversation.participants.end();
    });
    return i != conversations.end();
}

void
ConversationModelTester::banContact(const std::string& uri)
{
    auto banContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->removeContact(uri, true);
        })
        .addSignal("filterChanged", ConfigurationManager::instance(), SIGNAL(lrc::api::ConversationModel::filterChanged()))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(banContactSigsCaught["filterChanged"] > 0, true);
}

void
ConversationModelTester::unbanContact(const std::string& uri)
{
    auto contactInfo = accInfo_.contactModel->getContact(uri);
    auto unbanContactSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.contactModel->addContact(contactInfo);
        })
        .addSignal("filterChanged", ConfigurationManager::instance(), SIGNAL(lrc::api::ConversationModel::filterChanged()))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(unbanContactSigsCaught["filterChanged"] > 0, true);
}

bool
ConversationModelTester::isAContact(const std::string& uri)
{
    return !accInfo_.contactModel->getContactProfileId(uri).empty();
}

std::string
ConversationModelTester::addToContacts(const std::string& username)
{
    // Search contact
    auto setFilterSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->setFilter(username);
        })
        .addSignal("modelUpdated", *accInfo_.contactModel, SIGNAL(modelUpdated(const std::string&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(setFilterSigsCaught["modelUpdated"] > 0, true);

    // Add to contacts
    auto uri = accInfo_.conversationModel->owner.contactModel->getContact("").profileInfo.uri;
    auto makePermanentSigsCaught = WaitForSignalHelper([&]() {
            accInfo_.conversationModel->makePermanent(uri);
        })
        .addSignal("contactAdded", ConfigurationManager::instance(), SIGNAL(contactAdded(const QString&, const QString&, bool)))
        .wait(500);
    CPPUNIT_ASSERT_EQUAL(makePermanentSigsCaught["contactAdded"] > 0, true);

    return uri;
}

} // namespace test
} // namespace ring

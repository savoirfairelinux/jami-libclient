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
#include "databasetester.h"

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

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTester);

void
DatabaseTester::setUp()
{
    // Remove ring.db
    std::remove(lrc::Database::ringDB);
    // Init database
    database_ = std::unique_ptr<lrc::Database>(new lrc::Database());
}

void
DatabaseTester::testAddAndClearMessage()
{
    const auto accountId = "account0";
    const auto contactUri = "contact0";
    lrc::api::message::Info msg;
    msg.contact = contactUri;
    msg.body = "hello";
    msg.type = lrc::api::message::Type::TEXT;
    msg.status = lrc::api::message::Status::SENDING;

    database_->clearHistory(accountId, contactUri);
    CPPUNIT_ASSERT_EQUAL(findMessage(msg, accountId, contactUri), false);
    database_->addMessage(accountId, msg);
    CPPUNIT_ASSERT_EQUAL(findMessage(msg, accountId, contactUri), true);
    database_->clearHistory(accountId, contactUri);
    CPPUNIT_ASSERT_EQUAL(findMessage(msg, accountId, contactUri), false);
}

void
DatabaseTester::testGetHistory()
{
    const auto accountId = "account0";
    const auto contactUri = "contact0";
    lrc::api::message::Info msg;
    msg.contact = contactUri;
    msg.body = "hello";
    msg.type = lrc::api::message::Type::TEXT;
    msg.status = lrc::api::message::Status::SENDING;

    auto messages = database_->getHistory(accountId, contactUri);
    auto baseSize = messages.size();
    database_->addMessage(accountId, msg);
    database_->addMessage(accountId, msg);
    CPPUNIT_ASSERT_EQUAL(findMessage(msg, accountId, contactUri), true);
    messages = database_->getHistory(accountId, contactUri);
    CPPUNIT_ASSERT_EQUAL(baseSize + 2, messages.size());
}

void
DatabaseTester::testUnread()
{
    const auto accountId = "account0";
    const auto contactUri = "contact0";
    lrc::api::message::Info msg;
    msg.contact = contactUri;
    msg.body = "hello";
    msg.type = lrc::api::message::Type::TEXT;
    msg.status = lrc::api::message::Status::SENDING;

    database_->clearHistory(accountId, contactUri);
    CPPUNIT_ASSERT(database_->numberOfUnreads(accountId, contactUri) == 0);
    database_->addMessage(accountId, msg);
    CPPUNIT_ASSERT(database_->numberOfUnreads(accountId, contactUri) == 1);
    auto messages = database_->getHistory(accountId, contactUri);
    CPPUNIT_ASSERT(1 == messages.size());
    database_->setMessageRead(messages.begin()->first);
    CPPUNIT_ASSERT(database_->numberOfUnreads(accountId, contactUri) == 0);
}

void
DatabaseTester::testContact()
{
    // add Contact
    const auto contactUri = "contact0";
    const auto vcard =
    "BEGIN:VCARD\
    VERSION:2.1\
    UID:contact0\
    FN:contact\
    PHOTO;ENCODING=BASE64;TYPE=PNG:PHOTO\
    END:VCARD%";
    database_->addContact(contactUri, vcard);
    // TODO mock NameDirectory::instance().lookupName(account, QString(), contact_id);
    // TODO test getContactAttribute
}

void
DatabaseTester::tearDown()
{
    // Remove ring.db
    std::remove(lrc::Database::ringDB);
}

bool
DatabaseTester::findMessage(const lrc::api::message::Info& msg,
                            const std::string& accountId,
                            const std::string& contactUri) const
{
    auto messages = database_->getHistory(accountId, contactUri);
    for (const auto item : messages) {
        const auto message = item.second;
        if (message.contact == msg.contact && msg.body == message.body
        && msg.timestamp == message.timestamp && msg.type == message.type &&
        message.status == msg.status) {
            return true;
        }
    }
    return false;
}

} // namespace test
} // namespace ring

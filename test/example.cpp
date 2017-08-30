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
#include "example.h"

// Qt
#include <QString>

// Lrc
#include "dbus/configurationmanager.h"

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(ExampleTest);

void
ExampleTest::setUp()
{
    // NOTE: here we can create some accounts, contacts, etc
    // NOTE: We can also recreate ring.db
    lrc_ = std::unique_ptr<lrc::Lrc>(new lrc::Lrc());
}

void
ExampleTest::test()
{
    QMap<QString, QString> payloads;
    payloads["text/plain"] ="from test";
    ConfigurationManager::instance().emitIncomingAccountMessage(QString("0000"),
    QString("aaaaa"), payloads);
    CPPUNIT_ASSERT_EQUAL(1, 1);
}

} // namespace test
} // namespace ring

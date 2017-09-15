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
#include <map>

// Qt
#include <QString>
#include <QStandardPaths>

// Lrc
#include "dbus/configurationmanager.h"

namespace lrc
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTester);

void
DatabaseTester::setUp()
{
    // Remove ring.db
    auto dbPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/ring.db";
    dbPath_ = dbPath.toStdString();
    std::remove(dbPath_.c_str());
    // Init database
    database_ = std::unique_ptr<lrc::Database>(new lrc::Database());
}

void
DatabaseTester::tearDown()
{
    // Remove ring.db
    std::remove(dbPath_.c_str());
}

void
DatabaseTester::testInsertAndSelectCorrectValue()
{
    // Insert a new profile
    auto table = "profiles";
    auto bindsCol = std::map<std::string, std::string>();
    bindsCol.emplace(":uri", "uri");
    bindsCol.emplace(":alias", "alias");
    bindsCol.emplace(":photo", "photo");
    bindsCol.emplace(":type", "type");
    bindsCol.emplace(":status", "status");
    auto bindsSet = std::map<std::string, std::string>();
    bindsSet.emplace(":uri", "glados");
    bindsSet.emplace(":alias", "GLaDOs");
    bindsSet.emplace(":photo", "");
    bindsSet.emplace(":type", "0");
    bindsSet.emplace(":status", "0");
    auto result = database_->insertInto(table, bindsCol, bindsSet);
    // The query should succeed
    CPPUNIT_ASSERT(result != -1);

    auto col = "alias";
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "glados");
    auto selectResult = database_->select(col, table, whereCond, bindsWhere);
    CPPUNIT_ASSERT(selectResult.nbrOfCols == 1);
    CPPUNIT_ASSERT(selectResult.payloads[0] == "GLaDOs");
}

void
DatabaseTester::testInsertIncorrectFail()
{
    // Insert a new profile
    try {
        auto table = "profiles";
        auto bindsCol = std::map<std::string, std::string>();
        bindsCol.emplace(":NOTACOL", "NotACol");
        bindsCol.emplace(":alias", "alias");
        bindsCol.emplace(":photo", "photo");
        bindsCol.emplace(":type", "type");
        bindsCol.emplace(":status", "status");
        auto bindsSet = std::map<std::string, std::string>();
        bindsSet.emplace(":uri", "0000");
        bindsSet.emplace(":alias", "ALIAS");
        bindsSet.emplace(":photo", "0000");
        bindsSet.emplace(":type", "0");
        bindsSet.emplace(":status", "0");
        database_->insertInto(table, bindsCol, bindsSet);
        CPPUNIT_ASSERT_EQUAL(0, 1);
    } catch (lrc::Database::QueryInsertError& e) {
        CPPUNIT_ASSERT(e.details().size() > 0);
    }
}

void
DatabaseTester::testSelectInexistantValue()
{
    // Insert a new profile
    auto table = "profiles";
    auto col = "alias";
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "schrödinger");
    auto selectResult = database_->select(col, table, whereCond, bindsWhere);
    CPPUNIT_ASSERT(selectResult.nbrOfCols == 1);
    CPPUNIT_ASSERT(selectResult.payloads.size() == 0);
}

void
DatabaseTester::testUpdateCorrectValue()
{
    // Insert a new profile
    auto table = "profiles";
    auto bindsCol = std::map<std::string, std::string>();
    bindsCol.emplace(":uri", "uri");
    bindsCol.emplace(":alias", "alias");
    bindsCol.emplace(":photo", "photo");
    bindsCol.emplace(":type", "type");
    bindsCol.emplace(":status", "status");
    auto bindsSet = std::map<std::string, std::string>();
    bindsSet.emplace(":uri", "weasley");
    bindsSet.emplace(":alias", "good core");
    bindsSet.emplace(":photo", "");
    bindsSet.emplace(":type", "0");
    bindsSet.emplace(":status", "0");
    auto result = database_->insertInto(table, bindsCol, bindsSet);
    // The query should succeed
    CPPUNIT_ASSERT(result != -1);

    // Update value
    auto col = "alias=:alias";
    bindsCol = std::map<std::string, std::string>();
    bindsCol.emplace(":alias", "bad core");
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "weasley");
    database_->update(table, col, bindsCol, whereCond, bindsWhere);

    // And select it
    col = "alias";
    auto selectResult = database_->select(col, table, whereCond, bindsWhere);
    CPPUNIT_ASSERT(selectResult.nbrOfCols == 1);
    CPPUNIT_ASSERT(selectResult.payloads[0] == "bad core");
}

void
DatabaseTester::testUpdateInexistantValue()
{
    // Update an incorrect profile
    auto table = "profiles";
    auto bindsCol = std::map<std::string, std::string>();
    bindsCol.emplace(":alias", "schrödinger");
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "schrödinger");
    database_->update(table, "alias=:alias", bindsCol, whereCond, bindsWhere);
    // And try to find this profile
    auto selectResult = database_->select("alias", table, whereCond, bindsWhere);
    CPPUNIT_ASSERT(selectResult.nbrOfCols == 1);
    CPPUNIT_ASSERT(selectResult.payloads.size() == 0);
}

void
DatabaseTester::testDeleteCorrectValue()
{
    // Insert a new profile
    auto table = "profiles";
    auto bindsCol = std::map<std::string, std::string>();
    bindsCol.emplace(":uri", "uri");
    bindsCol.emplace(":alias", "alias");
    bindsCol.emplace(":photo", "photo");
    bindsCol.emplace(":type", "type");
    bindsCol.emplace(":status", "status");
    auto bindsSet = std::map<std::string, std::string>();
    bindsSet.emplace(":uri", "pbody");
    bindsSet.emplace(":alias", "P-Body");
    bindsSet.emplace(":photo", "");
    bindsSet.emplace(":type", "0");
    bindsSet.emplace(":status", "0");
    auto result = database_->insertInto(table, bindsCol, bindsSet);
    // The query should succeed
    CPPUNIT_ASSERT(result != -1);

    // Delete the profile
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "pbody");
    database_->deleteFrom(table, whereCond, bindsWhere);

    // And try to find this profile
    auto col = "alias";
    auto selectResult = database_->select(col, table, whereCond, bindsWhere);
    CPPUNIT_ASSERT(selectResult.nbrOfCols == 1);
    CPPUNIT_ASSERT(selectResult.payloads.size() == 0);
}

void
DatabaseTester::testDeleteInexistantValue()
{
    auto table = "profiles";
    auto whereCond = "uri=:uri";
    auto bindsWhere = std::map<std::string, std::string>();
    bindsWhere.emplace(":uri", "schrödinger");
    database_->deleteFrom(table, whereCond, bindsWhere);
    // Should not throw anything if fails
}

} // namespace test
} // namespace ring

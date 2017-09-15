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
#include "database.h"

namespace lrc
{
namespace test
{

class DatabaseTester : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE(DatabaseTester);
    CPPUNIT_TEST(testInsertAndSelectCorrectValue);
    CPPUNIT_TEST(testInsertIncorrectFail);
    CPPUNIT_TEST(testSelectInexistantValue);
    CPPUNIT_TEST(testUpdateCorrectValue);
    CPPUNIT_TEST(testUpdateInexistantValue);
    CPPUNIT_TEST(testDeleteCorrectValue);
    CPPUNIT_TEST(testDeleteInexistantValue);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    /**
     * Insert a correct value and try to retrieve it
     */
    void testInsertAndSelectCorrectValue();
    /**
     * Insert incorrect value
     */
    void testInsertIncorrectFail();
    /**
     * Select inexistant value
     */
    void testSelectInexistantValue();
    /**
     * Update value
     */
    void testUpdateCorrectValue();
    /**
     * Update inexistant value
     */
    void testUpdateInexistantValue();
    /**
     * Delete value
     */
    void testDeleteCorrectValue();
    /**
     * Delete inexistant value
     */
    void testDeleteInexistantValue();

protected:
    std::unique_ptr<lrc::Database> database_;
    std::string dbPath_;
};

} // namespace test
} // namespace lrc

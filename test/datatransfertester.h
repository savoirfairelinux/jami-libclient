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

class DataTransferTester : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(DataTransferTester);
    CPPUNIT_TEST(testReceivesMusic);
    CPPUNIT_TEST(testReceivesImage5MbNoPref);
    CPPUNIT_TEST(testReceivesImage5Mb);
    CPPUNIT_TEST_SUITE_END();

public:
    DataTransferTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Receives a new file, should be in conversations
     */
    void testReceivesMusic();
    /**
     * Receives a new image without any prefedred dir, should be awaiting peer
     */
    void testReceivesImage5MbNoPref();
    /**
     * Receives a new image with a prefedred dir, should accept tranfer
     */
    void testReceivesImage5Mb();
    /**
     * Receives a new image with a prefedred dir but too big, awaiting peer
     */
    void testReceivesImage50Mb();
    /**
     * Method automatically called after each test by CppUnit
     */
    void tearDown();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
    const lrc::api::account::Info& accInfo_;
};

} // namespace test
} // namespace ring

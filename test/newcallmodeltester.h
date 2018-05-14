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

class NewCallModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NewCallModelTester);
//    CPPUNIT_TEST(testCreateAndGetCall);
//    CPPUNIT_TEST(testAcceptHoldUnholdHangupCall);
//    CPPUNIT_TEST(testCreateAndGetAudioOnlyCall);
    CPPUNIT_TEST_SUITE_END();

public:
    NewCallModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Create a call between "ring2" and "contact0" and retrieve it.
     */
    void testCreateAndGetCall();
    /**
     * Create a audio-only call between "ring2" and "contact0" and retrieve it.
     */
    void testCreateAndGetAudioOnlyCall();
    /**
     * Simulate an incoming call from "contact1" for "ring2"
     * Accept the call, call status should be IN_PROGRESS
     * Hold the call, call status should be HOLD
     * Unhold the call, call status should be IN_PROGRESS
     * then hangUp.
     */
    void testAcceptHoldUnholdHangupCall();
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

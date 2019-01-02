/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#pragma once

// cppunit
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// std
#include <memory>

// Qt
#include <QObject>

namespace lrc
{
namespace api
{
class Lrc;
}
}

namespace ring
{
namespace test
{

class NewAccountModelTester :  public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(NewAccountModelTester);
    CPPUNIT_TEST_SUITE_END();

public:
    NewAccountModelTester();
    /**
     * Method automatically called before each test by CppUnit
     */
    void setUp();
    /**
     * Method automatically called after each test CppUnit
     */
    void tearDown();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
};

} // namespace test
} // namespace ring

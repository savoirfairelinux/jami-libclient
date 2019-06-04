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
#include "newaccountmodeltester.h"

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"
#include "utils/daemon_connector.h"

// lrc
#include "api/lrc.h"

namespace ring
{
namespace test
{

CPPUNIT_TEST_SUITE_REGISTRATION(NewAccountModelTester);

NewAccountModelTester::NewAccountModelTester()
: lrc_(new lrc::api::Lrc())
{

}

void
NewAccountModelTester::setUp()
{
    auto daemon = std::make_unique<Daemon>();
}


void
NewAccountModelTester::tearDown()
{

}

void
NewAccountModelTester::testX()
{

}


} // namespace test
} // namespace ring

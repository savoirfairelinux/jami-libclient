#include "newaccountmodeltester.h"

// Qt
#include <QString>
#include "utils/waitforsignalhelper.h"

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

}


void
NewAccountModelTester::tearDown()
{

}

} // namespace test
} // namespace ring

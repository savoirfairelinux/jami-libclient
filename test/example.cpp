#include "example.h"

#include <iostream>

namespace ring { namespace test {

CPPUNIT_TEST_SUITE_REGISTRATION(ExampleTest);

void
ExampleTest::test()
{
    std::cout << "Hello Ring!" << std::endl;
    CPPUNIT_ASSERT_EQUAL(1, 1);
}
}} // namespace ring::test

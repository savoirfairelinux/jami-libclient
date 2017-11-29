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
     * Get all contacts for account "ring1".
     * Contacts are defined in configurationmanager_mock.h
     */
    void testZero();
    /**
     * Method automatically called after each test CppUnit
     */
    void tearDown();

protected:
    std::unique_ptr<lrc::api::Lrc> lrc_;
};

} // namespace test
} // namespace ring

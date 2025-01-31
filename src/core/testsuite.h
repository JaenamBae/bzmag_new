#ifndef BZMAG_CORE_UTILITY_UNITTEST_TESTSUITE_H
#define BZMAG_CORE_UTILITY_UNITTEST_TESTSUITE_H
/**
    @ingroup bzmagCoreUnitTest
    @class bzmag::TestSuite
    @brief 
*/

#pragma warning(disable: 4251) // C4251 경고 무시

#include <list>
#include "platform.h"

namespace bzmag
{
    class TestCaseBase;
    class BZMAG_LIBRARY_API TestSuite
    {
    public:
        virtual~TestSuite();

        template <typename T>
        void addTestCase();
        void run();

    private:
        typedef std::list<TestCaseBase*> TestCases;

    private:
        TestCases testCases_;
    };

#include "testsuite.inl"

}

#endif // BZMAG_CORE_UTILITY_UNITTEST_TESTSUITE_H

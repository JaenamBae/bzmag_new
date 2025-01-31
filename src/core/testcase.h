#ifndef BZMAG_CORE_UTILITY_UNITTEST_TESTCASE_H
#define BZMAG_CORE_UTILITY_UNITTEST_TESTCASE_H
/**
    @ingroup bzmagCoreUnitTest
    @class bzmag::TestCase
    @brief 
*/

#include <list>
#include "platform.h"
#include "define.h"
#include "string.h"
#include "currentsnapshot.h"
#include "testcallerbase.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API TestCaseBase
    {
    public:
        struct Error
        {
            TestCallerBase* testCaller_;
            CurrentSnapShot snapShot_;
        };
        typedef std::list<Error> Errors;

    public:
        virtual~TestCaseBase() {}

        virtual void initialize() {}
        virtual void finalize() {}
        virtual void run() {}

        virtual void terminate()=0;
        virtual const char* toString() const=0;
        virtual int getNumTests() const=0;
        Errors& getErrors();

    protected:
        Errors errors_;
    };

    template <typename T>
    class TestCase : public TestCaseBase
    {
    public:
        typedef void (T::*TestMethod)();

    public:
        virtual~TestCase();

        void run();

        virtual void terminate();
        virtual int getNumTests() const;

    private:
        typedef std::list<TestCallerBase*> TestCallers;

    protected:
        TestCallers testCallers_;
    };

#include "testcase.inl"

#define BZMAGUNIT_ASSERT(e) do { if (!(e)) { \
    throw CurrentSnapShot("assert",\
    ___DATE___, ___TIME___, ___FILE___, ___FUNCTION___,\
    ___FUNCTION___, ___LINE___, #e);\
    } } while (0);

}

#endif // BZMAG_CORE_UTILITY_UNITTEST_TESTCASE_H

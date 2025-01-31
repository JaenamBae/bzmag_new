#ifndef BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H
#define BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H
/**
    @ingroup bzmagCoreUnitTest
    @class bzmag::TestCaller
    @brief 
*/

#include "testcase.h"
#include "testcallerbase.h"

namespace bzmag
{
    template <typename T>
    class TestCaller : public TestCallerBase
    {
    public:
        typedef typename TestCase<T> MyTestCase;
        typedef typename MyTestCase::TestMethod MyTestMethod;

    public:
        TestCaller(
            MyTestCase* test_case,
            MyTestMethod test_method,
            const char* name);
        virtual~TestCaller();

        virtual void call();
        virtual const String& getName() const;

    private:
        String name_;
        MyTestCase* testCase_;
        MyTestMethod testMethod_;
    };

#include "testcaller.inl"

#define BZMAGUNIT_BEGINE_TESTSUITE(name) typedef name MyTestCase;\
    typedef bzmag::TestCaller<name> MyTestCaller;\
    void addTest(MyTestCaller* test) { testCallers_.push_back(test); }\
    virtual const char* toString() const { return #name; }\
    name() {

#define BZMAGUNIT_TEST(name) addTest(\
    new MyTestCaller(this, &MyTestCase::name, #name));

#define BZMAGUNIT_END_TESTSUITE() }

}

#endif // BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H

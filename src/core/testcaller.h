#ifndef BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H
#define BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H

#include "testcase.h"
#include "testcallerbase.h"

namespace bzmag
{
    template <typename T>
    class TestCaller : public TestCallerBase
    {
    public:
        typedef TestCase<T> MyTestCase;
        typedef typename MyTestCase::TestMethod MyTestMethod;  // typename 추가

    public:
        TestCaller(
            MyTestCase* test_case,
            typename MyTestCase::TestMethod test_method,  // typename 추가
            const char* name);
        virtual ~TestCaller();

        virtual void call();
        virtual const String& getName() const;

    private:
        String name_;
        MyTestCase* testCase_;
        typename MyTestCase::TestMethod testMethod_;  // typename 추가
    };

#include "testcaller.inl"

#define BZMAGUNIT_BEGIN_TESTSUITE(name) typedef name MyTestCase; \
    typedef bzmag::TestCaller<name> MyTestCaller; \
    void addTest(MyTestCaller* test) { testCallers_.push_back(test); } \
    virtual const char* toString() const { return #name; } \
    public: name() {  // public 추가

#define BZMAGUNIT_TEST(name) addTest( \
    new MyTestCaller(this, &MyTestCase::name, #name));

#define BZMAGUNIT_END_TESTSUITE() }

}

#endif // BZMAG_CORE_UTILITY_UNITTEST_TESTCALLER_H

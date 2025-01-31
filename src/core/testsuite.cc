#include "testsuite.h"

#include "testcase.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
TestSuite::~TestSuite()
{
    for (TestCases::iterator iter = testCases_.begin();
         iter != testCases_.end(); ++iter)
    {
        (*iter)->terminate();
        delete (*iter);
    }
}


//-----------------------------------------------------------------------------
void TestSuite::run()
{
    int num_testcases = 0;
    int num_tests = 0;

    class Error
    {
    public:
        String testCaseName_;
        TestCallerBase* testCaller_;
        CurrentSnapShot snapShot_;
    };
    typedef std::list<Error> Errors;
    Errors errors;

    for (TestCases::iterator iter = testCases_.begin();
         iter != testCases_.end(); ++iter)
    {
        TestCaseBase* test_case = *iter;
        test_case->initialize();
        test_case->run();
        test_case->finalize();
        ++num_testcases;
        num_tests += test_case->getNumTests();

        for (TestCaseBase::Errors::iterator error_iter =
             test_case->getErrors().begin();
             error_iter != test_case->getErrors().end(); ++error_iter)
        {
            Error error;
            error.testCaseName_ = test_case->toString();
            error.testCaller_ = error_iter->testCaller_;
            error.snapShot_ = error_iter->snapShot_;
            errors.push_back(error);
        }
    }

    printf("\n\n");
    if (errors.empty())
    {
        printf("OK (%d)\n", num_tests);
    }
    else
    {
        for (Errors::iterator iter = errors.begin();
             iter != errors.end(); ++iter)
        {
            printf("%s Failed:\n", iter->testCaseName_.c_str());
            printf("%s(%d):\n",
                iter->snapShot_.fileName_.c_str(), 
                iter->snapShot_.line_);
            printf("    %s:\n", iter->testCaller_->getName().c_str());
            printf("        %s\n", iter->snapShot_.expression_.c_str());
            printf("\n");
        }
    }
}

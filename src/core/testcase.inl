//-----------------------------------------------------------------------------
inline TestCaseBase::Errors& TestCaseBase::getErrors()
{
    return errors_;
}

//-----------------------------------------------------------------------------
template <typename T>
TestCase<T>::~TestCase()
{
    terminate();    
}


//-----------------------------------------------------------------------------
template <typename T>
void TestCase<T>::run()
{
    for (TestCallers::iterator iter = testCallers_.begin();
         iter != testCallers_.end(); ++iter)
    {
        TestCallerBase* test_caller = *iter;
        try
        {
            test_caller->call();
            printf(".");
        }
        catch (CurrentSnapShot& snap_shot)
        {
            printf("X");
            Error error;
            error.testCaller_ = test_caller;
            error.snapShot_ = snap_shot;
            errors_.push_back(error);
            continue;
        }
    }
}


//-----------------------------------------------------------------------------
template <typename T>
void TestCase<T>::terminate()
{
    for (TestCallers::iterator iter = testCallers_.begin();
         iter != testCallers_.end(); ++iter)
        delete (*iter);
    testCallers_.clear();
}


//-----------------------------------------------------------------------------
template <typename T>
int TestCase<T>::getNumTests() const
{
    return static_cast<int>(testCallers_.size());
}

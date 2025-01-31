//-----------------------------------------------------------------------------
template <typename T>
TestCaller<T>::TestCaller
(MyTestCase* test_case, MyTestMethod test_method, const char* name):
testCase_(test_case), testMethod_(test_method), name_(name)
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
TestCaller<T>::~TestCaller()
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
void TestCaller<T>::call()
{
    T* instance = static_cast<T*>(testCase_);
    (instance->*testMethod_)();
}


//-----------------------------------------------------------------------------
template <typename T>
const String& TestCaller<T>::getName() const
{
    return name_;
}
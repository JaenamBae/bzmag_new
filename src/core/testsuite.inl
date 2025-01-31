//-----------------------------------------------------------------------------
template <typename T>
void TestSuite::addTestCase()
{
    testCases_.push_back(new T);
}

#ifndef BZMAG_CORE_UTILITY_UNITTEST_TESTCALLERBASE_H
#define BZMAG_CORE_UTILITY_UNITTEST_TESTCALLERBASE_H
/**
    @ingroup bzmagCoreUnitTest
    @class bzmag::TestCallerBase
    @brief 
*/

#include "string.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API TestCallerBase
    {
    public:
        virtual~TestCallerBase() {}

        virtual void call()=0;
        virtual const String& getName() const=0;
    };
}

#endif // BZMAG_CORE_UTILITY_UNITTEST_TESTCALLERBASE_H

#ifndef BZMAG_CORE_OBJECT_PARAMETER_H
#define BZMAG_CORE_OBJECT_PARAMETER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::Parameter
    @brief
*/

#include "platform.h"
#include "variables.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Parameter
    {
    public:
        //virtual ~Parameter() {}
        void clear();

        Variables* in();
        Variables* out();

    private:
        Variables in_;
        Variables out_;
    };

#include "parameter.inl"

}

#endif // BZMAG_CORE_OBJECT_PARAMETER_H

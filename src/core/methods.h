#ifndef BZMAG_CORE_PROPERTY_METHODS_H
#define BZMAG_CORE_PROPERTY_METHODS_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::Properties
    @brief
*/

#pragma warning(disable: 4251)

#include <map>
#include "platform.h"
#include "method.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Methods : public std::map<String, Method*>
    {
    public:
    };
}

#endif // BZMAG_CORE_PROPERTY_METHODS_H

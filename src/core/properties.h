#ifndef BZMAG_CORE_PROPERTY_PROPERTIES_H
#define BZMAG_CORE_PROPERTY_PROPERTIES_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::Properties
    @brief
*/

#pragma warning(disable: 4251)

#include <map>
#include "platform.h"
#include "property.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Properties : public std::map<String, Property*>
    {
    public:
    };
}

#endif // BZMAG_CORE_PROPERTY_PROPERTIES_H

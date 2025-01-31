#ifndef BZMAG_CORE_UTILITY_STRINGCONVERTER_H
#define BZMAG_CORE_UTILITY_STRINGCONVERTER_H
/**
    @ingroup bzmagCoreUtility
    @class bzmag::StringConverter
    @brief 
*/

#include "platform.h"
#include "primitivetype.h"
#include "string.h"

namespace bzmag
{
    class Object;
    class Property;
    class BZMAG_LIBRARY_API StringConverter
    {
    public:
        virtual type_id getTypeId()=0;
        virtual const char* getTypeKeyword() const=0;
        virtual const String& toString(Object* object, Property* property)=0;
        virtual void fromString(Object* object, Property* property, const String& value)=0;
    };
}

#endif // BZMAG_CORE_UTILITY_STRINGCONVERTER_H

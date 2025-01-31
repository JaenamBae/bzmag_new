#ifndef BZMAG_CORE_PROPERTY_PROPERTY_H
#define BZMAG_CORE_PROPERTY_PROPERTY_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::Property
    @brief
*/

#include "platform.h"
#include "string.h"
#include "primitivetype.h"
#include "typeid.h"

namespace bzmag
{
    class Object;
    class BZMAG_LIBRARY_API Property
    {
    public:
        //enum PTYPE {
        //  UNDEFINED = 0,
        //  PRIMITIVE,
        //  ENUM,
        //  STRUCT,
        //  LIST,
        //  DICTIONARY
        //};

    public:
        void setName(const String& name);
        virtual const String& getName() const;
        const String& toString(Object* object);
        void fromString(Object* object, const String& value);
        const char* getTypeKeyword() const;

        virtual bool isReadOnly() const = 0;
        virtual Property* findProperty(const String& name);

        virtual type_id getType() const;
        //virtual PTYPE getPredefinedType() const;

    private:
        String name_;
    };

#include "property.inl"

}

#endif // BZMAG_CORE_PROPERTY_PROPERTY_H

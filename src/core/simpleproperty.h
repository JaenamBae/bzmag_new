#ifndef BZMAG_CORE_PROPERTY_SIMPLEPROPERTY_H
#define BZMAG_CORE_PROPERTY_SIMPLEPROPERTY_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::SimpleProperty
    @brief
*/

#include "property.h"

namespace bzmag
{
    template <typename PROPERTY_TYPE>
    class SimpleProperty : public Property
    {
    public:
        virtual void set(Object* object, PROPERTY_TYPE value)=0;
        virtual PROPERTY_TYPE get(Object* object) const=0;

        virtual type_id getType() const;
		//virtual PTYPE getPredefinedType() const;
    };

#include "simpleproperty.inl"

}

#endif // BZMAG_CORE_PROPERTY_SIMPLEPROPERTY_H

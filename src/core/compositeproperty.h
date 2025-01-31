#ifndef BZMAG_CORE_PROPERTY_COMPOSITEPROPERTY_H
#define BZMAG_CORE_PROPERTY_COMPOSITEPROPERTY_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::CompositeProperty
    @brief
*/

#include "properties.h"

namespace bzmag
{
    template <typename PROPERTY_TYPE>
    class CompositeProperty : public Property
    {
    public:
        virtual void set(Object* object, PROPERTY_TYPE value) = 0;
        virtual PROPERTY_TYPE get(Object* object) const = 0;

        virtual type_id getType() const;
        //virtual PTYPE getPredefinedType() const;

        bool addProperty(Property* property);
        virtual Property* findProperty(const String& name);
        size_t getPropertySize() const;
        Properties::iterator firstProperty();
        Properties::iterator lastProperty();
        Properties::const_iterator firstProperty() const;
        Properties::const_iterator lastProperty() const;

    private:
        Properties properties_;
    };

#include "compositeproperty.inl"

}

#endif // BZMAG_CORE_PROPERTY_COMPOSITEPROPERTY_H

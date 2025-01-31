#ifndef BZMAG_CORE_PROPERTY_ENUMPROPERTYBINDER_H
#define BZMAG_CORE_PROPERTY_ENUMPROPERTYBINDER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::EnumPropertyBinder
    @brief
*/

#include "enumproperty.h"

namespace bzmag
{
    template <typename TYPE, typename PROPERTY_TYPE>
    class EnumPropertyBinder : public EnumProperty<PROPERTY_TYPE>
    {
    private:
        typedef void (TYPE::*Setter)(const PROPERTY_TYPE&);
        typedef const PROPERTY_TYPE& (TYPE::*Getter)() const;

    public:
        EnumPropertyBinder();

        void bind(Setter set, Getter get);
        virtual void set(Object* object, const String& name);
        virtual void set(Object* object, const PROPERTY_TYPE& value);
        virtual const PROPERTY_TYPE& get(Object* object) const;
        virtual bool isReadOnly() const;

    private:
        Setter setter;
        Getter getter;
    };

#include "enumpropertybinder.inl"

#define BIND_ENUM_PROPERTY(ptype, name, setter, getter) \
    static EnumPropertyBinder<type, ptype> s_##ptype##_property;\
    s_##ptype##_property.setName(#name);\
    s_##ptype##_property.bind(setter, getter);\
    TYPE.addProperty(&s_##ptype##_property);

#define ENUM_PROPERTY_ADD(ptype, value) \
    s_##ptype##_property.add(#value, value);

}

#endif // BZMAG_CORE_PROPERTY_ENUMPROPERTYBINDER_H

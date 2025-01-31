#ifndef BZMAG_CORE_PROPERTY_COMPOSITEPROPERTYBINDER_H
#define BZMAG_CORE_PROPERTY_COMPOSITEPROPERTYBINDER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::VectorProperty
    @brief
*/

#include "define.h"
#include "compositeproperty.h"
#include "properties.h"

namespace bzmag
{
    template <typename TYPE, typename PROPERTY_TYPE>
    class CompositePropertyBinder : public CompositeProperty<PROPERTY_TYPE>
    {
    private:
        typedef void (TYPE::*Setter)(PROPERTY_TYPE);
        typedef PROPERTY_TYPE (TYPE::*Getter)() const;

    public:
        CompositePropertyBinder();

        void bind(Setter set, Getter get);
        virtual void set(Object* object, PROPERTY_TYPE value);
        virtual PROPERTY_TYPE get(Object* object) const;
        virtual bool isReadOnly() const;

    private:
        Setter setter;
        Getter getter;
    };

#include "compositepropertybinder.inl"

#define BIND_COMPOSITE_PROPERTY(ptype, name, setter, getter) \
    static CompositePropertyBinder<type, ptype> s_##name##_property;\
    s_##name##_property.setName(#name);\
    s_##name##_property.bind(setter, getter);\
    TYPE.addProperty(&s_##name##_property);

#define BIND_COMPOSITE_PROPERTY_FILED(cpname, ptype, name, setter, getter) \
    static SimplePropertyBinder<type, ptype> s_##cpname##_##name##_property;\
    s_##cpname##_##name##_property.setName(#name);\
    s_##cpname##_##name##_property.bind(setter, getter);\
    s_##cpname##_property.addProperty(&s_##cpname##_##name##_property);
}

#endif // BZMAG_CORE_PROPERTY_COMPOSITEPROPERTYBINDER_H

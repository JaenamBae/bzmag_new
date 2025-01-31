#ifndef BZMAG_CORE_PROPERTY_SIMPLEPROPERTYBINDER_H
#define BZMAG_CORE_PROPERTY_SIMPLEPROPERTYBINDER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::SimplePropertyBinder
    @brief
*/

#include "simpleproperty.h"

namespace bzmag
{
    template <typename TYPE, typename PROPERTY_TYPE>
    class SimplePropertyBinder : public SimpleProperty<PROPERTY_TYPE>
    {
    private:
        typedef void (TYPE::*Setter)(PROPERTY_TYPE);
        typedef PROPERTY_TYPE (TYPE::*Getter)() const;

    public:
        SimplePropertyBinder();

        void bind(Setter set, Getter get);
        virtual void set(Object* object, PROPERTY_TYPE value);
        virtual PROPERTY_TYPE get(Object* object) const;
        virtual bool isReadOnly() const;

    private:
        Setter setter;
        Getter getter;
    };

#include "simplepropertybinder.inl"

#define BIND_PROPERTY(ptype, name, setter, getter) \
    static SimplePropertyBinder<type, ptype> s_##name##_property;\
    s_##name##_property.setName(#name);\
    s_##name##_property.bind(setter, getter);\
    TYPE.addProperty(&s_##name##_property);
}

#endif // BZMAG_CORE_PROPERTY_SIMPLEPROPERTYBINDER_H

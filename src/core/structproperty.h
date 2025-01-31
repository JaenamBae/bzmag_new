#ifndef BZMAG_CORE_PROPERTY_STRUCTPROPERTY_H
#define BZMAG_CORE_PROPERTY_STRUCTPROPERTY_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::StructProperty
    @brief
*/

#include "platform.h"
#include "define.h"
#include "property.h"
#include "properties.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API StructProperty : public Property
    {
    public:
        virtual bool isReadOnly() const;
        virtual type_id getType() const;
        //virtual PTYPE getPredefinedType() const;

        bool addProperty(Property* property);
        void removeProperty(const String& name);
        Property* findProperty(const String& name);
        size_t getPropertySize() const;
        Properties::iterator firstProperty();
        Properties::iterator lastProperty();
        Properties::const_iterator firstProperty() const;
        Properties::const_iterator lastProperty() const;

    private:
        struct ForTypeId {};

    private:
        Properties properties_;
    };

#include "structproperty.inl"

#define BIND_PROPERTY_STRUCT(name) \
    static StructProperty s_##name##_struct_property;\
    s_##name##_struct_property.setName(#name);\
    TYPE.addProperty(&s_##name##_struct_property);

#define BIND_STRUCT_TO_STRUCT(parent_group_name, group_name) \
    static StructProperty s_##group_name##_struct_property;\
    s_##group_name##_struct_property.setName(#group_name);\
    s_##parent_group_name##_struct_property.addProperty(&s_##group_name##_struct_property);

#define BIND_PROPERTY_TO_STRUCT(group_name, ptype, name, setter, getter) \
    static SimplePropertyBinder<type, ptype> s_##name##_property;\
    s_##name##_property.setName(#name);\
    s_##name##_property.bind(setter, getter);\
    s_##group_name##_struct_property.addProperty(&s_##name##_property);
}


#endif // BZMAG_CORE_PROPERTY_STRUCTPROPERTY_H

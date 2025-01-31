#ifndef BZMAG_CORE_PROPERTY_CONTAINERPROPERTY_H
#define BZMAG_CORE_PROPERTY_CONTAINERPROPERTY_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::ContainerProperty
    @brief
*/

#include "property.h"

namespace bzmag
{
    template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
    class ContainerProperty : public Property
    {
    private:
        typedef void (TYPE::* Inserter)(const KEY_TYPE&);
        typedef void (TYPE::* Remover)(const KEY_TYPE&);
        typedef VALUE_TYPE& (TYPE::* Searcher)(const KEY_TYPE&);

    public:
        ContainerProperty();

        virtual bool isReadOnly() const;
        virtual type_id getType() const;
        //virtual PTYPE getPredefinedType() const;


        void bind(Inserter inserter, Remover remover, Searcher searcher);

        void insert(Object* object, const KEY_TYPE& index);
        void remove(Object* object, const KEY_TYPE& index);
        VALUE_TYPE& search(Object* object, const KEY_TYPE& index);

    private:
        Inserter inserter_;
        Remover remover_;
        Searcher searcher_;
    };

#include "containerproperty.inl"

#define BIND_CONTAINER_PROPERTY(keytype, valuetype, name, inserter, remover, searcher) \
    static ContainerProperty<type, keytype, valuetype> s_##name##_property;\
    s_##name##_property.setName(#name);\
    s_##name##_property.bind(inserter, remover, searcher);\
    TYPE.addProperty(&s_##name##_property);
}

#endif // BZMAG_CORE_PROPERTY_CONTAINERPROPERTY_H

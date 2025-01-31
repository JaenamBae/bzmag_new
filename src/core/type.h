#ifndef BZMAG_CORE_OBJECT_TYPE_H
#define BZMAG_CORE_OBJECT_TYPE_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::Type
    @brief
*/
/**
    @ingroup bzmagCoreObject
    @class bzmag::ConcreteType
    @brief
*/

#include <bitset>
#include <set>
#include "platform.h"
#include "define.h"
#include "properties.h"
#include "methods.h"

namespace bzmag
{
    class Object;
    class BZMAG_LIBRARY_API Type
    {
    public:
        enum Flag
        {
            FLAG_BINDEDMETHOD,
            FLAG_BINDEDPROPERTY,

            FLAG_MAX,
        };
        typedef std::set<Type*> Types;

    public:
        Type(const String& name, Type* base);
        virtual ~Type() {}

        virtual Object* create() const=0;
        virtual void bindMethod()=0;
        virtual void bindProperty()=0;
        virtual bool isAbstract() const=0;

        Type* findTypeInGenerations(const String& name);

        Type* getBase();
        const String& getName() const;
        bool equal(const Type* type) const;
        bool isKindOf(const String& name) const;
        bool isKindOf(const Type* type) const;
        void addDerivedType(Type* type);
        const Types& getDerivedTypes() const;

        bool addMethod(Method* method);
        void removeMethod(const String& name);
        Method* findMethod(const String& name);
        size_t computeMethodSize() const;
        Methods::iterator firstMethod();
        Methods::iterator lastMethod();
        Methods::const_iterator firstMethod() const;
        Methods::const_iterator lastMethod() const;
        bool hasBindedMethod() const;

        bool addProperty(Property* property);
        void removeProperty(const String& name);
        Property* findProperty(const String& name);
        size_t computePropertySize() const;
        Properties::iterator firstProperty();
        Properties::iterator lastProperty();
        Properties::const_iterator firstProperty() const;
        Properties::const_iterator lastProperty() const;
        bool hasBindedProperty() const;
        
        void setFlag(Flag index, bool enable);

    private:
        String name_;
        Type* base_;
        Types derivedTypes_;
        Properties properties_;
        Methods methods_;
        std::bitset<FLAG_MAX> flags_;
    };

    template <typename T>
    class AbstractConcreteType : public Type
    {
    public:
        AbstractConcreteType(const String& name, Type* base);
        virtual Object* create() const;
        virtual void bindMethod();
        virtual void bindProperty();
        virtual bool isAbstract() const;
    };

    template <typename T>
    class ConcreteType : public AbstractConcreteType<T>
    {
    public:
        ConcreteType(const String& name, Type* base);
        virtual Object* create() const;
        virtual bool isAbstract() const;
    };

#include "type.inl"

/// declare ClassBase macro
#define DECLARE_CLASSBASE(cls) public:\
    typedef cls type;\
    virtual const char* toString() const { return #cls; }

/// declare AbstractClass macro
#define DECLARE_SUPERABSTRACTCLASS(cls) \
    DECLARE_CLASSBASE(cls)\
    virtual bzmag::Type* getType() { return &TYPE; }\
    static bzmag::AbstractConcreteType<cls> TYPE;

/// declare AbstractClass macro
#define DECLARE_ABSTRACTCLASS(cls, base_cls) \
    typedef base_cls super;\
    DECLARE_SUPERABSTRACTCLASS(cls)

/// declare SuperClass macro
#define DECLARE_SUPERCLASS(cls) \
    DECLARE_CLASSBASE(cls)\
    virtual bzmag::Type* getType() { return &TYPE; }\
    static bzmag::ConcreteType<cls> TYPE;

/// declare Class macro
#define DECLARE_CLASS(cls, base_cls) public:\
    typedef base_cls super;\
    DECLARE_SUPERCLASS(cls)



/// implement SuperAbstractClass macro
#define IMPLEMENT_SUPERABSTRACTCLASS(cls) \
    bzmag::AbstractConcreteType<cls> cls::TYPE(#cls, 0);

/// implement AbstractClass macro
#define IMPLEMENT_ABSTRACTCLASS(cls, base_cls) \
    bzmag::AbstractConcreteType<cls> cls::TYPE(#cls, &base_cls::TYPE);

/// implement SuperClass macro
#define IMPLEMENT_SUPERCLASS(cls) \
    bzmag::ConcreteType<cls> cls::TYPE(#cls, 0);

/// implement Class macro
#define IMPLEMENT_CLASS(cls, base_cls) \
    bzmag::ConcreteType<cls> cls::TYPE(#cls, &base_cls::TYPE);

}

#endif // BZMAG_CORE_OBJECT_TYPE_H

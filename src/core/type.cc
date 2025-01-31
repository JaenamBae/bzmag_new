#include "type.h"

//#include <boost/tokenizer.hpp>

using namespace bzmag;

//-----------------------------------------------------------------------------
Type::Type(const String& name, Type* base) :
    name_(name), base_(base)
{
    // empty
}


//-----------------------------------------------------------------------------
Type* Type::findTypeInGenerations(const String& name)
{
    Type* cur_type = this;
    while (cur_type)
    {
        if (cur_type->name_ == name)
            return cur_type;
        cur_type = cur_type->base_;
    }
    return 0;
}


//-----------------------------------------------------------------------------
bool Type::isKindOf(const String& name) const
{
    const Type* cur_type = this;
    while (cur_type)
    {
        if (cur_type->name_ == name)
            return true;
        cur_type = cur_type->base_;
    }
    return false;
}


//-----------------------------------------------------------------------------
bool Type::isKindOf(const Type* type) const
{
    const Type* cur_type = this;
    while (cur_type)
    {
        if (cur_type == type)
            return true;
        cur_type = cur_type->base_;
    }
    return false;
}


//-----------------------------------------------------------------------------
void Type::addDerivedType(Type* type)
{
    derivedTypes_.insert(type);
}


//-----------------------------------------------------------------------------
bool Type::addMethod(Method* method)
{
    if (hasBindedMethod())
        return false;
    std::pair<Methods::iterator, bool> result =
        methods_.insert(Methods::value_type(method->getName(), method));
    return result.second;
}


//-----------------------------------------------------------------------------
void Type::removeMethod(const String& name)
{
    methods_.erase(name);
}


//-----------------------------------------------------------------------------
Method* Type::findMethod(const String& name)
{
    const Type* type = this;
    while (type)
    {
        Methods::const_iterator find_iter =
            type->methods_.find(name);
        if (type->methods_.end() == find_iter)
        {
            type = type->base_;
            continue;
        }
        return find_iter->second;
    }
    return 0;
}


//-----------------------------------------------------------------------------
size_t Type::computeMethodSize() const
{
    size_t s = 0;
    const Type* type = this;
    while (type)
    {
        s += type->methods_.size();
        type = type->base_;
    }
    return s;
}


//-----------------------------------------------------------------------------
bool Type::hasBindedMethod() const
{
    return flags_[FLAG_BINDEDMETHOD];
};


//-----------------------------------------------------------------------------
bool Type::addProperty(Property* property)
{
    if (hasBindedProperty())
        return false;
    std::pair<Properties::iterator, bool> result =
        properties_.insert(Properties::value_type(
            property->getName(), property));
    return result.second;
}


//-----------------------------------------------------------------------------
void Type::removeProperty(const String& name)
{
    properties_.erase(name);
}


//-----------------------------------------------------------------------------
Property* Type::findProperty(const String& name)
{
    size_t o = name.find(".", 0);
    String prop_name(name.substr(0, o));

    const Type* type = this;
    while (type)
    {
        Properties::const_iterator find_iter = type->properties_.find(prop_name);
        if (type->properties_.end() == find_iter)
        {
            type = type->base_;
            continue;
        }

        if (o != -1)
        {
            Property* prop = find_iter->second;
            while (prop)
            {
                size_t l = name.find(".", o + 1);
                prop_name.assign(name.substr(o + 1, l - o - 1));
                o = l;
                prop = prop->findProperty(prop_name);
                if (l == -1) break;
            }
            return prop;
        }
        else
        {
            return find_iter->second;
        }
    }
    return 0;
}


//-----------------------------------------------------------------------------
size_t Type::computePropertySize() const
{
    size_t s = 0;
    const Type* type = this;
    while (type)
    {
        s += type->properties_.size();
        type = type->base_;
    }
    return s;
}


//-----------------------------------------------------------------------------
bool Type::hasBindedProperty() const
{
    return flags_[FLAG_BINDEDPROPERTY];
}

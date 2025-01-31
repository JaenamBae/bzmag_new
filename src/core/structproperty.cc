#include "structproperty.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
bool StructProperty::isReadOnly() const
{
    return true;
}

//-----------------------------------------------------------------------------
bool StructProperty::addProperty(Property* property)
{
    std::pair<Properties::iterator, bool> result =
        properties_.insert(Properties::value_type(
        property->getName(), property));
    return result.second;
}


//-----------------------------------------------------------------------------
void StructProperty::removeProperty(const String& name)
{
    properties_.erase(name);
}


//-----------------------------------------------------------------------------
Property* StructProperty::findProperty(const String& name)
{
    Properties::const_iterator find_iter =
        properties_.find(name);
    if (properties_.end() == find_iter)
        return 0;
    return find_iter->second;
}


//-----------------------------------------------------------------------------
size_t StructProperty::getPropertySize() const
{
    return properties_.size();
}


//-----------------------------------------------------------------------------
Properties::iterator StructProperty::firstProperty()
{
    return properties_.begin();
}


//-----------------------------------------------------------------------------
Properties::iterator StructProperty::lastProperty()
{
    return properties_.end();
}


//-----------------------------------------------------------------------------
Properties::const_iterator StructProperty::firstProperty() const
{
    return properties_.begin();
}


//-----------------------------------------------------------------------------
Properties::const_iterator StructProperty::lastProperty() const
{
    return properties_.end();
}

#include "property.h"
#include "tostring.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
const String& Property::toString(Object* object)
{
    return ToString::instance()->toString(object, this);
}


//-----------------------------------------------------------------------------
void Property::fromString(Object* object, const String& value)
{
    ToString::instance()->fromString(object, this, value);
}


//-----------------------------------------------------------------------------
const char* Property::getTypeKeyword() const
{
    return ToString::instance()->getTypeKeyword(this);
}


//-----------------------------------------------------------------------------
Property* Property::findProperty(const String& name)
{
    return 0;
}

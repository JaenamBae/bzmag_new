#include "tostring.h"

#include <strsafe.h>
#include "define.h"
#include "primitive_stringconverter.h"

using namespace bzmag;


//-----------------------------------------------------------------------------
ToString::ToString()
{
    static BoolStringConverter bool_sc;
    addConverter(&bool_sc);

    static Int16StringConverter int16_sc;
    addConverter(&int16_sc);

    static UnsignedInt16StringConverter unsignedint16_sc;
    addConverter(&unsignedint16_sc);

    static IntStringConverter int_sc;
    addConverter(&int_sc);

    static UnsignedIntStringConverter unsignedint_sc;
    addConverter(&unsignedint_sc);

    static Int32StringConverter int32_sc;
    addConverter(&int32_sc);

    static UnsignedInt32StringConverter unsignedint32_sc;
    addConverter(&unsignedint32_sc);

    static Int64StringConverter int64_sc;
    addConverter(&int64_sc);

    static UnsignedInt64StringConverter unsignedint64_sc;
    addConverter(&unsignedint64_sc);

    static FloatStringConverter float_sc;
    addConverter(&float_sc);

    static DoubleStringConverter double_sc;
    addConverter(&double_sc);

    static StringStringConverter string_sc;
    addConverter(&string_sc);

    static Vector2StringConverter vector2_sc;
    addConverter(&vector2_sc);

    static DataSetStringConverter dataset_sc;
    addConverter(&dataset_sc);

    static ColorStringConverter color_sc;
    addConverter(&color_sc);

    static ObjectStringConverter object_sc;
    addConverter(&object_sc);

    static NodeStringConverter node_sc;
    addConverter(&node_sc);
}


//-----------------------------------------------------------------------------
ToString::~ToString()
{
    // empty
}


//-----------------------------------------------------------------------------
void ToString::addConverter(StringConverter* sc)
{
    converters_[sc->getTypeId()] = sc;
}


//-----------------------------------------------------------------------------
const char* ToString::getTypeKeyword(const Property* property) const
{
    type_id id = property->getType();
    StringConverters::const_iterator find_iter =
        converters_.find(property->getType());
    if (converters_.end() == find_iter)
        return "";
    return find_iter->second->getTypeKeyword();
}


//-----------------------------------------------------------------------------
const String& ToString::toString(Object* object, Property* property)
{
    StringConverters::iterator find_iter =
        converters_.find(property->getType());
    if (converters_.end() == find_iter)
    {
        static String empty;
        return empty;
    }
    return find_iter->second->toString(object, property);
}


//-----------------------------------------------------------------------------
void ToString::fromString
(Object* object, Property* property, const String& value)
{
    StringConverters::iterator find_iter =
        converters_.find(property->getType());
    if (converters_.end() == find_iter)
        return;
    return find_iter->second->fromString(object, property, value);
}

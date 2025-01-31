//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
void EnumProperty<PROPERTY_TYPE>::add(const String& name, const Value& value)
{
    //stringMap_.insert(StringMap::value_type(name, value));
    stringMap_.insert(typename EnumProperty<PROPERTY_TYPE>::StringMap::value_type(name, value));

    //valueMap_.insert(ValueMap::value_type(value, name));
    valueMap_.insert(typename EnumProperty<PROPERTY_TYPE>::ValueMap::value_type(value, name));
}


//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
const String& EnumProperty<PROPERTY_TYPE>::getName(const Value& value)
{
    typename ValueMap::iterator find_iter = valueMap_.find(value);
    if (valueMap_.end() == find_iter)
    {
        static String __empty__;
        return __empty__;
    }
    return find_iter->second;
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
const typename EnumProperty<PROPERTY_TYPE>::Value&
EnumProperty<PROPERTY_TYPE>::getValue(const String& name)
{
    typename StringMap::iterator find_iter = stringMap_.find(name);
    if (stringMap_.end() == find_iter)
    {
        static int __empty__ = -1;
        return __empty__;
    }
    return find_iter->second;
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
typename EnumProperty<PROPERTY_TYPE>::iterator
EnumProperty<PROPERTY_TYPE>::firstEnumerator()
{
    return valueMap_.begin();
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
typename EnumProperty<PROPERTY_TYPE>::iterator
EnumProperty<PROPERTY_TYPE>::lastEnumerator()
{
    return valueMap_.end();
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
typename EnumProperty<PROPERTY_TYPE>::const_iterator
EnumProperty<PROPERTY_TYPE>::firstEnumerator() const
{
    return valueMap_.begin();
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
typename EnumProperty<PROPERTY_TYPE>::const_iterator
EnumProperty<PROPERTY_TYPE>::lastEnumerator() const
{
    return valueMap_.end();
}

//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
type_id EnumProperty<PROPERTY_TYPE>::getType() const
{
    return TypeId<PROPERTY_TYPE>::id();
}

//-----------------------------------------------------------------------------
//template <typename PROPERTY_TYPE>
//inline Property::PTYPE EnumProperty<PROPERTY_TYPE>::getPredefinedType() const
//{
//	return ENUM;
//}
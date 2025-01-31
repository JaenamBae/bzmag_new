//-----------------------------------------------------------------------------
template <typename PROPERTY_TYPE>
type_id SimpleProperty<PROPERTY_TYPE>::getType() const
{
    return TypeId<PROPERTY_TYPE>::id();
}

//-----------------------------------------------------------------------------
//template <typename PROPERTY_TYPE>
//Property::PTYPE SimpleProperty<PROPERTY_TYPE>::getPredefinedType() const
//{
//	return PRIMITIVE;
//}
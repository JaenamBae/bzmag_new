//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
EnumPropertyBinder<TYPE, PROPERTY_TYPE>::EnumPropertyBinder():
    setter(0), getter(0)
{
    // empty
}

//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
void EnumPropertyBinder<TYPE, PROPERTY_TYPE>::bind(Setter set, Getter get)
{
    setter = set;
    getter = get;
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
void EnumPropertyBinder<TYPE, PROPERTY_TYPE>::set
(Object* object, const String& name)
{
    //StringMap::iterator find_iter = this->stringMap_.find(name);
    typename EnumProperty<PROPERTY_TYPE>::StringMap::iterator find_iter = this->stringMap_.find(name);
    if (this->stringMap_.end() == find_iter)
        return;
    set(object, find_iter->second);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
void EnumPropertyBinder<TYPE, PROPERTY_TYPE>::set
(Object* object, const PROPERTY_TYPE& value)
{
    TYPE* self = DOWN_CAST<TYPE*>(object);
    (self->*setter)(value);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
const PROPERTY_TYPE& EnumPropertyBinder<TYPE, PROPERTY_TYPE>::get
(Object* object) const
{
    TYPE* self = DOWN_CAST<TYPE*>(object);
    return (self->*getter)();
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
bool EnumPropertyBinder<TYPE, PROPERTY_TYPE>::isReadOnly() const
{
    return 0 == setter;
}

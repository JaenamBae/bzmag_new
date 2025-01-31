//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
SimplePropertyBinder<TYPE, PROPERTY_TYPE>::SimplePropertyBinder():
    setter(0), getter(0)
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
void SimplePropertyBinder<TYPE, PROPERTY_TYPE>::bind(Setter set, Getter get)
{
    setter = set;
    getter = get;
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
void SimplePropertyBinder<TYPE, PROPERTY_TYPE>::set
(Object* object, PROPERTY_TYPE value)
{
    TYPE* self = static_cast<TYPE*>(object);
    (self->*setter)(value);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
PROPERTY_TYPE SimplePropertyBinder<TYPE, PROPERTY_TYPE>::get
(Object* object) const
{
    TYPE* self = static_cast<TYPE*>(object);
    return (self->*getter)();
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename PROPERTY_TYPE>
bool SimplePropertyBinder<TYPE, PROPERTY_TYPE>::isReadOnly() const
{
    return 0 == setter;
}

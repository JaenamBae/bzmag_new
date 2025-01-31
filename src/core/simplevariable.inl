//-----------------------------------------------------------------------------
template <typename T>
Variable* SimpleVariable<T>::clone()
{
    return new type(*this);
}


//-----------------------------------------------------------------------------
template <typename T>
T& SimpleVariable<T>::get()
{
    return data_;
}


//-----------------------------------------------------------------------------
template <typename T>
const T& SimpleVariable<T>::get() const
{
    return data_;
}


//-----------------------------------------------------------------------------
template <typename T>
typename SimpleVariable<T>::type& SimpleVariable<T>::operator = (const T& v)
{
    data_ = v;
    return *this;
}


//-----------------------------------------------------------------------------
template <typename T>
SimpleVariable<T>::operator T ()
{
    return data_;
}


//-----------------------------------------------------------------------------
template <typename T>
type_id SimpleVariable<T>::getType() const
{
    return TypeId<T>::id();
}

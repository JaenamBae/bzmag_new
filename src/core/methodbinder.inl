//-----------------------------------------------------------------------------
template <typename T>
void MethodBinder<T>::invoke(Object* self)
{
    baseBindFunc_(self, getParameter());
}


//-----------------------------------------------------------------------------
template <typename T>
void MethodBinder<T>::setBindFunc(BindFunc func)
{
    bindFunc_ = func;
}


//-----------------------------------------------------------------------------
template <typename T>
typename MethodBinder<T>::BindFunc MethodBinder<T>::getBindFunc()
{
    return bindFunc_;
}

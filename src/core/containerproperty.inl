//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::ContainerProperty():
    inserter_(0), remover_(0), searcher_(0)
{
    // empty
}

//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
bool ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::isReadOnly() const
{
    return (inserter_ == 0 || remover_ == 0);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
type_id ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::getType() const
{
    typedef std::pair<KEY_TYPE, VALUE_TYPE> ForTypeId;
    return TypeId<ForTypeId>::id();
}

//-----------------------------------------------------------------------------
//template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
//Property::PTYPE ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::getPredefinedType() const
//{
//	return LIST;
//}

//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
void ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::bind
(Inserter inserter, Remover remover, Searcher searcher)
{
    inserter_ = inserter;
    remover_ = remover;
    searcher_ = searcher;
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
void ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::insert
(Object* object, const KEY_TYPE& index)
{
    TYPE* self = DOWN_CAST<TYPE*>(object);
    return (self->*inserter_)(index);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
void ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::remove
(Object* object, const KEY_TYPE& index)
{
    TYPE* self = DOWN_CAST<TYPE*>(object);
    return (self->*remover_)(index);
}


//-----------------------------------------------------------------------------
template <typename TYPE, typename KEY_TYPE, typename VALUE_TYPE>
VALUE_TYPE& ContainerProperty<TYPE, KEY_TYPE, VALUE_TYPE>::search
(Object* object, const KEY_TYPE& index)
{
    TYPE* self = DOWN_CAST<TYPE*>(object);
    return (self->*searcher_)(index);
}

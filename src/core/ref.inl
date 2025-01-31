//-----------------------------------------------------------------------------
template <typename T>
Ref<T>::Ref() : instance_(nullptr)
{
    // empty
}

//-----------------------------------------------------------------------------
template <typename T>
Ref<T>::Ref(const Ref<T>& ref) : instance_(nullptr)
{
    initialize(ref.instance_);
}

//-----------------------------------------------------------------------------
template <typename T>
Ref<T>::Ref(Object* instance) : instance_(nullptr)
{
    initialize(instance);
}

//-----------------------------------------------------------------------------
template <typename T>
Ref<T>::~Ref()
{
    release();
}

//-----------------------------------------------------------------------------
template <typename T>
void Ref<T>::initialize(Object* instance)
{
    if (instance_)
        instance_->release();
    instance_ = static_cast<T*>(instance);
    if (instance_)
        instance_->addRef();
}

//-----------------------------------------------------------------------------
template <typename T>
inline  void Ref<T>::release()
{
    if (instance_)
        instance_->release();
    instance_ = 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline  bool Ref<T>::valid() const
{
    return instance_ ? true:false;
}

//-----------------------------------------------------------------------------
template <typename T>
inline  bool Ref<T>::invalid() const
{
    return !valid();
}

//-----------------------------------------------------------------------------
template <typename T>
template <typename DERIVED>
inline  DERIVED Ref<T>::get()
{            
    return dynamic_cast<DERIVED>(node_);
}

//-----------------------------------------------------------------------------
template <typename T>
template <typename DERIVED>
DERIVED Ref<T>::get() const
{            
    return dynamic_cast<DERIVED>(node_);
}

//-----------------------------------------------------------------------------
template <typename T>
template <typename DERIVED>
Ref<T>::operator DERIVED ()
{
    return get<DERIVED>();
}

//-----------------------------------------------------------------------------
template <typename T>
template <typename DERIVED>
Ref<T>::operator DERIVED () const
{
    return get<DERIVED>();
}

//-----------------------------------------------------------------------------
template <typename T>
Ref<T>::operator T* ()
{
    return instance_;
}



//-----------------------------------------------------------------------------
template <typename T>
T* Ref<T>::operator -> ()
{
    return instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
T* Ref<T>::operator -> () const
{
    return instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
T* Ref<T>::operator * ()
{
    return instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
T* Ref<T>::operator * () const
{
    return instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
typename Ref<T>::type& Ref<T>::operator = (Object* instance)
{
    initialize(instance);
    return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator == (Object* instance) const
{
    return node_ == instance;
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator != (Object* instance) const
{
    return !(*this == instance);
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator == (const type& rhs) const
{
    return instance_ == rhs.instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator != (const type& rhs) const
{
    return !(*this == rhs);
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator < (const type& rhs) const
{
    return instance_ < rhs.instance_;
}

//-----------------------------------------------------------------------------
template <typename T>
bool Ref<T>::operator > (const type& rhs) const
{
    return instance_ > rhs.instance_;
}

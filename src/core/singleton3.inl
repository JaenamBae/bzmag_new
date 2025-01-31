//-----------------------------------------------------------------------------
template <typename T>
Singleton3<T>::Singleton3()
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
Singleton3<T>::~Singleton3()
{
    if (this == s_instance)
    {
        SingletonServer::instance()->unregisterSingleton(this);
        s_instance = 0;
        s_path.clear();
    }
}


//-----------------------------------------------------------------------------
template <typename T>
int Singleton3<T>::addRefSingleton()
{
    T* t = dynamic_cast<T*>(this);
    if (t)
        return t->addRef();
    return 0;
}        


//-----------------------------------------------------------------------------
template <typename T>
int Singleton3<T>::releaseSingleton()
{
    T* t = dynamic_cast<T*>(this);
    if (t)
        return t->release();
    return 0;
}


//-----------------------------------------------------------------------------
template <typename T>
int Singleton3<T>::getRefSingleton() const
{
    const T* t = dynamic_cast<const T*>(this);
    if (t)
        return t->getRef();
    return 0;
}


//-----------------------------------------------------------------------------
template <typename T>
void Singleton3<T>::setSingletonPath(const Path& path, bool init)
{
    s_path = path;
    if (init)
    {
        if (!T::TYPE.isAbstract())
            instance();
    }
}


//-----------------------------------------------------------------------------
template <typename T>
T* Singleton3<T>::instance()
{
//    bzmag_assert(s_path.size());
    if (s_instance)
        return s_instance;
    s_instance = dynamic_cast<T*>(
        Kernel::instance()->create(T::TYPE.getName().c_str(), s_path));
    SingletonServer::instance()->registerSingleton(s_instance);
    return s_instance;
}


//-----------------------------------------------------------------------------
template <typename T>
bool Singleton3<T>::isInitialized()
{
    return s_instance ? true:false;
}


//-----------------------------------------------------------------------------
template <typename T>
T* Singleton3<T>::s_instance;


//-----------------------------------------------------------------------------
template <typename T>
Path Singleton3<T>::s_path;

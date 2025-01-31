//-----------------------------------------------------------------------------
template <typename T>
Singleton<T>::Singleton()
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
Singleton<T>::~Singleton()
{
    if (this == s_instance)
    {
        SingletonServer::instance()->unregisterSingleton(this);
        s_instance = 0;
    }
}


//-----------------------------------------------------------------------------
template <typename T>
T* Singleton<T>::instance()
{
    if (s_instance)
        return s_instance;
    s_instance = new T;
    SingletonServer::instance()->registerSingleton(s_instance);
    return s_instance;
}


//-----------------------------------------------------------------------------
template <typename T>
bool Singleton<T>::isInitialized()
{
    return s_instance ? true:false;
}


//-----------------------------------------------------------------------------
template <typename T>
T* Singleton<T>::s_instance;

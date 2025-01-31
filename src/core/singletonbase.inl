//-----------------------------------------------------------------------------
inline int SingletonBase::addRefSingleton()
{
    return ++refCount_;
}


//-----------------------------------------------------------------------------
inline int SingletonBase::releaseSingleton()
{
    if (--refCount_ == 0)
    {
        delete this;
        return 0;
    }
    return refCount_;
}


//-----------------------------------------------------------------------------
inline int SingletonBase::getRefSingleton() const
{
    return refCount_;
}

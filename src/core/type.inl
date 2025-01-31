//-----------------------------------------------------------------------------
inline Type* Type::getBase()
{
    return base_;
}


//-----------------------------------------------------------------------------
inline const Type::Types& Type::getDerivedTypes() const
{
    return derivedTypes_;
}


//-----------------------------------------------------------------------------
inline const String& Type::getName() const
{
    return name_;
}


//-----------------------------------------------------------------------------
inline bool Type::equal(const Type* type) const
{
    return this == type;
}


//-----------------------------------------------------------------------------
inline Methods::iterator Type::firstMethod()
{
    return methods_.begin();
}


//-----------------------------------------------------------------------------
inline Methods::iterator Type::lastMethod()
{
    return methods_.end();
}


//-----------------------------------------------------------------------------
inline Methods::const_iterator Type::firstMethod() const
{
    return methods_.begin();
}


//-----------------------------------------------------------------------------
inline Methods::const_iterator Type::lastMethod() const
{
    return methods_.end();
}


//-----------------------------------------------------------------------------
inline Properties::iterator Type::firstProperty()
{
    return properties_.begin();
}


//-----------------------------------------------------------------------------
inline Properties::iterator Type::lastProperty()
{
    return properties_.end();
}


//-----------------------------------------------------------------------------
inline Properties::const_iterator Type::firstProperty() const
{
    return properties_.begin();
}


//-----------------------------------------------------------------------------
inline Properties::const_iterator Type::lastProperty() const
{
    return properties_.end();
}


//-----------------------------------------------------------------------------
inline void Type::setFlag(Flag index, bool enable)
{
    flags_[index] = enable;
}


//-----------------------------------------------------------------------------
template <typename T>
AbstractConcreteType<T>::AbstractConcreteType(const String& name, Type* base):
Type(name, base)
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
Object* AbstractConcreteType<T>::create() const
{
    // can't create abstract class
    return 0;
}


//-----------------------------------------------------------------------------
template <typename T>
void AbstractConcreteType<T>::bindMethod()
{
    T::bindMethod();
    setFlag(FLAG_BINDEDMETHOD, true);
}


//-----------------------------------------------------------------------------
template <typename T>
void AbstractConcreteType<T>::bindProperty()
{
    T::bindProperty();
    setFlag(FLAG_BINDEDPROPERTY, true);
}


//-----------------------------------------------------------------------------
template <typename T>
bool AbstractConcreteType<T>::isAbstract() const
{
    return true;
}


//-----------------------------------------------------------------------------
template <typename T>
ConcreteType<T>::ConcreteType(const String& name, Type* base):
AbstractConcreteType<T>(name, base)
{
    // empty
}


//-----------------------------------------------------------------------------
template <typename T>
Object* ConcreteType<T>::create() const
{
    return new T();
}


//-----------------------------------------------------------------------------
template <typename T>
bool ConcreteType<T>::isAbstract() const
{
    return false;
}

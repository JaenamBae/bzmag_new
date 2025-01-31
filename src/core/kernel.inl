//-----------------------------------------------------------------------------
inline Node* Kernel::getRoot()
{
    return root_;
}


//-----------------------------------------------------------------------------
inline Kernel::Modules::iterator Kernel::firstModule()
{
    return modules_.begin();
}


//-----------------------------------------------------------------------------
inline Kernel::Modules::iterator Kernel::lastModule()
{
    return modules_.end();
}


//-----------------------------------------------------------------------------
inline Kernel::Modules::const_iterator Kernel::firstModule() const
{
    return modules_.begin();
}


//-----------------------------------------------------------------------------
inline Kernel::Modules::const_iterator Kernel::lastModule() const
{
    return modules_.end();
}


//-----------------------------------------------------------------------------
inline size_t Kernel::getNumModules() const
{
    return modules_.size();
}

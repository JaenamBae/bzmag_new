#include "module.h"

#include "kernel.h"
#include "type.h"
#include "object.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Module::Module
(const String& name,
    InitializeModuleFunc init_func, FinalizeModuleFunc fin_func) :
    name_(name), initializeModuleFunc_(init_func), finalizeModuleFunc_(fin_func)
{
    // empty
}

//-----------------------------------------------------------------------------
Module::~Module()
{
    // empty
}

//-----------------------------------------------------------------------------
void Module::initialize()
{
    if (initializeModuleFunc_)
        initializeModuleFunc_(this);
}

//-----------------------------------------------------------------------------
void Module::finalize()
{
    if (finalizeModuleFunc_)
        finalizeModuleFunc_(this);
    types_.clear();

    delete this;
}

//-----------------------------------------------------------------------------
Object* Module::create(const String& type_name)
{
    Types::iterator find_iter = types_.find(type_name);
    if (types_.end() == find_iter)
        return nullptr;
    return find_iter->second->create();
}

//-----------------------------------------------------------------------------
void Module::addType(Kernel* kernel, const String& type_name, Type* type)
{
    types_.insert(Types::value_type(type_name, type));

    kernel->addType(type_name, this);
    if (type->getBase())
        type->getBase()->addDerivedType(type);
    if (!type->hasBindedMethod())
        type->bindMethod();
    if (!type->hasBindedProperty())
        type->bindProperty();
}

//-----------------------------------------------------------------------------
const Type* Module::findType(const String& type_name) const
{
    Types::const_iterator fi = types_.find(type_name);
    if (types_.end() == fi)
        return 0;
    return fi->second;
}

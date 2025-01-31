#include "builtinmodule.h"

#include "node.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
BuiltinModule::BuiltinModule(Kernel* kernel, const String& name) :
    Module(name, 0, 0)
{
    addType(kernel, "Object", &Object::TYPE);
    addType(kernel, "Node", &Node::TYPE);
}

//-----------------------------------------------------------------------------
BuiltinModule::~BuiltinModule()
{
}

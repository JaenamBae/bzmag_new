#include "kernel.h"

//#include "assert.h"
#include "module.h"
#include "builtinmodule.h"
#include "singletonserver.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Kernel::Kernel()
{
    BuiltinModule* pModule = new BuiltinModule(this, "Built-in");
    addModule(pModule);
    root_ = create_node("Node", "");
    pushCwn(root_);
}


//-----------------------------------------------------------------------------
Kernel::~Kernel()
{
    while (cwn_.size())
        popCwn();
    root_.release();

    initModuleOrder_.reverse();
    for (InitModuleOrder::iterator i = initModuleOrder_.begin();
        i != initModuleOrder_.end(); ++i)
    {
        Module* module = *i;
        module->finalize();
    }

    initModuleOrder_.clear();
    modules_.clear();
    types_.clear();
}


//-----------------------------------------------------------------------------
Object* Kernel::create(const String& type_name)
{
    Types::iterator find_iter = types_.find(type_name);
    if (types_.end() == find_iter)
        return nullptr;
    Object* new_obj = find_iter->second->create(type_name);
    Node* new_node = dynamic_cast<Node*>(new_obj);
    return new_obj;
}


//-----------------------------------------------------------------------------
Node* Kernel::create(const String& type_name, const Path& path)
{
    Node* cur;
    if (path.isAbsolute())
        cur = root_;
    else
        cur = cwn_.top();
    for (Path::const_iterator token = path.begin(); token != path.end(); ++token)
    {
        Node* child = cur->findChild(*token);
        if (nullptr == child)
        {
            Node* new_node;
            if (std::next(token, 1) == path.end())
                new_node = create_node(type_name, *token);
            else
                new_node = create_node("Node", *token);
            if (nullptr == new_node)
                return nullptr;
            cur->attach(new_node);
            cur = new_node;
        }
        else
            cur = child;
    }

    return cur;
}


//-----------------------------------------------------------------------------
Node* Kernel::lookup(const Path& path)
{
    if (path == "/")
        return root_;
    if (path.isAbsolute())
        return root_->relativeNode(path);
    return cwn_.top()->relativeNode(path);
}


//-----------------------------------------------------------------------------
void Kernel::pushCwn(Node* node)
{
    cwn_.push(node);
}


//-----------------------------------------------------------------------------
Node* Kernel::popCwn()
{
//    bzmag_assert(cwn_.size() > 0);
    Node* node = cwn_.top();
    cwn_.pop();
    return node;
}


//-----------------------------------------------------------------------------
Node* Kernel::getCwn()
{
    return cwn_.top();
}


//-----------------------------------------------------------------------------
void Kernel::addModule(Module* module)
{
    if (nullptr == module)
        return;

    modules_.insert(Modules::value_type(module->getName(), module));
    module->initialize();
    initModuleOrder_.push_back(module);
}


//-----------------------------------------------------------------------------
Module* Kernel::findModule(const String& name)
{
    Modules::iterator find_iter = modules_.find(name);
    if (modules_.end() == find_iter)
        return nullptr;
    return find_iter->second;
}


//-----------------------------------------------------------------------------
Module* Kernel::findModuleByTypeName(const String& type_name) const
{
    Types::const_iterator fi = types_.find(type_name);
    if (types_.end() == fi)
        return nullptr;
    return fi->second;
}


//-----------------------------------------------------------------------------
void Kernel::addType(const String& type_name, Module* module)
{
    // insert type names in module to Kernel::types_ for Object creation
    types_.insert(Types::value_type(type_name, module));
}


//-----------------------------------------------------------------------------
const Type* Kernel::findType(const String& type_name) const
{
    Module* module = findModuleByTypeName(type_name);
    if (nullptr == module)
        return nullptr;
    return module->findType(type_name);
}


//-----------------------------------------------------------------------------
Node* Kernel::create_node(const String& type_name, const String& name)
{
    Types::iterator find_iter = types_.find(type_name);
    if (types_.end() == find_iter)
        return nullptr;
    Node* new_node = static_cast<Node*>(
        find_iter->second->create(type_name));
    if (nullptr == new_node)
        return nullptr;
    new_node->setName(name);

    return new_node;
}

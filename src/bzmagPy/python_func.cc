#include "python_extension.h"

#include "core/define.h"
#include "core/kernel.h"
#include "core/module.h"
#include "core/autoreleasepool.h"
// #include "core/xmlserializer.h"

using namespace bzmag;


//-----------------------------------------------------------------------------
bzNode* new_bznode(const char* type_name, const Path& path)
{
    bzNode* o = reinterpret_cast<bzNode*>(
        bzNodeType.tp_new(&bzNodeType, 0, 0));
    if (0 == o)
        return 0;
    o->node_ = Kernel::instance()->create(type_name, path);
    if (o->node_.invalid())
    {
        Py_DECREF(o);
        return 0;
    }
    return o;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_newobject(PyObject* self, PyObject* args)
{
    // parse argument
    char* type_name = 0;
    if (!PyArg_ParseTuple(args, "s:newobject", &type_name))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    bzObject* o = reinterpret_cast<bzObject*>(
        bzObjectType.tp_new(&bzObjectType, 0, 0));
    if (0 == o)
        return 0;
    o->object_ = Kernel::instance()->create(String("%s", type_name).c_str());
    o->createdByPython_ = true;
    if (0 == o->object_)
    {
        Py_DECREF(o);
        return 0;
    }
    return reinterpret_cast<PyObject*>(o);
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_new(PyObject* self, PyObject* args)
{
    // parse argument
    char* type_name = 0;
    char* path = 0;
    if (!PyArg_ParseTuple(args, "ss:new", &type_name, &path))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    // create new object on NOH and create new bzNode
    bzNode* o = new_bznode(
        String("%s", type_name).c_str(), String("%s", path));
    if (0 == o)
        return PyErr_Format(PyExc_Exception, "unable to create [%s]", path);
    return reinterpret_cast<PyObject*>(o);
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_get(PyObject* self, PyObject* args)
{
    char* path = 0;
    if (!PyArg_ParseTuple(args, "s:get", &path))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    Node* node = Kernel::instance()->lookup(String("%s", path));

    if (0 == node)
        return PyErr_Format(PyExc_Exception, "not found node in NOH [%s]", path);

    bzNode* o = reinterpret_cast<bzNode*>(bzNodeType.tp_new(&bzNodeType, 0, 0));
    if (0 == o)
        return 0;

    o->node_ = node;

    return reinterpret_cast<PyObject*>(o);
}

//-----------------------------------------------------------------------------
PyObject* bzmagPy_getObjectByID(PyObject* self, PyObject* args)
{
    // parse argument
    uint32 objectID = -1;
    if (!PyArg_ParseTuple(args, "I:getObjectByID", &objectID))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    Object* obj = AutoReleasePool::instance()->find(objectID);
    if (0 == obj)
        return PyErr_Format(PyExc_Exception, "not found object which ID [%i]", objectID);
    bzObject* o = reinterpret_cast<bzObject*>(
        bzObjectType.tp_new(&bzObjectType, 0, 0));
    if (0 == o)
        return 0;
    o->object_ = obj;
    return reinterpret_cast<PyObject*>(o);
}

//-----------------------------------------------------------------------------
PyObject* bzmagPy_getNodeByID(PyObject* self, PyObject* args)
{
    // parse argument
    uint32 nodeID = -1;
    if (!PyArg_ParseTuple(args, "I:getNodeByID", &nodeID))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    Object* node = AutoReleasePool::instance()->find(nodeID);
    if (0 == node)
        return PyErr_Format(PyExc_Exception, "not found node which ID [%i]", nodeID);
    bzNode* o = reinterpret_cast<bzNode*>(
        bzNodeType.tp_new(&bzNodeType, 0, 0));
    if (0 == o)
        return 0;
    o->node_ = node;
    return reinterpret_cast<PyObject*>(o);
}



//-----------------------------------------------------------------------------
PyObject* bzmagPy_delete(PyObject* self, PyObject* args)
{
    char* path = 0;
    if (!PyArg_ParseTuple(args, "s:delete", &path))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    Node* node = Kernel::instance()->lookup(String("%s", path));
    if (0 == node)
        return PyErr_Format(PyExc_Exception, "not found path [%s]", path);

    node->detach();
    AutoReleasePool::instance()->remove(node);

    Py_INCREF(Py_None);
    return Py_None;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_ls(PyObject* self, PyObject* args)
{
    Node* node = Kernel::instance()->getCwn();
    PyObject* children_names = PyTuple_New(node->getNumChildren());
    int index = 0;
    for (Node::NodeIterator i = node->firstChildNode();
        i != node->lastChildNode(); ++i, ++index)
    {
        PyTuple_SET_ITEM(children_names, index,
            PyString_FromString((*i)->getName()));
    }
    return children_names;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_pushCwn(PyObject* self, PyObject* args)
{
    bzNode* o = 0;
    if (!PyArg_ParseTuple(args, "O:pushcwn", &o))
        return PyErr_Format(PyExc_Exception, "mismatch argument");
    Kernel::instance()->pushCwn(o->node_);
    Py_INCREF(Py_True);
    return Py_True;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_popCwn(PyObject* self, PyObject* args)
{
    Kernel::instance()->popCwn();
    Py_INCREF(Py_True);
    return Py_True;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_print(PyObject* self, PyObject* args)
{
    return 0;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_exit(PyObject* self, PyObject* args)
{
    Py_INCREF(Py_True);
    return Py_True;
}


//-----------------------------------------------------------------------------
// PyObject* bzmagPy_serialize(PyObject* self, PyObject* args)
// {
//     bzNode* o = 0;
//     char* uri = 0;
//     if (!PyArg_ParseTuple(args, "sO:serialize", &uri, &o))
//         return PyErr_Format(PyExc_Exception, "mismatch argument");
// 
//     XmlSerializer s;
//     if (s.serialize(String(uri), o->node_))
//     {
//         Py_INCREF(Py_True);
//         return Py_True;
//     }
//     else
//     {
//         Py_INCREF(Py_False);
//         return Py_False;
//     }
// }


//-----------------------------------------------------------------------------
// PyObject* bzmagPy_deserialize(PyObject* self, PyObject* args)
// {
//     bzNode* parent_o = 0;
//     char* uri = 0;
//     char* name = 0;
//     if (!PyArg_ParseTuple(args, "Os|s:deserialize", &parent_o, &uri, &name))
//         return PyErr_Format(PyExc_Exception, "mismatch argument");
// 
//     XmlSerializer s;
//     Object* result_o = 0;
//     if (name)
//         result_o = s.deserialize(
//             parent_o->node_, String(uri), String(name).c_str());
//     else
//         result_o = s.deserialize(parent_o->node_, String(uri), 0);
//     if (0 == result_o)
//         return PyErr_Format(PyExc_Exception, "resource not found(%s)", uri);
// 
//     bzObject* o = reinterpret_cast<bzObject*>(
//         bzObjectType.tp_new(&bzObjectType, 0, 0));
//     if (0 == o)
//     {
//         Py_INCREF(Py_None);
//         return Py_None;
//     }
//     o->object_ = result_o;
//     return reinterpret_cast<PyObject*>(o);
// 
// }


//-----------------------------------------------------------------------------
PyObject* bzmagPy_getModuleList(PyObject* self, PyObject* args)
{
    PyObject* result = PyTuple_New(Kernel::instance()->getNumModules());

    int i = 0;
    for (Kernel::Modules::iterator m = Kernel::instance()->firstModule();
        m != Kernel::instance()->lastModule(); ++m, ++i)
    {
        Module* module = m->second;
        PyTuple_SET_ITEM(result, i, PyString_FromString(module->getName()));
    }

    return result;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_getTypeList(PyObject* self, PyObject* args)
{
    char* module_name = 0;
    if (!PyArg_ParseTuple(args, "s:getTypeList", &module_name))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    PyObject* result = 0;
    Module* module = Kernel::instance()->findModule(String(module_name).c_str());
    if (0 == module)
    {
        result = PyTuple_New(0);
        return result;
    }
    else
        result = PyTuple_New(module->getNumTypes());

    int i = 0;
    for (Module::Types::iterator t = module->firstType();
        t != module->lastType(); ++t, ++i)
    {
        const Type* type = t->second;
        PyTuple_SET_ITEM(result, i, PyString_FromString(type->getName()));
    }

    return result;

}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_getDerivedTypes(PyObject* self, PyObject* args)
{
    char* type_name = 0;
    if (!PyArg_ParseTuple(args, "s:getDerivedTypes", &type_name))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    PyObject* result = 0;
    const Type* type = Kernel::instance()->findType(type_name);
    if (0 == type)
        return PyErr_Format(PyExc_TypeError,
            "there're no exist specified name of type \'%s\'", type_name);
    const Type::Types& types = type->getDerivedTypes();
    result = PyTuple_New(types.size());
    int i = 0;
    for (Type::Types::const_iterator t = types.begin();
        t != types.end(); ++t, ++i)
    {
        PyTuple_SET_ITEM(result, i, PyString_FromString((*t)->getName()));
    }

    return result;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_getTypeInfo(PyObject* self, PyObject* args)
{
    char* type_name = 0;
    if (!PyArg_ParseTuple(args, "s:getTypeInfo", &type_name))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    const Type* type = Kernel::instance()->findType(type_name);
    if (0 == type)
        return PyErr_Format(PyExc_TypeError,
            "there're no exist specified name of type \'%s\'", type_name);

    PyObject* result = PyTuple_New(3);

    // name
    PyTuple_SET_ITEM(result, 0, PyString_FromString(type->getName()));
    // abstract
    PyTuple_SET_ITEM(result, 1, PyBool_FromLong(type->isAbstract()));
    // module name
    PyTuple_SET_ITEM(result, 2, PyString_FromString(
        Kernel::instance()->findModuleByTypeName(type_name)->getName()));

    return result;
}


//-----------------------------------------------------------------------------
PyObject* bzmagPy_isNode(PyObject* self, PyObject* args)
{
    // parse argument
    char* type_name = 0;
    if (!PyArg_ParseTuple(args, "s:isNode", &type_name))
        return PyErr_Format(PyExc_Exception, "mismatch argument");

    // 타입을 조사해서 Node 에서 파생된 것이 아니면 Object에서 파생된 것으로 간주
    // Object와 Node의 생성방식은 다름
    // 특히 Object는 커널에서 생성할수 없으므로 주의!
    const Type* type = Kernel::instance()->findType(type_name);
    if (0 == type)
        return PyErr_Format(PyExc_TypeError,
            "there're no exist specified name of type \'%s\'", type_name);

    return type->isKindOf("Node") ? Py_True : Py_False;
}
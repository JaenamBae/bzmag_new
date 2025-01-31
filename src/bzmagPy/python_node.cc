#include "python_extension.h"

#include "core/typeid.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
PyObject* bzNode_new(PyTypeObject* type, PyObject* args, PyObject* keys)
{
    bzNode* o = reinterpret_cast<bzNode*>(type->tp_alloc(type, 0));
    return reinterpret_cast<PyObject*>(o);
}


//-----------------------------------------------------------------------------
void bzNode_dealloc(PyObject* self)
{
    bzNode* o = reinterpret_cast<bzNode*>(self);
    o->node_.release();
    self->ob_type->tp_free(self);
}


//-----------------------------------------------------------------------------
PyObject* bzNode_repr(bzNode* self)
{
    return PyString_FromString((*self->node_)->getAbsolutePath());
}


//-----------------------------------------------------------------------------
PyObject* bzNode_getattro(bzNode* self, PyObject* name)
{
    String attr_name(PyString_AsString(name));
    Type* type = self->node_->getType();

    // find method
    Method* method = type->findMethod(attr_name);
    if (method)
    {
        self->method_ = method;
        Py_INCREF(self);
        Py_INCREF(name);
        return (PyObject*)(self);
    }
    // find property
    else
    {
        Property* prop = type->findProperty(attr_name);
        if (prop)
            return build_property_to_pyobject(self->node_, prop);
    }

    return PyErr_Format(PyExc_Exception,
        "\'%s\' Node has no attribute \'%s\'",
        self->node_->getAbsolutePath().c_str(), attr_name.c_str());
}


//-----------------------------------------------------------------------------
PyObject* bzNode_call(PyObject* self, PyObject* args, PyObject* keys)
{
    bzNode* o = reinterpret_cast<bzNode*>(self);
    if (o->node_.invalid())
        return PyErr_Format(PyExc_Exception, "invalid bzNode");
    return invoke_method(
        o->node_, o->method_, args, keys);
}


//-----------------------------------------------------------------------------
int bzNode_setattro(bzNode* self, PyObject* name, PyObject* value)
{
    if (self->node_.valid())
    {
        set_property(self->node_, self->node_->getAbsolutePath(), name, value);
    }
    return 0;
}

//-----------------------------------------------------------------------------
PyTypeObject bzNodeType =
{
    PyVarObject_HEAD_INIT(NULL, 0)          // ob_size
    "bzNode",                               // tp_name
    sizeof(bzNode),                         // tp_basicssize
    0,                                      // tp_itemsize
    bzNode_dealloc,                         // tp_dealloc
    0,                                      // tp_print
    (getattrfunc)0,                         // tp_getattr
    (setattrfunc)0,                         // tp_setattr
    0,                                      // tp_compare
    0,                                      // tp_repr
    0,                                      // tp_as_number
    0,                                      // tp_as_mapping
    0,                                      // tp_as_sequence
    0,                                      // tp_as_hash
    (ternaryfunc)bzNode_call,               // tp_call
    (reprfunc)bzNode_repr,                  // tp_str
    (getattrofunc)bzNode_getattro,          // tp_getattro
    (setattrofunc)bzNode_setattro,          // tp_setattro
    0,                                      // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                     // tp_flags
    "bzmagPy Node",                         // tp_doc
    0,                                      // tp_traverse
    0,                                      // tp_clear
    0,                                      // tp_richcompare
    0,                                      // tp_weaklistoffset
    0,                                      // tp_iter
    0,                                      // tp_iternext
    0,                                      // tp_methods,
    0,                                      // tp_members
    0,                                      // tp_getset
    0,                                      // tp_base
    0,                                      // tp_dict
    0,                                      // tp_descr_get
    0,                                      // tp_descr_set
    0,                                      // tp_dictoffset
    0,                                      // tp_init
    0,                                      // tp_alloc
    bzNode_new,                             // tp_new
};

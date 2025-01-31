#include "python_extension.h"

#include "core/typeid.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
PyObject* bzObject_new(PyTypeObject* type, PyObject* args, PyObject* keys)
{
    bzObject* o = reinterpret_cast<bzObject*>(type->tp_alloc(type, 0));
    return reinterpret_cast<PyObject*>(o);
}


//-----------------------------------------------------------------------------
void bzObject_dealloc(PyObject* self)
{
    bzObject* o = reinterpret_cast<bzObject*>(self);
    if (o->createdByPython_)
        delete o->object_;
    self->ob_type->tp_free(self);
}


//-----------------------------------------------------------------------------
PyObject* bzObject_getattro(bzObject* self, PyObject* name)
{
    String attr_name(PyString_AsString(name));
    Type* type = self->object_->getType();

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
            return build_property_to_pyobject(self->object_, prop);
    }

    return PyErr_Format(PyExc_Exception,
        "\'%p\' Object has no attribute \'%s\'",
        self->object_, attr_name.c_str());
}


//-----------------------------------------------------------------------------
PyObject* bzObject_call(PyObject* self, PyObject* args, PyObject* keys)
{
    bzObject* o = reinterpret_cast<bzObject*>(self);
    if (0 == o->object_)
        return PyErr_Format(PyExc_Exception, "invalid bzObject");
    return invoke_method(o->object_, o->method_, args, keys);
}


//-----------------------------------------------------------------------------
int bzObject_setattro(bzObject* self, PyObject* name, PyObject* value)
{
    if (self->object_)
    {
        set_property(self->object_, "", name, value);
    }
    return 0;
}



//-----------------------------------------------------------------------------
PyTypeObject bzObjectType =
{
    PyVarObject_HEAD_INIT(NULL, 0)          // ob_size
    "bzObject",                             // tp_name
    sizeof(bzObject),                       // tp_basicssize
    0,                                      // tp_itemsize
    bzObject_dealloc,                       // tp_dealloc
    0,                                      // tp_print
    (getattrfunc)0,                         // tp_getattr
    (setattrfunc)0,                         // tp_setattr
    0,                                      // tp_compare
    0,                                      // tp_repr
    0,                                      // tp_as_number
    0,                                      // tp_as_mapping
    0,                                      // tp_as_sequence
    0,                                      // tp_as_hash
    (ternaryfunc)bzObject_call,             // tp_call
    0,                                      // tp_str
    (getattrofunc)bzObject_getattro,        // tp_getattro
    (setattrofunc)bzObject_setattro,        // tp_setattro
    0,                                      // tp_as_buffer
    Py_TPFLAGS_DEFAULT,                     // tp_flags
    "bzmagPy Object",                       // tp_doc
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
    bzObject_new,                           // tp_new
};

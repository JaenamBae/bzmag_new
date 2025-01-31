#ifndef BZMAG_PYTHON_PYTHONFUNC_H
#define BZMAG_PYTHON_PYTHONFUNC_H

#include <python.h>
#include "core/ref.h"
#include "core/node.h"

#if PY_MAJOR_VERSION >= 3
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#define PyInt_FromLong PyLong_FromLong
#define PyInt_FromSize_t PyLong_FromSize_t
#define PyString_Check PyUnicode_Check
#define PyString_FromString PyUnicode_FromString
#define PyString_FromStringAndSize PyUnicode_FromStringAndSize
#if PY_VERSION_HEX < 0x03030000
#error "Python 3.0 - 3.2 are not supported."
#else
#define PyString_AsString(ob) \
    (PyUnicode_Check(ob)? PyUnicode_AsUTF8(ob): PyBytes_AsString(ob))
#define PyString_AsStringAndSize(ob, charpp, sizep) \
    (PyUnicode_Check(ob)? \
       ((*(charpp) = PyUnicode_AsUTF8AndSize(ob, (sizep))) == NULL? -1: 0): \
       PyBytes_AsStringAndSize(ob, (charpp), (sizep)))
#endif
#endif


struct module_state {
	PyObject *error;
};


#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

//-----------------------------------------------------------------------------
extern PyObject* build_property_to_pyobject
(bzmag::Object* obj, bzmag::Property* prop);
extern PyObject* build_variable_to_pyobject(bzmag::Variable* variable);
extern PyObject* build_input_paramter
(bzmag::Variables* v, PyObject* args, const char* method_name);
extern int set_property
(bzmag::Object* object, const bzmag::Path& phat,
 PyObject* name, PyObject* value);
extern PyObject* invoke_method
(bzmag::Object* object, bzmag::Method* method, PyObject* args, PyObject* keys);

//-----------------------------------------------------------------------------
struct bzObject
{
    PyObject_HEAD
    bzmag::Object* object_;
    bzmag::Method* method_;
    bool createdByPython_;
    
    bzObject():method_(0), createdByPython_(false) {}
};

extern PyTypeObject bzObjectType;

extern PyObject* bzmagPy_newobject(PyObject* self, PyObject* args);

//-----------------------------------------------------------------------------
struct bzNode
{
    PyObject_HEAD
    bzmag::Ref<bzmag::Node> node_;
    bzmag::Method* method_;

    bzNode():method_(0) {}
};

typedef std::map<bzmag::Path, bzNode*> bzNodes;

extern PyObject* g_module;
extern PyTypeObject bzNodeType;

extern PyObject* bzmagPy_new(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_get(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getObjectByID(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getNodeByID(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_delete(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_ls(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_pushCwn(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_popCwn(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_print(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_exit(PyObject* self, PyObject* args);
// extern PyObject* bzmagPy_serialize(PyObject* self, PyObject* args);
// extern PyObject* bzmagPy_deserialize(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getModuleList(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getTypeList(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getDerivedTypes(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_getTypeInfo(PyObject* self, PyObject* args);
extern PyObject* bzmagPy_isNode(PyObject* self, PyObject* args);

#endif // BZMAG_PYTHON_PYTHONFUNC_H

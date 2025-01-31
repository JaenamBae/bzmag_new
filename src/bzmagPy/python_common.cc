#include "python_extension.h"

#include "core/autoreleasepool.h"
#include "core/vector2.h"
#include "core/color.h"
#include "core/uri.h"
#include "core/simpleproperty.h"
#include "core/enumproperty.h"
#include "core/structproperty.h"
#include "core/compositeproperty.h"
#include "core/containerproperty.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
int bzmagPy_toint(PyObject* value)
{
    if (PyInt_Check(value))
        return PyInt_AsLong(value);
    else if (PyFloat_Check(value))
        return static_cast<int>(PyFloat_AsDouble(value));
    return 0;
}


//-----------------------------------------------------------------------------
// property type
PyObject* build_property_to_pyobject(bzmag::Object* obj, bzmag::Property* prop)
{
    if (TypeId<bool>::check(prop->getType()))
    {
        return Py_BuildValue("b",
            reinterpret_cast<SimpleProperty<bool>*>(prop)->get(obj));
    }
    else if (TypeId<int16>::check(prop->getType()))
    {
        return Py_BuildValue("h",
            reinterpret_cast<SimpleProperty<int16>*>(prop)->get(obj));
    }
    else if (TypeId<uint16>::check(prop->getType()))
    {
        return Py_BuildValue("H",
            reinterpret_cast<SimpleProperty<uint16>*>(prop)->get(obj));
    }
    else if (TypeId<int>::check(prop->getType()))
    {
        return Py_BuildValue("i",
            reinterpret_cast<SimpleProperty<int>*>(prop)->get(obj));
    }
    else if (TypeId<unsigned int>::check(prop->getType()))
    {
        return Py_BuildValue("I",
            reinterpret_cast<SimpleProperty<unsigned int>*>(prop)->get(obj));
    }
    else if (TypeId<int32>::check(prop->getType()))
    {
        return Py_BuildValue("l",
            reinterpret_cast<SimpleProperty<int32>*>(prop)->get(obj));
    }
    else if (TypeId<uint32>::check(prop->getType()))
    {
        return Py_BuildValue("k",
            reinterpret_cast<SimpleProperty<uint32>*>(prop)->get(obj));
    }
    else if (TypeId<int64>::check(prop->getType()))
    {
        return Py_BuildValue("L",
            reinterpret_cast<SimpleProperty<int64>*>(prop)->get(obj));
    }
    else if (TypeId<uint64>::check(prop->getType()))
    {
        return Py_BuildValue("K",
            reinterpret_cast<SimpleProperty<uint64>*>(prop)->get(obj));
    }

    else if (TypeId<float32>::check(prop->getType()))
    {
        return Py_BuildValue("f",
            reinterpret_cast<SimpleProperty<float32>*>(prop)->get(obj));
    }
    else if (TypeId<float64>::check(prop->getType()))
    {
        return Py_BuildValue("d",
            reinterpret_cast<SimpleProperty<float64>*>(prop)->get(obj));
    }
    else if (TypeId<String>::check(prop->getType()))
    {
        return Py_BuildValue("s",
            reinterpret_cast<SimpleProperty
            <const String&>*>(prop)->get(obj).c_str());
    }
    else if (TypeId<Uri>::check(prop->getType()))
    {
        return Py_BuildValue("s",
            reinterpret_cast<SimpleProperty
            <const Uri&>*>(prop)->get(obj).c_str());
    }
    else if (TypeId<Vector2>::check(prop->getType()))
    {
        const Vector2* v = &reinterpret_cast<
            SimpleProperty<const Vector2&>*>(prop)->get(obj);
        return Py_BuildValue("(d, d)", v->x_, v->y_);
    }
    else if (TypeId<Color>::check(prop->getType()))
    {
        const Color* c = &reinterpret_cast<
            SimpleProperty<const Color&>*>(prop)->get(obj);
        return Py_BuildValue("(i, i, i, i)", c->r_, c->g_, c->b_, c->a_);
    }
    else if (TypeId<Object*>::check(prop->getType()))
    {
        Object* object = reinterpret_cast<SimpleProperty<Object*>*>(prop)->get(obj);
        if (0 == object)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        bzObject* o = reinterpret_cast<bzObject*>(
            bzObjectType.tp_new(&bzObjectType, 0, 0));
        if (0 == o)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        o->object_ = object;
        o->createdByPython_ = true;
        return reinterpret_cast<PyObject*>(o);
    }
    else if (TypeId<Node*>::check(prop->getType()))
    {
        Node* node = reinterpret_cast<SimpleProperty<Node*>*>(prop)->get(obj);
        if (0 == node)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        bzNode* o = reinterpret_cast<bzNode*>(
            bzNodeType.tp_new(&bzNodeType, 0, 0));
        if (0 == o)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        o->node_ = node;
        return reinterpret_cast<PyObject*>(o);
    }


    return 0;
}


//-----------------------------------------------------------------------------
// Output parameter for methods
PyObject* build_variable_to_pyobject(Variable* variable)
{
    if (TypeId<bool>::check(variable->getType()))
    {
        return Py_BuildValue("b",
            reinterpret_cast<SimpleVariable<bool>*>(variable)->get());
    }
    else if (TypeId<int16>::check(variable->getType()))
    {
        return Py_BuildValue("h",
            reinterpret_cast<SimpleVariable<int16>*>(variable)->get());
    }
    else if (TypeId<uint16>::check(variable->getType()))
    {
        return Py_BuildValue("H",
            reinterpret_cast<SimpleVariable<uint16>*>(variable)->get());
    }
    else if (TypeId<int>::check(variable->getType()))
    {
        return Py_BuildValue("i",
            reinterpret_cast<SimpleVariable<int>*>(variable)->get());
    }
    else if (TypeId<unsigned int>::check(variable->getType()))
    {
        return Py_BuildValue("I",
            reinterpret_cast<SimpleVariable<unsigned int>*>(variable)->get());
    }
    else if (TypeId<int32>::check(variable->getType()))
    {
        return Py_BuildValue("l",
            reinterpret_cast<SimpleVariable<int32>*>(variable)->get());
    }
    else if (TypeId<uint32>::check(variable->getType()))
    {
        return Py_BuildValue("k",
            reinterpret_cast<SimpleVariable<uint32>*>(variable)->get());
    }
    else if (TypeId<int64>::check(variable->getType()))
    {
        return Py_BuildValue("L",
            reinterpret_cast<SimpleVariable<int64>*>(variable)->get());
    }
    else if (TypeId<uint64>::check(variable->getType()))
    {
        return Py_BuildValue("K",
            reinterpret_cast<SimpleVariable<uint64>*>(variable)->get());
    }

    else if (TypeId<float32>::check(variable->getType()))
    {
        return Py_BuildValue("f",
            reinterpret_cast<SimpleVariable<float32>*>(variable)->get());
    }
    else if (TypeId<float64>::check(variable->getType()))
    {
        return Py_BuildValue("d",
            reinterpret_cast<SimpleVariable<float64>*>(variable)->get());
    }
    else if (TypeId<String>::check(variable->getType()))
    {
        return Py_BuildValue("s",
            reinterpret_cast<SimpleVariable<String>*>(variable)->get().c_str());
    }
    else if (TypeId<Uri>::check(variable->getType()))
    {
        return Py_BuildValue("s",
            reinterpret_cast<SimpleVariable<Uri>*>(variable)->get().c_str());
    }
    else if (TypeId<Vector2>::check(variable->getType()))
    {
        Vector2* v = &reinterpret_cast<SimpleVariable<Vector2>*>(variable)->get();
        return Py_BuildValue("(d, d)", v->x_, v->y_);
    }
    else if (TypeId<Color>::check(variable->getType()))
    {
        Color* c = &reinterpret_cast<SimpleVariable<Color>*>(variable)->get();
        return Py_BuildValue("(i, i, i, i)", c->r_, c->g_, c->b_, c->a_);
    }
    else if (TypeId<Object*>::check(variable->getType()))
    {
        Object* object = reinterpret_cast<SimpleVariable<Object*>*>(variable)->get();
        if (0 == object)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        bzObject* o = reinterpret_cast<bzObject*>(
            bzObjectType.tp_new(&bzObjectType, 0, 0));
        if (0 == o)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        o->object_ = object;
        o->createdByPython_ = true;
        return reinterpret_cast<PyObject*>(o);
    }
    else if (TypeId<Node*>::check(variable->getType()))
    {
        Node* node = reinterpret_cast<SimpleVariable<Node*>*>(variable)->get();
        if (0 == node)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        bzNode* o = reinterpret_cast<bzNode*>(
            bzNodeType.tp_new(&bzNodeType, 0, 0));
        if (0 == o)
        {
            Py_INCREF(Py_None);
            return Py_None;
        }
        o->node_ = node;
        return reinterpret_cast<PyObject*>(o);
    }
    return 0;
}


//-----------------------------------------------------------------------------
// input parameter for methods
PyObject* build_input_paramter
(Variables* v, PyObject* args, const char* method_name)
{
    uint32 num_args = static_cast<uint32>(PyTuple_Size(args));
    for (uint32 a = 0; a < num_args; ++a)
    {
        PyObject* arg = PyTuple_GET_ITEM(args, a);
        if (TypeId<bool>::check(v->get(a)->getType()))
        {
            typedef bool type;
            if (PyBool_Check(arg))
            {
                v->get<type>(a) = PyInt_AsLong(arg) ? true : false;
                continue;
            }
            else if (PyInt_Check(arg))
            {
                v->get<type>(a) = PyInt_AsLong(arg) ? true : false;
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = PyFloat_AsDouble(arg) ? true : false;
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'bool\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<int16>::check(v->get(a)->getType()))
        {
            typedef int16 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'int16\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<uint16>::check(v->get(a)->getType()))
        {
            typedef uint16 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'uint16\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<int>::check(v->get(a)->getType()))
        {
            typedef int type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'int\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<unsigned int>::check(v->get(a)->getType()))
        {
            typedef unsigned int type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'unsigned int\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<int32>::check(v->get(a)->getType()))
        {
            typedef int32 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'int32\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<uint32>::check(v->get(a)->getType()))
        {
            typedef uint32 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'uint32\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<int64>::check(v->get(a)->getType()))
        {
            typedef int64 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'int64\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<uint64>::check(v->get(a)->getType()))
        {
            typedef uint64 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'uint64\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<float32>::check(v->get(a)->getType()))
        {
            typedef float32 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'float32\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<float64>::check(v->get(a)->getType()))
        {
            typedef float64 type;
            if (PyInt_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyInt_AsLong(arg));
                continue;
            }
            else if (PyFloat_Check(arg))
            {
                v->get<type>(a) = static_cast<type>(PyFloat_AsDouble(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'float64\'",
                method_name, arg->ob_type->tp_name);
        }

        /*
        The changes in strings, Unicode and bytes are of course one of the biggest changes also when
        writing C extensions.In the C API, as with integers, there has been no renaming amongst the
        strings and the unicode type is still called unicode.The str type and all accompanying support
        functions are gone and the new bytestype has replaced it.
        This means that if your extension returns or handles binary data you will in Python 2 get and
        return PyString objects, while you in Python 3 will handle PyBytes objects.Where you handle
        text data you should in Python 2 accept both PyString and PyUnicode while in Python 3
        only PyUnicode is relevant.This can be handled with the same techniques as
        for int and long above, you can either make two versions of the code and choose between them
        with an #if, or you can redefine the missing PyString functions in Python 3 as
        either PyBytes or PyUnicode depending on what you need.
        */
        else if (TypeId<String>::check(v->get(a)->getType()))
        {
            if (PyString_Check(arg))
            {
                v->get<String>(a) = String("%s", PyString_AsString(arg));
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'String\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<Uri>::check(v->get(a)->getType()))
        {
            if (PyString_Check(arg))
            {
                v->get<Uri>(a) = String("%s", PyString_AsString(arg));
                continue;
            }
            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'Uri\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<Vector2>::check(v->get(a)->getType()))
        {
            if (PyTuple_Check(arg))
            {
                Vector2& tv = v->get<Vector2>(a).get();
                if (PyTuple_Size(arg) == 2)
                {
                    for (int i = 0; i < 2; ++i)
                    {
                        float value = 0;
                        PyObject* o = PyTuple_GET_ITEM(arg, i);
                        if (PyInt_Check(o))
                            value = static_cast<float>(PyInt_AsLong(o));
                        else if (PyFloat_Check(o))
                            value = static_cast<float>(PyFloat_AsDouble(o));
                        tv.a_[i] = value;
                    }
                    continue;
                }
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'Vector2\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<Color>::check(v->get(a)->getType()))
        {
            if (PyTuple_Check(arg))
            {
                Color& tc = v->get<Color>(a).get();
                if (PyTuple_Size(arg) == 4)
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        int value = 0;
                        PyObject* o = PyTuple_GET_ITEM(arg, i);
                        if (PyInt_Check(o))
                            value = PyInt_AsLong(o);
                        else if (PyFloat_Check(o))
                            value = static_cast<int>(PyFloat_AsDouble(o));
                        tc.array_[i] = value;
                    }
                    continue;
                }
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'Color\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<Object*>::check(v->get(a)->getType()))
        {
            if (PyObject_TypeCheck(arg, &bzObjectType))
            {
                bzObject* o = reinterpret_cast<bzObject*>(arg);
                v->get<Object*>(a) = o->object_;
                continue;
            }

            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'bzObject\'",
                method_name, arg->ob_type->tp_name);
        }
        else if (TypeId<Node*>::check(v->get(a)->getType()))
        {
            if (PyObject_TypeCheck(arg, &bzNodeType))
            {
                bzNode* o = reinterpret_cast<bzNode*>(arg);
                v->get<Node*>(a) = o->node_.get<Node*>();
                continue;
            }
            return PyErr_Format(PyExc_TypeError,
                "%s() : cannot convert parameter from \'%s\' to \'bzNode\'",
                method_name, arg->ob_type->tp_name);
        }
        else
        {
            return PyErr_Format(PyExc_TypeError,
                "%s() : not supported parameter type \'%s\'",
                method_name, arg->ob_type->tp_name);
        }
    }

    return Py_True;
}


//-----------------------------------------------------------------------------
// Bug Fix : 2017.09.23 
int set_property
(bzmag::Object* object, const Path& path, PyObject* name, PyObject* value)
{
    // find property
    String prop_name(PyString_AsString(name));
    Property* prop = object->getType()->findProperty(prop_name);

    if (0 == prop)
    {
        PyErr_Format(PyExc_AttributeError,
            "\'%s\' object has no property \'%s\'",
            path.c_str(), prop_name.c_str());
        return -1;
    }


    // set property
    if (TypeId<bool>::check(prop->getType()))
    {
        typedef SimpleProperty<bool> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyBool_Check(value))
            ap->set(object, Py_True == value ? true : false);
        else if (PyInt_Check(value))
            ap->set(object, PyInt_AsLong(value) ? true : false);
        else if (PyFloat_Check(value))
            ap->set(object, PyFloat_AsDouble(value) ? true : false);
        else if (PyString_Check(value))
        {
            String v(PyString_AsString(value));
            if (v == "true" || v == "True")
                ap->set(object, true);
            else
                ap->set(object, false);
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'bool\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<int16>::check(prop->getType()))
    {
        typedef int16 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'int16\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<uint16>::check(prop->getType()))
    {
        typedef uint16 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'uint16\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<int>::check(prop->getType()))
    {
        typedef int type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'int\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<unsigned int>::check(prop->getType()))
    {
        typedef unsigned int type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'unsigned int\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<int32>::check(prop->getType()))
    {
        typedef int32 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'int32\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<uint32>::check(prop->getType()))
    {
        typedef uint32 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'uint32\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<int64>::check(prop->getType()))
    {
        typedef int64 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'int64\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<uint64>::check(prop->getType()))
    {
        typedef uint64 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'uint64\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<float32>::check(prop->getType()))
    {
        typedef float32 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'float32\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<float64>::check(prop->getType()))
    {
        typedef float64 type;
        typedef SimpleProperty<type> AdaptiveProperty;
        AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
        if (PyFloat_Check(value))
            ap->set(object, static_cast<type>(PyFloat_AsDouble(value)));
        else if (PyInt_Check(value))
            ap->set(object, static_cast<type>(PyInt_AsLong(value)));
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'float64\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<String>::check(prop->getType()))
    {
        if (PyString_Check(value))
        {
            typedef SimpleProperty<const String&> AdaptiveProperty;
            AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
            ap->set(object, String("%s", PyString_AsString(value)));
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'String\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<Uri>::check(prop->getType()))
    {
        if (PyString_Check(value))
        {
            typedef SimpleProperty<const Uri&> AdaptiveProperty;
            AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
            ap->set(object, String("%s", PyString_AsString(value)).c_str());
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'Uri\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<Vector2>::check(prop->getType()))
    {
        if (PyTuple_Check(value))
        {
            typedef SimpleProperty<const Vector2&> AdaptiveProperty;
            AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
            Vector2 v;
            for (int i = 0; i < 2; ++i)
            {
                PyObject* e = PyTuple_GET_ITEM(value, i);
                if (PyInt_Check(e))
                    v.set(i, static_cast<float>(PyInt_AsLong(e)));
                else if (PyFloat_Check(e))
                    v.set(i, static_cast<float>(PyFloat_AsDouble(e)));
            }
            ap->set(object, v);
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'Vector2\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<Color>::check(prop->getType()))
    {
        if (PyTuple_Check(value))
        {
            typedef SimpleProperty<const Color&> AdaptiveProperty;
            AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
            Color v(
                bzmagPy_toint(PyTuple_GET_ITEM(value, 0)),
                bzmagPy_toint(PyTuple_GET_ITEM(value, 1)),
                bzmagPy_toint(PyTuple_GET_ITEM(value, 2)),
                bzmagPy_toint(PyTuple_GET_ITEM(value, 3)));
            ap->set(object, v);
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'Color\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<Object*>::check(prop->getType()))
    {
        int32 ID = 0;
        Object* o = nullptr;
        if (PyInt_Check(value)) {
            ID = PyInt_AsLong(value);
            if (ID != -1) {
                o = AutoReleasePool::instance()->find(ID);
            }
            if (o != nullptr || ID == -1)
            {
                typedef SimpleProperty<Object*> AdaptiveProperty;
                AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
                ap->set(object, o);
            }
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'Object\'",
                value->ob_type->tp_name);
            return -1;
        }
    }
    else if (TypeId<Node*>::check(prop->getType()))
    {
        int32 ID = 0;
        Object* o = nullptr;
        if (PyInt_Check(value)) {
            ID = PyInt_AsLong(value);
            if (ID != -1) {
                o = AutoReleasePool::instance()->find(ID);
            }
            if (o != nullptr || ID == -1)
            {
                typedef SimpleProperty<Node*> AdaptiveProperty;
                AdaptiveProperty* ap = static_cast<AdaptiveProperty*>(prop);
                Node* node = reinterpret_cast<Node*>(o);
                ap->set(object, node);
            }
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                "cannot convert value from \'%s\' to \'Node\'",
                value->ob_type->tp_name);
            return -1;
        }
    }


    return 0;
}


//-----------------------------------------------------------------------------
PyObject* invoke_method
(Object* object, Method* method, PyObject* args, PyObject* keys)
{
    // make input parameter
    uint32 num_args = static_cast<uint32>(PyTuple_Size(args));
    Parameter* param = method->getParameter();
    if (num_args != param->in()->size())
        return PyErr_Format(PyExc_TypeError,
            "%s() takes exactly %d argument (%d given)",
            method->getName().c_str(), param->in()->size(), num_args);
    if (!build_input_paramter(param->in(), args, method->getName()))
        return 0;

    // invoke method
    method->invoke(object);

    // make output parameter
    if (method->getParameter()->out()->isForceList())
    {
        PyObject* result = PyTuple_New(param->out()->size());
        for (uint32 a = 0; a < param->out()->size(); ++a)
        {
            PyTuple_SET_ITEM(result, a,
                build_variable_to_pyobject(param->out()->get(a)));
        }
        return result;
    }
    else
    {
        switch (param->out()->size())
        {
            // parameter size == 0
        case 0:
            Py_INCREF(Py_None);
            return Py_None;
            // parameter size == 1
        case 1:
            return build_variable_to_pyobject(param->out()->get(0));
            // parameter size > 1
        default:
        {
            PyObject* result = PyTuple_New(param->out()->size());
            for (uint32 a = 0; a < param->out()->size(); ++a)
            {
                PyTuple_SET_ITEM(result, a,
                    build_variable_to_pyobject(param->out()->get(a)));
            }
            return result;
        }
        }
    }
}

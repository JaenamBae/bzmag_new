#include <windows.h>
#include <iostream>
#include "python_extension.h"
#include "pythonscriptserver.h"
#include "core/kernel.h"
#include "core/module.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;

IMPLEMENT_CLASS(bzmagPyScriptServer, ScriptServer);

//-----------------------------------------------------------------------------
static PyMethodDef bzmagPyMethods[] =
{
    { "newobj",  bzmagPy_newobject, METH_VARARGS, "create new nonamed bzObject" },
    { "new",  bzmagPy_new, METH_VARARGS, "create new bzNode" },
    { "get",  bzmagPy_get, METH_VARARGS, "get TobObject from NOH" },
	{ "getObject",  bzmagPy_getObjectByID, METH_VARARGS, "get Object by ID" },
	{ "getNode",  bzmagPy_getNodeByID, METH_VARARGS, "get Node by ID" },
    { "delete",  bzmagPy_delete, METH_VARARGS, "delete bzNode" },
    { "ls",  bzmagPy_ls, METH_VARARGS, "list current work node" },
    { "pushcwn",  bzmagPy_pushCwn, METH_VARARGS, "push current work node" },
    { "popcwn",  bzmagPy_popCwn, METH_VARARGS, "pop current work node" },
    { "print",  bzmagPy_print, METH_VARARGS, "debug print" },
    { "exit",  bzmagPy_exit, METH_VARARGS, "exit application" },
//     { "serialize",  bzmagPy_serialize, METH_VARARGS, "serialize objects to resource" },
//     { "deserialize",  bzmagPy_deserialize, METH_VARARGS, "deserialize objects from resource" },
    { "getModuleList",  bzmagPy_getModuleList, METH_VARARGS, "get module list" },
    { "getTypeList",  bzmagPy_getTypeList, METH_VARARGS, "get type list specified module name" },
    { "getDerivedTypes",  bzmagPy_getDerivedTypes, METH_VARARGS, "get derived type list specified type name" },
    { "getTypeInfo",  bzmagPy_getTypeInfo, METH_VARARGS, "get type information specified type name" },
    { "isNode",  bzmagPy_isNode, METH_VARARGS, "specify the given type is kind of bzNodes" },
    { NULL, NULL, 0, NULL }        /* Sentinel */
};

//-----------------------------------------------------------------------------
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "bzmagPy",       // m_name
    "bzmagPy module",// m_doc
    -1,              // m_size
    bzmagPyMethods,  // m_methods
    NULL,            // m_reload
    NULL,            // m_traverse 
    NULL,            // m_clear
    NULL             // m_free 
};

//-----------------------------------------------------------------------------
unsigned long long bzmagPyScriptServer::initialize()
{
    PyStatus status;
    PyConfig config;
    PyConfig_InitPythonConfig(&config);

    // 프로그램 이름 설정
    status = PyConfig_SetString(&config, &config.program_name, L"bzmagPy");
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 0;
    }

    // Python 초기화 수행
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        Py_ExitStatusException(status);
        return 0;
    }

    PyConfig_Clear(&config);

    // bzmagPy 모듈 생성
    g_module = PyModule_Create(&moduledef);
    if (!g_module) {
        return 0;
    }

    // 예외 객체 초기화
    PyObject* error_obj = PyErr_NewException("bzmagPy.error", NULL, NULL);
    if (!error_obj || PyModule_AddObject(g_module, "error", error_obj) < 0) {
        Py_XDECREF(error_obj);
        Py_XDECREF(g_module);
        return 0;
    }

    // bzObject 타입 초기화
    if (PyType_Ready(&bzObjectType) < 0 || PyModule_AddObject(g_module, "bzObject", (PyObject*)&bzObjectType) < 0) {
        Py_DECREF(g_module);
        return 0;
    }
    Py_INCREF(&bzObjectType);

    // bzNode 타입 초기화
    if (PyType_Ready(&bzNodeType) < 0 || PyModule_AddObject(g_module, "bzNode", (PyObject*)&bzNodeType) < 0) {
        Py_DECREF(&bzObjectType);
        Py_DECREF(g_module);
        return 0;
    }
    Py_INCREF(&bzNodeType);

    return (unsigned long long)g_module;
}

//-----------------------------------------------------------------------------
bzmagPyScriptServer::bzmagPyScriptServer()
{
    // Test to see if we are running inside an existing Python interpreter
    if (!Py_IsInitialized())
    {
        Py_Initialize();
        initialize();
    }
}


//-----------------------------------------------------------------------------
bzmagPyScriptServer::~bzmagPyScriptServer()
{
    Py_DECREF(&bzObjectType);
    Py_DECREF(&bzNodeType);
    //PyErr_Clear();
}


//-----------------------------------------------------------------------------
bool bzmagPyScriptServer::run(const String& str, String* result)
{
    // Python 예외 상태 초기화
    if (PyErr_Occurred()) {
        PyErr_Clear();
    }

    // Python 출력 리다이렉션 설정
    PyObject* sysModule = PyImport_ImportModule("sys");
    PyObject* ioModule = PyImport_ImportModule("io");
    if (!sysModule || !ioModule) {
        if (result) {
            *result = "Failed to import Python modules (sys, io)";
        }
        return false;
    }

    PyObject* stringIO = PyObject_CallMethod(ioModule, "StringIO", nullptr);
    if (!stringIO) {
        if (result) {
            *result = "Failed to create StringIO object";
        }
        Py_DECREF(sysModule);
        Py_DECREF(ioModule);
        return false;
    }

    PyObject_SetAttrString(sysModule, "stdout", stringIO);
    PyObject_SetAttrString(sysModule, "stderr", stringIO);

    // Python 코드 실행
    if (PyRun_SimpleString(str.c_str()) == -1) {
        // Python 예외 상태 확인
        if (PyErr_Occurred()) {
            PyObject* pType, * pValue, * pTraceback;
            PyErr_Fetch(&pType, &pValue, &pTraceback);
            PyErr_NormalizeException(&pType, &pValue, &pTraceback);

            PyObject* tracebackModule = PyImport_ImportModule("traceback");
            if (tracebackModule) {
                PyObject* formatException = PyObject_GetAttrString(tracebackModule, "format_exception");
                if (formatException && PyCallable_Check(formatException)) {
                    PyObject* args = PyTuple_Pack(3, pType, pValue, pTraceback);
                    PyObject* formattedList = PyObject_CallObject(formatException, args);
                    Py_DECREF(args);

                    if (formattedList) {
                        PyObject* formattedStr = PyUnicode_Join(PyUnicode_FromString(""), formattedList);
                        if (formattedStr) {
                            const char* errorMessage = PyUnicode_AsUTF8(formattedStr);
                            if (result) {
                                *result = "Python Error:\n" + String(errorMessage);
                            }
                            Py_DECREF(formattedStr);
                        }
                        Py_DECREF(formattedList);
                    }
                }
                Py_XDECREF(formatException);
                Py_DECREF(tracebackModule);
            }

            Py_XDECREF(pType);
            Py_XDECREF(pValue);
            Py_XDECREF(pTraceback);
        }
        else {
            // `PyErr_Occurred()`가 false일 때 stderr에서 출력 가져오기
            PyObject* output = PyObject_CallMethod(stringIO, "getvalue", nullptr);
            if (output) {
                const char* outputStr = PyUnicode_AsUTF8(output);
                if (result) {
                    *result = outputStr ? String(outputStr) : "Unknown error occurred.";
                }
                Py_DECREF(output);
            }
            else {
                if (result) {
                    *result = "Failed to retrieve error message from stderr.";
                }
            }
        }

        Py_DECREF(stringIO);
        Py_DECREF(sysModule);
        Py_DECREF(ioModule);
        return false;
    }

    // Python 실행 성공 시 출력 가져오기
    PyObject* output = PyObject_CallMethod(stringIO, "getvalue", nullptr);
    if (output) {
        const char* outputStr = PyUnicode_AsUTF8(output);
        if (result) {
            *result = outputStr ? String(outputStr) : "";
        }
        Py_DECREF(output);
    }

    // 리다이렉션 복원
    PyObject_SetAttrString(sysModule, "stdout", Py_None);
    PyObject_SetAttrString(sysModule, "stderr", Py_None);

    Py_DECREF(stringIO);
    Py_DECREF(sysModule);
    Py_DECREF(ioModule);

    return true;
}

//-----------------------------------------------------------------------------
bool bzmagPyScriptServer::call(const String& str, Parameter* parameter)
{
    return false;
}


//-----------------------------------------------------------------------------
bool bzmagPyScriptServer::runFile(const Uri& uri, String* result)
{
    return false;
}



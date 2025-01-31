#include <python.h>
#include "core/define.h"
#include "core/kernel.h"
#include "core/module.h"
#include "pythonscriptserver.h"
#include "platform.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
PyObject* g_module = 0;

//-----------------------------------------------------------------------------
void initialize_bzmagPy(Module* module)
{
    REGISTER_TYPE(module, bzmagPyScriptServer);
    bzmagPyScriptServer::setSingletonPath("/sys/python");
}


//-----------------------------------------------------------------------------
void finalize_bzmagPy(Module* module)
{
    //PyRun_SimpleString("import gc; gc.collect()");  // 가비지 컬렉션 강제 실행

    // Python 종료
    if (Py_IsInitialized()) {
        Py_Finalize();
    }
}

//-----------------------------------------------------------------------------
DECLARE_MODULE(PYLIBLIBRARY_API, bzmagPy);


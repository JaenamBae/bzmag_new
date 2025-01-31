#include "scriptserver.h"

#include "methodbinder.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
static void ScriptServer_bs_run_s(ScriptServer* self, Parameter* param)
{
    String result;
    param->out()->get<bool>(0) =
        self->run(param->in()->get<String>(0).get(),
        &result);
    param->out()->get<String>(1) = result;
}


//-----------------------------------------------------------------------------
static void ScriptServer_bs_runFile_s(ScriptServer* self, Parameter* param)
{
    String result;
    param->out()->get<bool>(0) =
        self->runFile(param->in()->get<String>(0).get(),
        &result);
    param->out()->get<String>(1) = result;
}


//-----------------------------------------------------------------------------
void ScriptServer::bindMethod()
{
    BIND_METHOD(bs_run_s, ScriptServer_bs_run_s);
    BIND_METHOD(bs_runFile_s, ScriptServer_bs_runFile_s);
}

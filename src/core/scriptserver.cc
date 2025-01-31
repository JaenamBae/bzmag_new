#include "scriptserver.h"

using namespace bzmag;

IMPLEMENT_CLASS(ScriptServer, Node);

//-----------------------------------------------------------------------------
bool ScriptServer::run(const String& str, String* result)
{
    return false;
}

//-----------------------------------------------------------------------------
bool ScriptServer::call(const String& str, Parameter* parameter)
{
    return false;
}


//-----------------------------------------------------------------------------
bool ScriptServer::runFile(const Uri& uri, String* result)
{
    return false;
}

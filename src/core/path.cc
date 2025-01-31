#include "path.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Path::Path() :
    String("/"), tokenizer_(String::c_str(), '/')
{
    // empty
}


//-----------------------------------------------------------------------------
Path::Path(const char* path) :
    String(path), tokenizer_(String::c_str(), '/')
{
    // empty
}


//-----------------------------------------------------------------------------
Path::Path(const wchar_t* path) :
    String(path), tokenizer_(String::c_str(), '/')
{
    // empty
}


//-----------------------------------------------------------------------------
Path::Path(const Path& path) :
    String(path), tokenizer_(String::c_str(), '/')
{
    // empty
}


//-----------------------------------------------------------------------------
Path::Path(const String& path) :
    String(path), tokenizer_(String::c_str(), '/')
{
    // empty
}


#include "uri.h"

#include <list>
#include "define.h"
#include "assert.h"
// #include "resourcemanager.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Uri::Uri()
{
    // empty
}


//-----------------------------------------------------------------------------
Uri::Uri(const char* uri)
{
    set(uri);
}


//-----------------------------------------------------------------------------
Uri::Uri(const String& uri)
{
    set(uri.c_str());
}


//-----------------------------------------------------------------------------
void Uri::clear()
{
    uri_.clear();
    protocol_.clear();
    managed_.clear();
    ip_.clear();
    port_.clear();
    path_.clear();
}


//-----------------------------------------------------------------------------
bool Uri::empty() const
{
    return uri_.empty();
}


//-----------------------------------------------------------------------------
size_t Uri::size() const
{
    return uri_.size();
}


//-----------------------------------------------------------------------------
void Uri::set(const char* uri)
{
    protocol_.clear();
    ip_.clear();
    port_.clear();
    managed_.clear();
    path_.clear();

    uri_ = uri;

    size_t s = 0;
    // find protocol type
    size_t e = uri_.find("://");
    // not found protocol
    if (-1 == e)
        return;
    protocol_.assign(uri_, 0, e);
    s = e + 3;

    if (protocol_ == "managed")
    {
        // find managed name(split by '#')
        e = uri_.find("#", s);
        managed_.assign(uri_, s, e - s);
        // if found managed name then assign path
        if (-1 != e)
        {
            s = e + 1;
            if (s < uri_.size())
                path_.assign(uri_, s, -1);
        }
    }
    else if (protocol_ == "file")
    {
        path_.assign(uri_, s, -1);
    }
    else if (protocol_ == "http")
    {
        // find port
        e = uri_.find(":", s);
        if (-1 == e)
        {
            // ip only
            e = uri_.find("/", s);
            ip_.assign(uri_, s, e - s);
        }
        else
        {
            // ip + port
            ip_.assign(uri_, s, e - s);
            s = e + 1;
            e = uri_.find("/", s);
            port_.assign(uri_, s, e - s);
        }

        if (-1 != e)
            path_.assign(uri_, e + 1, -1);
    }
    else
    {
        // no available protocol
        return;
    }
}


//-----------------------------------------------------------------------------
String Uri::extractPath() const
{
    size_t s = path_.rfind("/");
    if (-1 == s)
        return String();
    return String(path_, 0, s);
}

//-----------------------------------------------------------------------------
String Uri::extractFile() const
{
    size_t s = path_.rfind("/");
    // 찾지 못했다면 path 가 File
    if (-1 == s)
        return path_;
    return String(path_, s + 1, -1);
}


//-----------------------------------------------------------------------------
String Uri::extractFileName() const
{
    size_t e = path_.rfind(".");
    size_t s = path_.rfind("/", e);
    return String(path_, s + 1, e - s - 1);
}


//-----------------------------------------------------------------------------
String Uri::extractExtension() const
{
    size_t s = path_.rfind(".");
    return String(path_, s + 1, -1);
}


//-----------------------------------------------------------------------------
void Uri::setProtocol(const char* protocol)
{
    protocol_ = protocol;
}


//-----------------------------------------------------------------------------
void Uri::setPackage(const char* managed)
{
    managed_ = managed;
}


//-----------------------------------------------------------------------------
void Uri::setIp(const char* ip)
{
    ip_ = ip;
}


//-----------------------------------------------------------------------------
void Uri::setPort(const char* port)
{
    port_ = port;
}


//-----------------------------------------------------------------------------
void Uri::setPath(const char* path)
{
    path_ = path;
}

//-----------------------------------------------------------------------------
// String Uri::makePhysicalAbsolutePath() const
// {
//     String apath;
// #ifdef _WIN32
// 
//     char cwd[MAX_PATH];
//     apath += bzmag_getcwd(cwd, 256);
//     apath += "/" + ResourceManager::instance()->getBasePath();
// 
//     apath += "/" + managed_;
//     make_absolutepath(&apath);
//     apath += "/" + path_;
// 
// #endif
// 
//     return apath;
// }


//-----------------------------------------------------------------------------
void Uri::make_absolutepath(String* path) const
{
    size_t nbegin = 0;
    std::list<String> pathes;
    for (size_t i = 0; i < path->size(); ++i)
    {
        if (path->at(i) == '/' ||
            path->at(i) == '\\' &&
            nbegin != i)
        {
            String name(path->substr(nbegin, i - nbegin));
            nbegin = i + 1;
            if (name == ".")
                continue;
            else if (name == "..")
                pathes.pop_back();
            else
                pathes.push_back(name);
        }
    }
    pathes.push_back(path->substr(nbegin, -1));

    *path = "";
    while (pathes.size())
    {
        *path += pathes.front();
        pathes.pop_front();
        if (pathes.size() > 0)
            *path += "/";
    }
}
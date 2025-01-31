#include "singletonserver.h"

#include "singleton.h"
#include "assert.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
SingletonServer SingletonServer::s_instance_;

//-----------------------------------------------------------------------------
SingletonServer::SingletonServer()
{

}


//-----------------------------------------------------------------------------
SingletonServer::~SingletonServer()
{
    clear();
}


//-----------------------------------------------------------------------------
void SingletonServer::clear()
{
    singletons_.reverse();

    for (Singletons::iterator i = singletons_.begin();
         i != singletons_.end();)
    {
        SingletonBase* s = *i;
        i = singletons_.erase(i);
        s->releaseSingleton();
    }
}

//-----------------------------------------------------------------------------
void SingletonServer::registerSingleton(SingletonBase* singleton)
{
    singleton->addRefSingleton();
    singletons_.push_back(singleton);
}


//-----------------------------------------------------------------------------
void SingletonServer::unregisterSingleton(SingletonBase* singleton)
{
    singletons_.remove(singleton);
    if (singleton->getRefSingleton() > 0)
        singleton->releaseSingleton();
}

//-----------------------------------------------------------------------------
SingletonServer* SingletonServer::instance()
{
    return &s_instance_;
}

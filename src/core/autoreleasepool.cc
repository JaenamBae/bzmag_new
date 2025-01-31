#include "autoreleasepool.h"
#include "object.h"

namespace bzmag
{

//-----------------------------------------------------------------------------
AutoReleasePool::AutoReleasePool()
{
    
}

//-----------------------------------------------------------------------------
AutoReleasePool::~AutoReleasePool()
{
    pump();
}


//-----------------------------------------------------------------------------
void AutoReleasePool::add(Object* o)
{
    pool[o->getID()] = o;
}


//-----------------------------------------------------------------------------
void AutoReleasePool::remove(Object* o)
{
    pool.erase(o->getID());
}

//-----------------------------------------------------------------------------
Object* AutoReleasePool::find(uint32 ID)
{
    if( pool.find(ID) != pool.end())
        return pool[ID];
    else
        return nullptr;
}


//-----------------------------------------------------------------------------
void AutoReleasePool::pump()
{
    pool.clear();
}

}

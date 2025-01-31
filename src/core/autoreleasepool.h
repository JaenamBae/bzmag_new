#ifndef BZMAG_CORE_UTILITY_AUTORELEASEPOOL_H
#define BZMAG_CORE_UTILITY_AUTORELEASEPOOL_H
/**
@ingroup bzmagCoreUtility
@class bzmag::AutoReleasePool
@brief Auto release Object from memory
*/

#include <map>
#include "platform.h"
#include "singleton.h"
#include "ref.h"
#include "primitivetype.h"

namespace bzmag
{ 

class Object;
class BZMAG_LIBRARY_API AutoReleasePool : public Singleton<AutoReleasePool>
{
public:
    AutoReleasePool();
    virtual ~AutoReleasePool();

    void add(Object* o);
    void remove(Object* o);
    Object* find(uint32 ID);
    void pump();

private:
    typedef std::map<uint32, Ref<Object>> Pool;

private:
    Pool pool;
};

}

#endif // BZMAG_CORE_UTILITY_AUTORELEASEPOOL_H
#include "unifiedserializer.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
bool UnifiedSerializer::serialize(const Uri& uri, Object* object)
{
    return true;
}


//-----------------------------------------------------------------------------
Object* UnifiedSerializer::deserialize
(Node* parent, const Uri& uri, const char* name)
{
    return 0;
}

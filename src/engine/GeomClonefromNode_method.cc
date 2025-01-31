#include "GeomClonefromNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomCloneFromNode_v_setCloneToNode_n(GeomCloneFromNode* self,
    Parameter* param)
{
    self->setReferenceNode(param->in()->get<GeomCloneToNode*>(0).get());
}


//----------------------------------------------------------------------------
void GeomCloneFromNode::bindMethod()
{
    BIND_METHOD(v_setCloneToNode_n, GeomCloneFromNode_v_setCloneToNode_n);
}

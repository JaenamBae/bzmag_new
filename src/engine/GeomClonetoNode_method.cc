#include "GeomClonetoNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomCloneToNode_z_getClonedNodes_v(GeomCloneToNode* self,
    Parameter* param)
{
    param->out()->clear();
    for (GeomCloneToNode::FromIterator it = self->firstClonedNode();
        it != self->lastClonedNode();
        ++it)
    {
        Node* n = (Node*)(*it);
        param->out()->add<Node*>(n);
    }
}

//----------------------------------------------------------------------------
void GeomCloneToNode::bindMethod()
{
    BIND_METHOD(z_getClonedNodes_v, GeomCloneToNode_z_getClonedNodes_v);
}

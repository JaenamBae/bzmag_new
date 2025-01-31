#include "CoilNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void CoilNode_v_setReferenceNode_n(CoilNode* self, Parameter* param)
{
    GeomHeadNode* head = param->in()->get<GeomHeadNode*>(0);
    self->setReferenceNode(head);
}

//----------------------------------------------------------------------------
void CoilNode::bindMethod()
{
    BIND_METHOD(v_setReferenceNode_n, CoilNode_v_setReferenceNode_n);
}

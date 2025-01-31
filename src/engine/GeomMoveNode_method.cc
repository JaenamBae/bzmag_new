#include "GeomMoveNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomMoveNode_v_setParameters_ss(GeomMoveNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1));
}

//----------------------------------------------------------------------------
void GeomMoveNode::bindMethod()
{
    BIND_METHOD(v_setParameters_ss, GeomMoveNode_v_setParameters_ss);
}

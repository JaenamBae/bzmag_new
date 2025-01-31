#include <cmath>

#include "GeomCircleNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomCircleNode_v_setParameters_sss(GeomCircleNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));
}

//----------------------------------------------------------------------------
void GeomCircleNode::bindMethod()
{
    BIND_METHOD(v_setParameters_sss, GeomCircleNode_v_setParameters_sss);
}

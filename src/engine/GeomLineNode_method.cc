#include <cmath>

#include "GeomLineNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomLineNode_v_setParameters_ss(GeomLineNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1));
}

//----------------------------------------------------------------------------
void GeomLineNode::bindMethod()
{
    BIND_METHOD(v_setParameters_ss, GeomLineNode_v_setParameters_ss);
}

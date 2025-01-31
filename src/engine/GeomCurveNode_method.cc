#include <cmath>

#include "GeomCurveNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomCurveNode_v_setParameters_ssss(GeomCurveNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<String>(3));
}

//----------------------------------------------------------------------------
void GeomCurveNode::bindMethod()
{
    BIND_METHOD(v_setParameters_sss, GeomCurveNode_v_setParameters_ssss);
}

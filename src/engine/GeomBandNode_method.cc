#include "GeomBandNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomBandNode_v_setParameters_ssss(GeomBandNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2),
        param->in()->get<String>(3));
}

//----------------------------------------------------------------------------
void GeomBandNode::bindMethod()
{
    BIND_METHOD(v_setParameters_ssss, GeomBandNode_v_setParameters_ssss);
}

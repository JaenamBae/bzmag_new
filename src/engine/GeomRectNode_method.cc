#include "GeomRectNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomRectNode_v_setParameters_sss(GeomRectNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1),
        param->in()->get<String>(2));
}

//----------------------------------------------------------------------------
void GeomRectNode::bindMethod()
{
    BIND_METHOD(v_setParameters_sss, GeomRectNode_v_setParameters_sss);
}

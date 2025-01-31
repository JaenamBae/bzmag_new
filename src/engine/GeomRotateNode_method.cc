#include "GeomRotateNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomRotateNode_v_setParameters_s(GeomRotateNode* self, Parameter* param)
{
    self->setParameters(param->in()->get<String>(0));
}

//----------------------------------------------------------------------------
void GeomRotateNode::bindMethod()
{
    BIND_METHOD(v_setParameters_s, GeomRotateNode_v_setParameters_s);
}

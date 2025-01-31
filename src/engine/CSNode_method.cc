#include "CSNode.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void CSNode_v_setParameters_ss(CSNode* self, Parameter* param)
{
    self->setParameters(
        param->in()->get<String>(0),
        param->in()->get<String>(1));
}

//----------------------------------------------------------------------------
static void CSNode_dddddd_getTransformation_v(CSNode* self, Parameter* param)
{
    Transformation& trans = self->transformation();

    param->out()->get<float64>(0) = CGAL::to_double(trans.m(0, 0));
    param->out()->get<float64>(1) = CGAL::to_double(trans.m(0, 1));
    param->out()->get<float64>(2) = CGAL::to_double(trans.m(0, 2));
    param->out()->get<float64>(3) = CGAL::to_double(trans.m(1, 0));
    param->out()->get<float64>(4) = CGAL::to_double(trans.m(1, 1));
    param->out()->get<float64>(5) = CGAL::to_double(trans.m(1, 2));
}

//----------------------------------------------------------------------------
void CSNode::bindMethod()
{
    BIND_METHOD(v_setParameters_ss, CSNode_v_setParameters_ss);
    BIND_METHOD(dddddd_getTransformation_v, CSNode_dddddd_getTransformation_v);
}

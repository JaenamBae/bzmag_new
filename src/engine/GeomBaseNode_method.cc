#include "GeomBaseNode.h"
#include "core/methodbinder.h"
#include "core/vector2.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomBaseNode_b_hitTest_dd(GeomBaseNode* self, Parameter* param)
{
    bool bHit = self->hitTest(
        param->in()->get<float64>(0),
        param->in()->get<float64>(1));

    param->out()->get<bool>(0) = bHit;
}

//----------------------------------------------------------------------------
static void GeomBaseNode_b_isCovered_v(GeomBaseNode* self, Parameter* param)
{
    bool bCovered = self->isCovered();
    param->out()->get<bool>(0) = bCovered;
}

//----------------------------------------------------------------------------
static void GeomBaseNode_n_getHeadNode_v(GeomBaseNode* self, Parameter* param)
{
    param->out()->get<GeomHeadNode*>(0) = self->getHeadNode();
}

//----------------------------------------------------------------------------
void GeomBaseNode::bindMethod()
{
    BIND_METHOD(b_hitTest_dd, GeomBaseNode_b_hitTest_dd);
    BIND_METHOD(b_isCovered_v, GeomBaseNode_b_isCovered_v);
    BIND_METHOD(n_getHeadNode_v, GeomBaseNode_n_getHeadNode_v);
}

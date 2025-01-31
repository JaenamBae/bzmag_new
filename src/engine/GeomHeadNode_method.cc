#include "GeomHeadNode.h"
#include "GeomToPath.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;


//----------------------------------------------------------------------------
static void GeomHeadNode_z_getPath_i(GeomHeadNode* self, Parameter* param)
{
    param->out()->clear();
    uint32 normal_deviation = param->in()->get<uint32>(0);

    // CCW direction
    GeomToPath geom_to_path(self);
    if (normal_deviation > 0) geom_to_path.setNormalDeviation(normal_deviation);

    GeomToPath::VertexList vertices;
    geom_to_path.makePath(vertices);

    GeomToPath::VertexList::const_iterator it;
    for (it = vertices.begin(); it != vertices.end(); ++it)
    {
        GeomToPath::VertexInfo pt = (*it);
        param->out()->add<float64>(pt.x);
        param->out()->add<float64>(pt.y);
        param->out()->add<uint32>(pt.cmd);
    }
}

//----------------------------------------------------------------------------
static void GeomHeadNode_i_getNumberOfEdge_v(GeomHeadNode* self, Parameter* param)
{
    uint64 nEdge = self->getCurves().size();
    param->out()->get<uint64>(0) = nEdge;
}

//----------------------------------------------------------------------------
static void GeomHeadNode_z_getEdgePath_i(GeomHeadNode* self, Parameter* param)
{
    param->out()->clear();

    uint32 edgeID = param->in()->get<uint32>(0);
    GeomToPath geom_to_path(self);
    GeomToPath::VertexList vertices;

    geom_to_path.makeEdgePath(edgeID, vertices);
    GeomToPath::VertexList::const_iterator it;
    for (it = vertices.begin(); it != vertices.end(); ++it)
    {
        GeomToPath::VertexInfo pt = (*it);
        param->out()->add<float64>(pt.x);
        param->out()->add<float64>(pt.y);
        param->out()->add<uint32>(pt.cmd);
    }
}

//----------------------------------------------------------------------------
static void GeomHeadNode_b_contain_n(GeomHeadNode* self, Parameter* param)
{
    GeomHeadNode* n1 = param->in()->get<GeomHeadNode*>(0).get();
    param->out()->get<bool>(0) = self->contain(n1);
}


//----------------------------------------------------------------------------
static void GeomHeadNode_b_hitTest_dd(GeomHeadNode* self, Parameter* param)
{
    bool bHit = self->hitTest(
        param->in()->get<float64>(0),
        param->in()->get<float64>(1));

    param->out()->get<bool>(0) = bHit;
}

//----------------------------------------------------------------------------
static void GeomHeadNode_b_isCovered_v(GeomHeadNode* self, Parameter* param)
{
    param->out()->get<bool>(0) = self->isCovered();
}

//----------------------------------------------------------------------------
static void GeomHeadNode_v_setMaterialNode_n(GeomHeadNode* self, Parameter* param)
{
    MaterialNode* material = param->in()->get<MaterialNode*>(0);
    self->setMaterialNode(material);
}

//----------------------------------------------------------------------------
static void GeomHeadNode_v_setCSNode_n(GeomHeadNode* self, Parameter* param)
{
    CSNode* cs = param->in()->get<CSNode*>(0);
    self->setReferedCS(cs);
}

//----------------------------------------------------------------------------
static void GeomHeadNode_v_setColor_iiii(GeomHeadNode* self, Parameter* param)
{
    Color color(param->in()->get<int>(0), 
        param->in()->get<int>(1),
        param->in()->get<int>(2),
        param->in()->get<int>(3));
    self->setColor(color);
}

//----------------------------------------------------------------------------
void GeomHeadNode::bindMethod()
{
    BIND_METHOD(b_hitTest_dd, GeomHeadNode_b_hitTest_dd);
    BIND_METHOD(b_isCovered_v, GeomHeadNode_b_isCovered_v);

    BIND_METHOD(z_getPath_i, GeomHeadNode_z_getPath_i);
    BIND_METHOD(i_getNumberOfEdge_v, GeomHeadNode_i_getNumberOfEdge_v);
    BIND_METHOD(z_getEdgePath_i, GeomHeadNode_z_getEdgePath_i);
    BIND_METHOD(b_contain_n, GeomHeadNode_b_contain_n);

    BIND_METHOD(v_setColor_iiii, GeomHeadNode_v_setColor_iiii);
    BIND_METHOD(v_setMaterialNode_n, GeomHeadNode_v_setMaterialNode_n);
    BIND_METHOD(v_setCSNode_n, GeomHeadNode_v_setCSNode_n);
}

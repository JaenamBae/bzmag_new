#include "BCNode.h"
#include "GeomHeadNode.h"
#include "GeomToPath.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void BCNode_z_getPath_v(BCNode* self, Parameter* param)
{
    param->out()->clear();

    GeomToPath geom_to_path(self);
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
void BCNode::bindMethod()
{
    BIND_METHOD(z_getPath_v, BCNode_z_getPath_v);
}

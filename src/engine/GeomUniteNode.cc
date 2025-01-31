#include "GeomUniteNode.h"
#include "GeomHeadNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(GeomUniteNode, GeomBooleanNode);

//----------------------------------------------------------------------------
GeomUniteNode::GeomUniteNode()
{

}

//----------------------------------------------------------------------------
GeomUniteNode::~GeomUniteNode()
{

}

//----------------------------------------------------------------------------
void GeomUniteNode::boolean_operation(Polygon_set_2& polyset)
{
    // 부모노드가 없으면 연산을 수행할 수 없음
    GeomBaseNode* parent = dynamic_cast<GeomBaseNode*>(getParent());
    if (!parent) {
        return;
    }

    ToolIter it;
    for (it = toolnodes_.begin(); it != toolnodes_.end(); ++it)
    {
        GeomHeadNode* tool = *it;
        const Polygon_set_2& geom_tool = tool->getPolyset();
        if (tool->isCovered()) {
            polyset.join(geom_tool);
        }
    }
    //polyset.remove_redundant_edges();
}

//----------------------------------------------------------------------------
void GeomUniteNode::bindProperty()
{

}
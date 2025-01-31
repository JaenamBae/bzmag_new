#include "GeomBooleanNode.h"
#include "GeomHeadNode.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_ABSTRACTCLASS(GeomBooleanNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomBooleanNode::GeomBooleanNode()
{

}

//----------------------------------------------------------------------------
GeomBooleanNode::~GeomBooleanNode()
{

}

//----------------------------------------------------------------------------
void GeomBooleanNode::updateToolNodes()
{
    // Set new tool nodes
    toolnodes_.clear();

    Node::ConstNodeIterator it;
    for (it = firstChildNode(); it != lastChildNode(); ++it)
    {
        Node* node = *it;
        GeomHeadNode* hn = dynamic_cast<GeomHeadNode*>(node);
        if (hn) {
            toolnodes_.push_back(hn);
        }
    }
}

//----------------------------------------------------------------------------
bool GeomBooleanNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    boolean_operation(polyset);
    return true;
}

/*
//----------------------------------------------------------------------------
void GeomBooleanNode::updateTransform()
{
    Transformation trans = getMyTransform();
    last_trans_ = trans * last_trans_;
}*/

//----------------------------------------------------------------------------
void GeomBooleanNode::updateLinkedNode()
{
    // 나의 ToolNodes를 linked_nodes에 넣는다
    updateToolNodes();
    GeomBooleanNode::ToolIter it;
    for (it = firstToolNode(); it != lastToolNode(); ++it) {
        GeomHeadNode* tool = *it;
        linked_heads_.push_back(LinkedHead(tool, Transformation()));
    }
}
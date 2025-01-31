#include "GeomCoverlineNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(GeomCoverLineNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomCoverLineNode::GeomCoverLineNode()
{
    be_covered_ = true;
}

//----------------------------------------------------------------------------
GeomCoverLineNode::~GeomCoverLineNode()
{

}

//----------------------------------------------------------------------------
bool GeomCoverLineNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    return true;
}

//----------------------------------------------------------------------------
void GeomCoverLineNode::updateCovered()
{
    be_covered_ = true;
}

//----------------------------------------------------------------------------
void GeomCoverLineNode::bindProperty()
{

}

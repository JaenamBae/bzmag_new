#include "GeomClonetoNode.h"
#include "GeomClonefromNode.h"
#include "GeomHeadNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomCloneToNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomCloneToNode::GeomCloneToNode()
{

}

//----------------------------------------------------------------------------
GeomCloneToNode::~GeomCloneToNode()
{
    //clones_.clear();
}

//----------------------------------------------------------------------------
GeomCloneToNode::FromIterator GeomCloneToNode::firstClonedNode()
{
    return clones_.begin();
}

//----------------------------------------------------------------------------
GeomCloneToNode::FromIterator GeomCloneToNode::lastClonedNode()
{
    return clones_.end();
}

//----------------------------------------------------------------------------
void GeomCloneToNode::clearBelongings()
{
    clones_.clear();
}

//----------------------------------------------------------------------------
bool GeomCloneToNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    return true;
}

//----------------------------------------------------------------------------
bool GeomCloneToNode::update()
{
    std::unique_lock<std::mutex> lock(mtx_);

    FromIterator it;
    for (it = clones_.begin(); it != clones_.end(); ++it) {
        GeomCloneFromNode* fromNode = *it;
        fromNode->update();
    }

    lock.unlock();
    return GeomBaseNode::update();
}

//----------------------------------------------------------------------------
void GeomCloneToNode::bindProperty()
{

}

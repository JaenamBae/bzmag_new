#include "GeomClonefromNode.h"
#include "GeomClonetoNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"


using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomCloneFromNode, GeomPrimitiveNode);

//----------------------------------------------------------------------------
GeomCloneFromNode::GeomCloneFromNode() : from_(nullptr)
{

}

//----------------------------------------------------------------------------
GeomCloneFromNode::~GeomCloneFromNode()
{
    if (from_.valid()) {
        from_->clones_.remove(this);
    }
}

//----------------------------------------------------------------------------
void GeomCloneFromNode::setReferenceNode(GeomCloneToNode* node)
{
    if (from_.valid()) from_->clones_.remove(this);

    from_ = nullptr;
    if (node) {
        node->clones_.push_back(this);
        node->clones_.unique();
        from_ = node;
    }

    update();
}

//----------------------------------------------------------------------------
GeomCloneToNode* GeomCloneFromNode::getReferenceNode() const
{
    return from_;
}

//----------------------------------------------------------------------------
void GeomCloneFromNode::clearBelongings()
{
    if (from_.valid()) {
        from_->clones_.remove(this);
        from_ = nullptr;
    }
}

//----------------------------------------------------------------------------
bool GeomCloneFromNode::make_geometry(Polygon_set_2& polyset, Curves& curves, Vertices& vertices, Transformation transform)
{
    if (from_.valid()) {
        from_->makeGeometry(transform);
        polyset = from_->getPolyset();
        curves = from_->getCurves();
        vertices = from_->getVertices();
    }

    // From이 없으면 Geometry를 생성할 수 없음
    return true;
}

//----------------------------------------------------------------------------
void GeomCloneFromNode::updateCovered()
{
    if (from_.valid()) be_covered_ = from_->isCovered();
    else be_covered_ = false;
}

//----------------------------------------------------------------------------
void GeomCloneFromNode::bindProperty()
{
    BIND_PROPERTY(GeomCloneToNode*, CloneFrom, &setReferenceNode, &getReferenceNode);
}
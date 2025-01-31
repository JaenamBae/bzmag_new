#include "GeomPrimitiveNode.h"
#include "GeomHeadNode.h"
#include "CSNode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_ABSTRACTCLASS(GeomPrimitiveNode, GeomBaseNode);

//----------------------------------------------------------------------------
GeomPrimitiveNode::GeomPrimitiveNode()
{

}

//----------------------------------------------------------------------------
GeomPrimitiveNode::~GeomPrimitiveNode()
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
void GeomPrimitiveNode::setReferedCS(CSNode* cs)
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
    }
    cs_ = cs;
    if (cs_.valid()) {
        cs_->insertReferenceNode(this);
    }

    update();
}

//----------------------------------------------------------------------------
CSNode* GeomPrimitiveNode::getReferedCS() const
{
    return cs_;
}

//----------------------------------------------------------------------------
Transformation GeomPrimitiveNode::getMyTransform()
{
    Transformation trans;
    if (cs_.valid()) {
        trans = cs_->transformation();
    }
    return trans;
}

void GeomPrimitiveNode::clearBelongings()
{
    if (cs_.valid()) {
        cs_->removeReferenceNode(this);
        cs_ = nullptr;
    }
}

//----------------------------------------------------------------------------
void GeomPrimitiveNode::updateTransform()
{
    // 변환 업데이트
    Transformation trans = getMyTransform();
    last_trans_ = trans;
}

//----------------------------------------------------------------------------
void GeomPrimitiveNode::updateLinkedNode()
{
    Transformation trans = getMyTransform();
    LinkedHeads::iterator it;
    for (it = linked_heads_.begin(); it != linked_heads_.end(); ++it)
    {
        (*it).second = trans;
    }
}

//----------------------------------------------------------------------------
void GeomPrimitiveNode::bindProperty()
{
    BIND_PROPERTY(CSNode*, CoordinateSystem,
        &setReferedCS,
        &getReferedCS);
}
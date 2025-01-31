#include "GeomHeadRefNode.h"
#include "GeomHeadNode.h"
#include "BCNode.h"
#include "core/simplepropertybinder.h"
#include "core/enumpropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(GeomHeadRefNode, Node);

//----------------------------------------------------------------------------
GeomHeadRefNode::GeomHeadRefNode()
{

}

//----------------------------------------------------------------------------
GeomHeadRefNode::~GeomHeadRefNode()
{

}

//----------------------------------------------------------------------------
GeomHeadNode* GeomHeadRefNode::getHeadNode() const
{
    return head_;
}

//----------------------------------------------------------------------------
void GeomHeadRefNode::setHeadNode(GeomHeadNode* head)
{
    head_ = head;
    update();
}

//----------------------------------------------------------------------------
bool GeomHeadRefNode::update()
{
    Node* parent = getParent();
    if (parent) {
        return parent->update();
    }
    return true;
}

//----------------------------------------------------------------------------
void GeomHeadRefNode::onAttachTo(Node* parent)
{
    BCNode* bc = dynamic_cast<BCNode*>(parent);
    if (bc) {
        bc->addBoundary(this);
    }
}

//----------------------------------------------------------------------------
void GeomHeadRefNode::onDetachFrom(Node* parent)
{
    BCNode* bc = dynamic_cast<BCNode*>(parent);
    if (bc) {
        bc->removeBoundary(this);
    }
}

void GeomHeadRefNode::clearBelongings()
{
    head_ = nullptr;
}

//----------------------------------------------------------------------------
void GeomHeadRefNode::bindProperty()
{
    BIND_PROPERTY(GeomHeadNode*, HeadNode, &setHeadNode, &getHeadNode);
}

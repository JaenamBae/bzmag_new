#include "MasterPeriodicBCnode.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(MasterPeriodicBCNode, BCNode);


//----------------------------------------------------------------------------
MasterPeriodicBCNode::MasterPeriodicBCNode()
{

}

//----------------------------------------------------------------------------
MasterPeriodicBCNode::~MasterPeriodicBCNode()
{

}

//----------------------------------------------------------------------------
void MasterPeriodicBCNode::setDirection(bool direction)
{
    dirction_ = direction;
}

//----------------------------------------------------------------------------
bool MasterPeriodicBCNode::getDirection() const
{
    return dirction_;
}

//----------------------------------------------------------------------------
bool MasterPeriodicBCNode::update()
{
    return BCNode::update();
}

//----------------------------------------------------------------------------
void MasterPeriodicBCNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void MasterPeriodicBCNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void MasterPeriodicBCNode::bindProperty()
{
    BIND_PROPERTY(bool, Direction, &setDirection, &getDirection);
}


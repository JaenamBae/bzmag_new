#include "DirichletBCnode.h"
#include "Expression.h"

#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"


using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(DirichletBCNode, BCNode);

//----------------------------------------------------------------------------
DirichletBCNode::DirichletBCNode() : value_(nullptr)
{
    uint32 key = getID();
    value_ = new Expression();
    value_->setKey("bc_" + std::to_string(key));
}

//----------------------------------------------------------------------------
DirichletBCNode::~DirichletBCNode()
{

}

//----------------------------------------------------------------------------
void DirichletBCNode::setBCValue(const String& value)
{
    if (!value_.valid()) return;
    const String& pvalue = value_->getExpression();
    if (!value_->setExpression(value)) {
        value_->setExpression(pvalue);
    }
}

//----------------------------------------------------------------------------
const String& DirichletBCNode::getBCValue() const
{
    return value_->getExpression();
}

float64 bzmag::engine::DirichletBCNode::evaluateBCValue() const
{
    return value_->eval();
}

//----------------------------------------------------------------------------
bool DirichletBCNode::update()
{
    return BCNode::update();
}

//----------------------------------------------------------------------------
void DirichletBCNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void DirichletBCNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void DirichletBCNode::clearBelongings()
{
    value_->setExpression("0");
    value_ = nullptr;
}

//----------------------------------------------------------------------------
void DirichletBCNode::bindProperty()
{
    BIND_PROPERTY(const String&, Value, &setBCValue, &getBCValue);
}


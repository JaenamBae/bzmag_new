#include "CoilNode.h"
#include "WindingNode.h"
#include "GeomHeadNode.h"
#include "Expression.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(CoilNode, Node);

//----------------------------------------------------------------------------
CoilNode::CoilNode() : ref_node_(nullptr), direction_(true), turns_(nullptr), sturns_("1")
{
    uint32 key = getID();

    turns_ = new Expression();
    turns_->setKey("turns_" + std::to_string(key));
    turns_->setExpression(sturns_);
}

//----------------------------------------------------------------------------
CoilNode::~CoilNode()
{

}

//----------------------------------------------------------------------------
double CoilNode::getCurrent()
{
    WindingNode* parent = dynamic_cast<WindingNode*>(getParent());
    if (parent) {
        double ampere_turns = parent->I_->eval() * turns_->eval() / parent->a_->eval();
        return ampere_turns;
    }
    return 0;
}

//----------------------------------------------------------------------------
void CoilNode::setReferenceNode(GeomHeadNode* node)
{
    ref_node_ = node;
}

//----------------------------------------------------------------------------
GeomHeadNode* CoilNode::getReferenceNode() const
{
    return ref_node_;
}

//-----------------------------------------------------------------------------
float64 CoilNode::evaluateNumberOfTurns() const
{
    return turns_->eval();
}

//-----------------------------------------------------------------------------
void CoilNode::setNumberOfTurns(const String& turns)
{
    if (turns_.invalid()) return;

    // 일단 Expression으로 변환 시도
    const String& pturns = turns_->getExpression();
    if (!turns_->setExpression(turns)) {
        // 실패하면 원상복귀
        turns_->setExpression(pturns);
        return;
    }

    // 맴버 새로운 값으로 업데이트
    sturns_ = turns;
}

//-----------------------------------------------------------------------------
const String& CoilNode::getNumberOfTurns() const
{
    return sturns_;
}

//----------------------------------------------------------------------------
//void CoilNode::clear()
//{
//    ref_node_ = nullptr;
//    direction_ = true;
//    sturns_ = "1";
//    turns_->setExpression(sturns_);
//}

//----------------------------------------------------------------------------
bool CoilNode::update()
{
    return true;
}

//----------------------------------------------------------------------------
void CoilNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void CoilNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void bzmag::engine::CoilNode::clearBelongings()
{
    turns_->setExpression("0");
    turns_ = nullptr;

    ref_node_ = nullptr;
}

//----------------------------------------------------------------------------
void CoilNode::bindProperty()
{
    BIND_PROPERTY(bool, Direction, &setDirection, &getDirection);
    BIND_PROPERTY(const String&, Turns, &setNumberOfTurns, &getNumberOfTurns);
    BIND_PROPERTY(GeomHeadNode*, ReferenceHead, &setReferenceNode, &getReferenceNode);
}


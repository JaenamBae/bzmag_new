#include "WindingNode.h"
#include "GeomHeadNode.h"
#include "Expression.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(WindingNode, Node);

//----------------------------------------------------------------------------
WindingNode::WindingNode() : a_(nullptr), I_(nullptr), sa_("1"), sI_("0")
{
    uint32 key = getID();

    a_ = new Expression();
    I_ = new Expression();
    a_->setKey("a_" + std::to_string(key));
    I_->setKey("I_" + std::to_string(key));

    a_->setExpression(sa_);
    I_->setExpression(sI_);
}

//----------------------------------------------------------------------------
WindingNode::~WindingNode()
{

}

//----------------------------------------------------------------------------
//void WindingNode::clear()
//{
//    sa_ = "1";
//    sI_ = "0";
//    a_->setExpression(sa_);
//    I_->setExpression(sI_);
//}

//----------------------------------------------------------------------------
void WindingNode::setCurrent(const String& I)
{
    if (I_.invalid()) return;

    // 일단 Expression으로 변환 시도
    const String& pI = I_->getExpression();
    if (!I_->setExpression(I)) {
        // 실패하면 원상복귀
        I_->setExpression(pI);
        return;
    }

    sI_ = I;
}

//----------------------------------------------------------------------------
const String& WindingNode::getCurrent() const
{
    return sI_;
}

//----------------------------------------------------------------------------
void WindingNode::setNumberOfParallelBranches(const String& a)
{
    if (a_.invalid()) return;

    // 일단 Expression으로 변환 시도
    const String& pa = a_->getExpression();
    if (!a_->setExpression(a)) {
        // 실패하면 원상복귀
        a_->setExpression(pa);
        return;
    }

    sa_ = a;
}

//----------------------------------------------------------------------------
const String& WindingNode::getNumberOfParallelBranches() const
{
    return sa_;
}

//----------------------------------------------------------------------------
float64 WindingNode::evaluateCurrent() const
{
    return I_->eval();
}

//----------------------------------------------------------------------------
float64 WindingNode::evaluateParallelBranches() const
{
    return a_->eval();
}

//----------------------------------------------------------------------------
bool WindingNode::update()
{
    return true;
}

//----------------------------------------------------------------------------
void WindingNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void WindingNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void WindingNode::clearBelongings()
{
    a_->setExpression("0");
    I_->setExpression("0");

    a_ = nullptr;
    I_ = nullptr;
}

//----------------------------------------------------------------------------
void WindingNode::bindProperty()
{
    BIND_PROPERTY(const String&, Current, &setCurrent, &getCurrent);
    BIND_PROPERTY(const String&, ParallelBranches, &setNumberOfParallelBranches, &getNumberOfParallelBranches);
}


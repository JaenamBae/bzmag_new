#include "PostNode.h"
#include "core/simplepropertybinder.h"
#include "engine/ExpressionServer.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(PostNode, Node);

//----------------------------------------------------------------------------
PostNode::PostNode() : obj_(nullptr)
{

}

PostNode::~PostNode()
{
    if (obj_) delete obj_;
}

void PostNode::setDrawingObject(DrawingObject* object)
{
    if (obj_) delete obj_;
    obj_ = object;
}

DrawingObject* PostNode::getDrawingObject()
{
    return obj_;
}

void PostNode::setDrawingFlag(bool flag)
{
    draw_flag_ = flag;
}

bool PostNode::getDrawingFlag() const
{
    return draw_flag_;
}

void PostNode::bindProperty()
{
    BIND_PROPERTY(bool, Show, &setDrawingFlag, &getDrawingFlag);
}

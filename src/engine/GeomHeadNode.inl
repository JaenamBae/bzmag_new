#include "GeomHeadNode.h"

//-----------------------------------------------------------------------------
inline void GeomHeadNode::setColor(const Color& color)
{
    color_ = color;
}

//-----------------------------------------------------------------------------
inline const Color& GeomHeadNode::getColor() const
{
    return color_;
}

//-----------------------------------------------------------------------------
inline void GeomHeadNode::setModelNode(bool model)
{
    model_node_ = model;
}

//-----------------------------------------------------------------------------
inline bool GeomHeadNode::isModelNode() const
{
    return model_node_;
}

//-----------------------------------------------------------------------------
inline void GeomHeadNode::setHideStatus(bool hide)
{
    hide_ = hide;
}

//-----------------------------------------------------------------------------
inline bool GeomHeadNode::isHide() const
{
    return hide_;
}

//-----------------------------------------------------------------------------
inline void GeomHeadNode::setStandAlone(bool standalone)
{
    standalone_ = standalone;
}

//-----------------------------------------------------------------------------
inline bool GeomHeadNode::isStandAlone() const
{
    return standalone_;
}

//-----------------------------------------------------------------------------
inline int32 GeomHeadNode::getNumberOfElements() const
{
    return num_elements_;
}

//-----------------------------------------------------------------------------
inline void GeomHeadNode::setNumberOfElements(int32 ne)
{
    num_elements_ = ne;
}

//-----------------------------------------------------------------------------
inline String GeomHeadNode::description() const
{
    return "Head";
}

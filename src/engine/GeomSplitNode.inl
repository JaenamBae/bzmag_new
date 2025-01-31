
//-----------------------------------------------------------------------------
inline void GeomSplitNode::setPlane(const SPLIT_PLANE& plane)
{
    plane_ = plane;
    update();
}

//-----------------------------------------------------------------------------
inline const GeomSplitNode::SPLIT_PLANE& GeomSplitNode::getPlane() const
{
    return plane_;
}

//-----------------------------------------------------------------------------
inline void GeomSplitNode::setOrientation(bool o)
{
    selectd_plane_ = o;
    update();
}

//-----------------------------------------------------------------------------
inline bool GeomSplitNode::getOrientation() const
{
    return selectd_plane_;
}

//-----------------------------------------------------------------------------
inline String GeomSplitNode::description() const
{
    return "SpiltBody";
}


//-----------------------------------------------------------------------------
inline void GeomCircleNode::setCenter(const String& center)
{

    setParameters(center, sradii_, ssegs_);
}

//-----------------------------------------------------------------------------
inline void GeomCircleNode::setRadius(const String& radii)
{
    setParameters(scenter_, radii, ssegs_);
}

//-----------------------------------------------------------------------------
inline void GeomCircleNode::setSegments(const String& segs)
{
    setParameters(scenter_, sradii_, segs);
}

//-----------------------------------------------------------------------------
inline const String& GeomCircleNode::getCenter() const
{
    return scenter_;
}

//-----------------------------------------------------------------------------
inline const String& GeomCircleNode::getRadius() const
{
    return sradii_;
}

//-----------------------------------------------------------------------------
inline const String& GeomCircleNode::getSegments() const
{
    return ssegs_;
}

//-----------------------------------------------------------------------------
inline String GeomCircleNode::description() const
{
    return "CreateCircle";
}

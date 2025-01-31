
//-----------------------------------------------------------------------------
inline void GeomBandNode::setCenter(const String& center)
{

    setParameters(center, sradii_, swidth_, ssegs_);
}

//-----------------------------------------------------------------------------
inline void GeomBandNode::setRadius(const String& radii)
{
    setParameters(scenter_, radii, swidth_, ssegs_);
}

//-----------------------------------------------------------------------------
inline void GeomBandNode::setWidth(const String& width)
{
    setParameters(scenter_, sradii_, width, ssegs_);
}

//-----------------------------------------------------------------------------
inline void GeomBandNode::setSegments(const String& segs)
{
    setParameters(scenter_, sradii_, swidth_, segs);
}

//-----------------------------------------------------------------------------
inline const String& GeomBandNode::getCenter() const
{
    return scenter_;
}

//-----------------------------------------------------------------------------
inline const String& GeomBandNode::getRadius() const
{
    return sradii_;
}

//-----------------------------------------------------------------------------
inline const String& GeomBandNode::getWidth() const
{
    return swidth_;
}

//-----------------------------------------------------------------------------
inline const String& GeomBandNode::getSegments() const
{
    return ssegs_;
}

//-----------------------------------------------------------------------------
inline String GeomBandNode::description() const
{
    return "CreateBand";
}

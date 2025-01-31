
//-----------------------------------------------------------------------------
inline void GeomRectNode::setPoint(const String& point)
{
    setParameters(point, swidth_, sheight_);
}

//-----------------------------------------------------------------------------
inline void GeomRectNode::setWidth(const String& width)
{
    setParameters(spoint_, width, sheight_);
}

//-----------------------------------------------------------------------------
inline void GeomRectNode::setHeight(const String& height)
{
    setParameters(spoint_, swidth_, height);
}

//-----------------------------------------------------------------------------
inline const String& GeomRectNode::getPoint() const
{
    return spoint_;
}

//-----------------------------------------------------------------------------
inline const String& GeomRectNode::getWidth() const
{
    return swidth_;
}

//-----------------------------------------------------------------------------
inline const String& GeomRectNode::getHeight() const
{
    return sheight_;
}

//-----------------------------------------------------------------------------
inline String GeomRectNode::description() const
{
    return "CreateRectangle";
}

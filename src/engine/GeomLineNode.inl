
//-----------------------------------------------------------------------------
inline void GeomLineNode::setStartPoint(const String& start)
{
    setParameters(start, send_);
}

//-----------------------------------------------------------------------------
inline void GeomLineNode::setEndPoint(const String& end)
{
    setParameters(sstart_, end);
}

//-----------------------------------------------------------------------------
inline const String& GeomLineNode::getStartPoint() const
{
    return sstart_;
}

//-----------------------------------------------------------------------------
inline const String& GeomLineNode::getEndPoint() const
{
    return send_;
}

//-----------------------------------------------------------------------------
inline String GeomLineNode::description() const
{
    return "CreateLine";
}

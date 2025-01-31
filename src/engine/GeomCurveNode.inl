
//-----------------------------------------------------------------------------
inline void GeomCurveNode::setStartPoint(const String& start)
{
    setParameters(start, send_, scenter_, sradius_);
}

//-----------------------------------------------------------------------------
inline void GeomCurveNode::setEndPoint(const String& end)
{
    setParameters(sstart_, end, scenter_, sradius_);
}

//-----------------------------------------------------------------------------
inline void GeomCurveNode::setCenterPoint(const String& center)
{
    setParameters(sstart_, send_, center, sradius_);
}

//-----------------------------------------------------------------------------
inline void GeomCurveNode::setRadius(const String& radius)
{
    setParameters(sstart_, send_, scenter_, radius);
}

//-----------------------------------------------------------------------------
inline const String& GeomCurveNode::getStartPoint() const
{
    return sstart_;
}

//-----------------------------------------------------------------------------
inline const String& GeomCurveNode::getEndPoint() const
{
    return send_;
}

//-----------------------------------------------------------------------------
inline const String& GeomCurveNode::getCenterPoint() const
{
    return scenter_;
}

//-----------------------------------------------------------------------------
inline const String& GeomCurveNode::getRadius() const
{
    return sradius_;
}


//-----------------------------------------------------------------------------
inline String GeomCurveNode::description() const
{
    return "CreateCurve";
}

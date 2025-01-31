
//-----------------------------------------------------------------------------
inline void GeomRotateNode::setAngle(const String& angle)
{
    setParameters(angle);
}

//-----------------------------------------------------------------------------
inline const String& GeomRotateNode::getAngle() const
{
    return sangle_;
}

//-----------------------------------------------------------------------------
inline String GeomRotateNode::description() const
{
    return "Rotate";
}


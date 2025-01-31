
//-----------------------------------------------------------------------------
inline void CSNode::setOrigin(const String& origin)
{
    setParameters(origin, sangle_);
}

//-----------------------------------------------------------------------------
inline void CSNode::setAngle(const String& angle)
{
    setParameters(sorigin_, angle);
}

//-----------------------------------------------------------------------------
inline const String& CSNode::getOrigin() const
{
    return sorigin_;
}

//-----------------------------------------------------------------------------
inline const String& CSNode::getAngle() const
{
    return sangle_;
}

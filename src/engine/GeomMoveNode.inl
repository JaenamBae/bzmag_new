
//-----------------------------------------------------------------------------
inline void GeomMoveNode::set_dx(const String& dx)
{
    setParameters(dx, sdy_);
}

//-----------------------------------------------------------------------------
inline void GeomMoveNode::set_dy(const String& dy)
{
    setParameters(sdx_, dy);
}

//-----------------------------------------------------------------------------
inline const String& GeomMoveNode::get_dx() const
{
    return sdx_;
}

//-----------------------------------------------------------------------------
inline const String& GeomMoveNode::get_dy() const
{
    return sdy_;
}

//-----------------------------------------------------------------------------
inline String GeomMoveNode::description() const
{
    return "Move";
}


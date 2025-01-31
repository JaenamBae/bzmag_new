//-----------------------------------------------------------------------------
inline const String& Uri::get() const
{
    return uri_;
}


//-----------------------------------------------------------------------------
inline const char* Uri::c_str() const
{
    return uri_.c_str();
}


//-----------------------------------------------------------------------------
inline const String& Uri::getProtocol() const
{
    return protocol_;
}

//-----------------------------------------------------------------------------
inline const String& Uri::getPackage() const
{
    return managed_;
}

//-----------------------------------------------------------------------------
inline const String& Uri::getIp() const
{
    return ip_;
}

//-----------------------------------------------------------------------------
inline const String& Uri::getPort() const
{
    return port_;
}

//-----------------------------------------------------------------------------
inline const String& Uri::getPath() const
{
    return path_;
}

//-----------------------------------------------------------------------------
inline bool Uri::operator < (const Uri& rhs) const
{
    return uri_ < rhs.uri_;
}

//-----------------------------------------------------------------------------
inline bool Uri::operator == (const Uri& rhs) const
{
    return (uri_ == rhs.uri_);
}

//-----------------------------------------------------------------------------
inline bool Uri::operator != (const Uri& rhs) const
{
    return !((*this) == rhs);
}

//-----------------------------------------------------------------------------
inline Uri::operator const String& () const
{
    return uri_;
}

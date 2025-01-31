//-----------------------------------------------------------------------------
inline bool String::empty() const
{
    return data_.empty();
}


//-----------------------------------------------------------------------------
inline String::size_type String::size() const
{
    return data_.size();
}


//-----------------------------------------------------------------------------
inline String::size_type String::length() const
{
    return data_.length();
}

inline void String::clear()
{
    data_.clear();
}

inline void String::resize(size_type size)
{
    data_.resize(size);
}

inline void String::resize(size_type size, value_type c)
{
    data_.resize(size, c);
}

inline void String::reserve(size_type capacity)
{
    data_.reserve(capacity);
}

inline String& String::assign(const String& s)
{
    data_.assign(s.data_);
    return (*this);
}
inline String& String::assign(const String& s, size_type offset, size_type count)
{
    data_.assign(s.data_, offset, count);
    return (*this);
}

inline String& String::assign(const value_type* c, size_type num)
{
    data_.assign(c, num);
    return (*this);
}

inline String& String::assign(const value_type* c)
{
    data_.assign(c);
    return (*this);
}

inline String& String::assign(size_type count, value_type c)
{
    data_.assign(count, c);
    return (*this);
}

inline String& String::assign(const_pointer first, const_pointer last)
{
    data_.assign(first, last);
    return (*this);
}

inline String& String::assign(const_iterator first, const_iterator last)
{
    data_.assign(first, last);
    return (*this);
}

inline String::size_type String::find(const String& s, size_type offset) const
{
    return data_.find(s.data_, offset);
}

inline String::size_type String::find(const value_type* c, size_type offset, size_type count) const
{
    return data_.find(c, offset, count);
}

inline String::size_type String::find(const value_type* c, size_type offset) const
{
    return data_.find(c, offset);
}

inline String::size_type String::find(value_type c, size_type offset) const
{
    return data_.find(c, offset);
}

inline String::size_type String::rfind(const String& s, size_type offset) const
{
    return data_.rfind(s.data_, offset);
}

inline String::size_type String::rfind(const value_type* c, size_type offset, size_type count) const
{
    return data_.rfind(c, offset, count);
}

inline String::size_type String::rfind(const value_type* c, size_type offset) const
{
    return data_.rfind(c, offset);
}

inline String::size_type String::rfind(value_type c, size_type offset) const
{
    return data_.rfind(c, offset);
}

inline String String::substr(size_type offset, size_type count) const
{
    return String(data_, offset, count);
}

inline int String::compare(const String& s) const
{
    return data_.compare(s.data_);
}

inline int String::compare(size_type offset, size_type count, const String& s) const
{
    return data_.compare(offset, count, s.data_);
}

inline int String::compare(size_type loffset, size_type count, const String& s, size_type roffset, size_type ncount) const
{
    return data_.compare(loffset, count, s.data_, roffset, ncount);
}

inline int String::compare(const value_type* c) const
{
    return data_.compare(c);
}

inline int String::compare(size_type offset, size_type count, const value_type* c) const
{
    return data_.compare(offset, count, c);
}

inline int String::compare(size_type offset, size_type count, const value_type* c, size_type ncount) const
{
    return data_.compare(offset, count, c, ncount);
}

inline bool String::contains(const String& str) const
{
    return data_.find(str.c_str()) != std::string::npos;
}

inline String String::trim() const
{
    auto start = data_.begin();
    while (start != data_.end() && std::isspace(*start)) start++;

    auto end = data_.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return (start <= end ? std::string(start, end + 1) : std::string());
}

//-----------------------------------------------------------------------------
inline const String::value_type* String::c_str() const
{
    return data_.c_str();
}


//-----------------------------------------------------------------------------
inline const String::value_type* String::data() const
{
    return data_.data();
}


//-----------------------------------------------------------------------------
inline String::reference String::at(size_type offset)
{
    return data_.at(offset);
}


//-----------------------------------------------------------------------------
inline String::const_reference String::at(size_type offset) const
{
    return data_.at(offset);
}


//-----------------------------------------------------------------------------
inline String::iterator String::begin()
{
    return data_.begin();
}


//-----------------------------------------------------------------------------
inline String::iterator String::end()
{
    return data_.end();
}


//-----------------------------------------------------------------------------
inline String::const_iterator String::begin() const
{
    return data_.begin();
}


//-----------------------------------------------------------------------------
inline String::const_iterator String::end() const
{
    return data_.end();
}


//-----------------------------------------------------------------------------
inline String::reverse_iterator String::rbegin()
{
    return data_.rbegin();
}


//-----------------------------------------------------------------------------
inline String::reverse_iterator String::rend()
{
    return data_.rend();
}


//-----------------------------------------------------------------------------
inline String::const_reverse_iterator String::rbegin() const
{
    return data_.rbegin();
}


//-----------------------------------------------------------------------------
inline String::const_reverse_iterator String::rend() const
{
    return data_.rend();
}


//-----------------------------------------------------------------------------
inline bool String::operator < (const String& s) const
{
    return data_ < s.data_;
}


//-----------------------------------------------------------------------------
inline bool String::operator > (const String& s) const
{
    return data_ > s.data_;
}


//-----------------------------------------------------------------------------
inline bool operator == (const String& s1, const String& s2)
{
    return s1.data_ == s2.data_;
}


//-----------------------------------------------------------------------------
inline bool operator != (const String& s1, const String& s2)
{
    return s1.data_ != s2.data_;
}


//-----------------------------------------------------------------------------
inline bool operator == (const String& s1, const char* s2)
{
    return s1.data_ == s2;
}


//-----------------------------------------------------------------------------
inline bool operator != (const String& s1, const char* s2)
{
    return s1.data_ != s2;
}


//-----------------------------------------------------------------------------
inline bool operator == (const char* s1, const String& s2)
{
    return s2.data_ == s1;
}


//-----------------------------------------------------------------------------
inline bool operator != (const char* s1, const String& s2)
{
    return s2.data_ != s1;
}


//-----------------------------------------------------------------------------
inline bzmag::String operator + (const String& s1, const String& s2)
{
    String r(s1);
    r += s2;
    return r;
}


//-----------------------------------------------------------------------------
inline bzmag::String operator + (const String& s1, const char* s2)
{
    String r(s1);
    r += s2;
    return r;
}


//-----------------------------------------------------------------------------
inline bzmag::String operator + (const char* s1, const String& s2)
{
    String r(s1);
    r += s2;
    return r;
}

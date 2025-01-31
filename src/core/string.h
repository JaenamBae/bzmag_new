#ifndef BZMAG_CORE_TYPE_STRING_H
#define BZMAG_CORE_TYPE_STRING_H
/**
    @ingroup bzmagCoreType
    @class bzmag::String
    @brief
*/

#pragma warning(disable: 4251)

#include <string>
#include "platform.h"
#include "define.h"
#include "primitivetype.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API String
    {
    public:
        typedef std::string stdstring;

        typedef stdstring::size_type size_type;
        typedef stdstring::reference reference;
        typedef stdstring::const_reference const_reference;
        typedef stdstring::iterator iterator;
        typedef stdstring::const_iterator const_iterator;
        typedef stdstring::reverse_iterator reverse_iterator;
        typedef stdstring::const_reverse_iterator const_reverse_iterator;
        typedef stdstring::value_type value_type;
        typedef stdstring::const_pointer pointer;
        typedef stdstring::const_pointer const_pointer;
        static const size_type npos = -1;

    public:
        String() {}
        String(const std::string& s):data_(s) {}
        String(const char* s, ...);
        String(const wchar_t* s, ...);
        String(const String& s, size_type offset, size_type count):
            data_(s.data_, offset, count) {}

        virtual ~String() {}

        bool empty() const;
        size_type size() const;
        size_type length() const;

        void clear();
        void resize(size_type size);
        void resize(size_type size, value_type c);
        void reserve(size_type capacity = 0);

        String& assign(const String& s);
        String& assign(const String& s, size_type offset, size_type count);
        String& assign(const value_type* c, size_type num);
        String& assign(const value_type* c);
        String& assign(size_type count, value_type c);
        String& assign(const_pointer first, const_pointer last);
        String& assign(const_iterator first, const_iterator last);

        size_type find(const String& s, size_type offset = 0) const;
        size_type find(const value_type* c, size_type offset, size_type count) const;
        size_type find(const value_type* c, size_type offset = 0) const;
        size_type find(value_type c, size_type offset = 0) const;

        size_type rfind(const String& s, size_type offset = npos) const;
        size_type rfind(const value_type* c, size_type offset, size_type count) const;
        size_type rfind(const value_type* c, size_type offset = npos) const;
        size_type rfind(value_type c, size_type offset = npos) const;

        String substr(size_type offset = 0, size_type count = npos) const;

        int compare(const String& s) const;
        int compare(size_type offset, size_type count, const String& s) const;
        int compare(size_type loffset, size_type count, const String& s, size_type roffset, size_type ncount) const;
        int compare(const value_type* c) const;
        int compare(size_type offset, size_type count, const value_type* c) const;
        int compare(size_type offset, size_type count, const value_type* c, size_type ncount) const;

        bool contains(const String& str) const;

        String trim() const;

        const value_type* c_str() const;
        const value_type* data() const;

        reference at(size_type offset);
        const_reference at(size_type offset) const;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        reverse_iterator rbegin();
        reverse_iterator rend();
        const_reverse_iterator rbegin() const;
        const_reverse_iterator rend() const;
        
        void format(const char* s, ...);
        void format(const char* s, va_list args);

        void format(const wchar_t* s, ...);
        void format(const wchar_t* s, va_list args);

        bool operator < (const String& s) const;
        bool operator > (const String& s) const;

        friend bool operator == (const String& s1, const String& s2);
        friend bool operator != (const String& s1, const String& s2);
        friend bool operator == (const String& s1, const char* s2);
        friend bool operator != (const String& s1, const char* s2);
        friend bool operator == (const char* s1, const String& s2);
        friend bool operator != (const char* s1, const String& s2);

        friend String operator + (const String& s1, const String& s2);
        friend String operator + (const String& s1, const char* s2);
        friend String operator + (const char* s1, const String& s2);

        const String& operator += (const String& s)
        {
            data_ += s.data_;
            return (*this);
        }
        
        operator char* ();
        operator const char* () const;
        operator wchar_t* ();
        operator const wchar_t* () const;
        
        operator short();
        operator const short() const;
        operator unsigned short();
        operator const unsigned short() const;

        operator int();
        operator const int() const;
        operator unsigned int();
        operator const unsigned int() const;
        operator int32();
        operator const int32() const;
        operator uint32();
        operator const uint32() const;
        operator int64 ();
        operator const int64 () const;
        operator uint64 ();
        operator const uint64 () const;
        operator float32 ();
        operator const float32 () const;
        operator float64 ();
        operator const float64 () const;
        
        operator std::string() const { return std::string(this->c_str()); }

        /**
            Utilities
        */
        //@{
        int toInt() const;
        int32 toInt32() const;
        int64 toInt64() const;
        float toFloat() const;
        double toDouble() const;
        String extractPath() const;
        void replace(const String& src_str, const String& dst_str);
        //@}

    public:
        static void encoding(
            char* src, size_t src_len,
            char* dest, size_t dest_len);

    private:
        stdstring data_;
    };

#include "string.inl"

}

#endif // BZMAG_CORE_TYPE_STRING_H

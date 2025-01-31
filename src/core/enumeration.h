#ifndef BZMAG_CORE_UTILITY_ENUMERATION_H
#define BZMAG_CORE_UTILITY_ENUMERATION_H
/**
    @ingroup bzmagCoreUtility
    @class bzmag::Enumeration
    @brief Enumeration for enum
*/

#include <map>
#include <unordered_map>
#include "define.h"
#include "primitivetype.h"
#include "string.h"

namespace bzmag
{
    template <typename T, int SIZE, int INVALID=SIZE>
    class Enumeration
    {
        typedef Enumeration<T, SIZE, INVALID> my_type;

    public:
        typedef std::map<String, T> StringMap;
        typedef std::unordered_map<T, String> ValueMap;
        typedef typename ValueMap::iterator enumerator;
        typedef typename ValueMap::const_iterator const_enumerator;

    public:
        Enumeration():value_(static_cast<T>(INVALID))
        {
        }        
        Enumeration(const char* str)
        {
            fromString(str);
        }
        Enumeration(const String& str)
        {
            fromString(str);
        }
        Enumeration(const T& value):value_(value)
        {
            // empty
        }
        Enumeration(const my_type& value):value_(value.value_)
        {
            // empty
        }
        Enumeration(const int& value)
        {
            value_ = static_cast<T>(value);
        }

        static void add(const String& name, const int& value)
        {
            stringMap_.insert(StringMap::value_type(name, static_cast<T>(value)));
            valueMap_.insert(ValueMap::value_type(static_cast<T>(value), name));
        }

        void fromString(const char* str)
        {
            value_ = from_string(str);
        }
        void fromString(const String& str)
        {
            value_ = from_string(str.c_str());
        }
        const String& toString() const
        {
            ValueMap::iterator find_iter = valueMap_.find(value_);
            if (valueMap_.end() == find_iter)
            {
                static String empty;
                return empty;
            }
            return find_iter->second;
        }

        enumerator beginEnum()
        {
            return valueMap_.begin();
        }
        enumerator endEnum()
        {
            return valueMap_.end();
        }
        const_enumerator beginEnum() const
        {
            return valueMap_.begin();
        }
        const_enumerator endEnum() const
        {
            return valueMap_.end();
        }

        bool operator == (const Enumeration& lhs) const
        {
            return value_ == lhs.value_;
        }
        bool operator != (const Enumeration& lhs) const
        {
            return !(*this == lhs);
        }
        bool operator == (const T& lhs) const
        {
            return value_ == lhs;
        }
        bool operator != (const T& lhs) const
        {
            return !(*this == lhs);
        }
        bool operator == (const char* lhs) const
        {
            return value_ == from_string(lhs);
        }
        bool operator != (const char* lhs) const
        {
            return !(*this == lhs);
        }
        operator const T& () const
        {
            return value_;
        }

        template <typename T>
        operator T () const
        {
            return static_cast<T>(value_);
        }

    private:
        T from_string(const char* str) const
        {
            StringMap::iterator find_iter = stringMap_.find(str);
            if (stringMap_.end() == find_iter)
                return static_cast<T>(INVALID);
            return find_iter->second;
        }

    private:
        T value_;

        static StringMap stringMap_;
        static ValueMap valueMap_;
    };

    template <typename T, int SIZE, int INVALID>
    typename Enumeration<T, SIZE, INVALID>::StringMap
    Enumeration<T, SIZE, INVALID>::stringMap_;

    template <typename T, int SIZE, int INVALID>
    typename Enumeration<T, SIZE, INVALID>::ValueMap
    Enumeration<T, SIZE, INVALID>::valueMap_;
}

#endif // BZMAG_CORE_UTILITY_ENUMERATION_H

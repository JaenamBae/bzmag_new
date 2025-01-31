#ifndef BZMAG_CORE_TYPE_TUPLE3_H
#define BZMAG_CORE_TYPE_TUPLE3_H
/**
    @ingroup bzmagCoreType
    @class bzmag::Tuple3
    @brief
*/

namespace bzmag
{
    template <typename T>
    class Tuple3
    {
    public:
        typedef Tuple3<T> type;

    public:
        Tuple3();
        Tuple3(T x, T y, T z);

        void clear();
        void set(T x, T y, T z);
        void set(int i, T value)
        {
            a_[i] = value;
        }

        const T& operator [] (int i) const
        {
            return a_[i];
        }
        T& operator [] (int i)
        {
            return a_[i];
        }
        
    public:
        union
        {
            struct
            {
                T x_, y_, z_;
            };
            T a_[3];
        };
        
    };

#include "tuple3.inl"

}

#endif // BZMAG_CORE_TYPE_TUPLE3_H

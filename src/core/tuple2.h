#ifndef BZMAG_CORE_TYPE_TUPLE2_H
#define BZMAG_CORE_TYPE_TUPLE2_H
/**
@ingroup bzmagCoreType
@class bzmag::Tuple2
@brief
*/


namespace bzmag
{
    template <typename T>
    class Tuple2
    {
    public:
        typedef Tuple2<T> type;

    public:
        Tuple2();
        Tuple2(T x, T y);

        void clear();
        void set(T x, T y);
        void set(int i, T value);

        const T& operator [] (int i) const;
        T& operator [] (int i);
        T x() const;
        T y() const;

    public:
        union
        {
            struct
            {
                T x_, y_;
            };
            T a_[2];
        };
    };

#include "tuple2.inl"

}

#endif // BZMAG_CORE_TYPE_TUPLE2_H

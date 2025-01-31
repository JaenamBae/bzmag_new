#ifndef BZMAG_CORE_UTILITY_REF_H
#define BZMAG_CORE_UTILITY_REF_H
/**
    @ingroup bzmagCoreUtility
    @class bzmag::Ref
    @brief 
*/

#include "define.h"

namespace bzmag
{
    class Object;
    template <typename T>
    class Ref
    {
    public:
        typedef Ref<T> type;

    public:
        Ref();
        Ref(const type& ref);
        Ref(Object* instance);
        ~Ref();

        void initialize(Object* instance);
        void release();

        bool valid() const;
        bool invalid() const;

        template <typename DERIVED>
        DERIVED get();
        template <typename DERIVED>
        DERIVED get() const;
        template <typename DERIVED>
        operator DERIVED ();
        template <typename DERIVED>
        operator DERIVED () const;

        operator T* ();
        T* operator -> ();
        T* operator -> () const;

        T* operator * ();
        T* operator * () const;

        type& operator = (Object* instance);

        bool operator == (Object* instance) const;
        bool operator != (Object* instance) const;

        bool operator == (const type& rhs) const;
        bool operator != (const type& rhs) const;
        bool operator < (const type& rhs) const;
        bool operator > (const type& rhs) const;

    private:
        union
        {
            T* instance_;   // real type
            Object* node_;    // abstract type
        };
    };

#include "ref.inl"

}

#endif // BZMAG_CORE_UTILITY_REF_H

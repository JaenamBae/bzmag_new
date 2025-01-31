#ifndef BZMAG_CORE_OBJECT_SIMPLEVARIABLE_H
#define BZMAG_CORE_OBJECT_SIMPLEVARIABLE_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::SimpleVariable
    @brief
*/

#include "define.h"
#include "variable.h"
#include "typeid.h"

namespace bzmag
{
    template <typename T>
    class SimpleVariable : public Variable
    {
    public:
        typedef SimpleVariable<T> type;

    public:
        //virtual ~SimpleVariable() {}
        virtual Variable* clone();
        virtual type_id getType() const;

        T& get();
        const T& get() const;

        type& operator = (const T& v);
        operator T ();

    public:
        T data_;
    };

#include "simplevariable.inl"

}

#endif // BZMAG_CORE_OBJECT_SIMPLEVARIABLE_H

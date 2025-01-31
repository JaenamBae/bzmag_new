#ifndef BZMAG_CORE_OBJECT_METHODBINDER_H
#define BZMAG_CORE_OBJECT_METHODBINDER_H
/**
    @ingroup bzmagCoreObject
    @class bzmag::MethodBinder
    @brief 
*/

#include "method.h"

namespace bzmag
{
    template <typename T>
    class MethodBinder : public Method
    {
    public:
        typedef void (*BaseBindFunc)(Object*, Parameter*);
        typedef void (*BindFunc)(T*, Parameter*);

    public:
        virtual void invoke(Object* self);

        void setBindFunc(BindFunc func);
        BindFunc getBindFunc();

    private:
        union
        {
            BaseBindFunc baseBindFunc_;
            BindFunc bindFunc_;
        };
    };

#include "methodbinder.inl"

#define BIND_METHOD(name, bind_func) \
    if (!TYPE.hasBindedMethod()) {\
        static MethodBinder<type> s_##name##_method;\
        s_##name##_method.setPrototype(#name);\
        s_##name##_method.setBindFunc(\
            static_cast<MethodBinder<type>::BindFunc>(bind_func));\
        TYPE.addMethod(&s_##name##_method); }
}

#endif // BZMAG_CORE_OBJECT_METHODBINDER_H

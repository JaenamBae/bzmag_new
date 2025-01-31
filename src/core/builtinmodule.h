#ifndef BZMAG_CORE_KERNEL_BUILTINMODULE_H
#define BZMAG_CORE_KERNEL_BUILTINMODULE_H
/**
    @ingroup bzmagCoreKernel
    @class bzmag::BuiltinModule
    @brief 
*/

#include "platform.h"
#include "module.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API BuiltinModule : public Module
    {
    public:
        BuiltinModule(Kernel* kernel, const String& name);
        virtual~BuiltinModule();
    };
}

#endif // BZMAG_CORE_KERNEL_BUILTINMODULE_H

#ifndef BZMAG_CORE_KERNEL_MODULE_H
#define BZMAG_CORE_KERNEL_MODULE_H
/**
@ingroup bzmagCoreKernel
@class bzmag::Module
@brief
*/

#include <map>
#include "platform.h"
#include "primitivetype.h"
#include "string.h"

namespace bzmag
{
    class Type;
    class Object;
    class Kernel;
    class BZMAG_LIBRARY_API Module
    {
    public:
        typedef std::map<String, const Type*> Types;
        typedef void(*InitializeModuleFunc)(Module*);
        typedef void(*FinalizeModuleFunc)(Module*);

    public:
        Module(
            const String& name,
            InitializeModuleFunc init_func,
            FinalizeModuleFunc fin_func);
        virtual~Module();

        virtual void initialize();
        virtual void finalize();

        Object* create(const String& type_name);

        void addType(Kernel* kernel, const String& type_name, Type* type);
        const Type* findType(const String& type_name) const;

        const String& getName() const;
        size_t getNumTypes() const;

        Types::iterator firstType();
        Types::iterator lastType();
        Types::const_iterator firstType() const;
        Types::const_iterator lastType() const;

    private:
        /// Module's name
        String name_;
        /// Types
        Types types_;
        /// Initialize Module Function Pointer
        InitializeModuleFunc initializeModuleFunc_;
        /// Finalize Module Function Pointer
        FinalizeModuleFunc finalizeModuleFunc_;
    };

#include "module.inl"

    /// Register Type macro
#define REGISTER_TYPE(m, cls) m->addType(\
    bzmag::Kernel::instance(), #cls, &cls::TYPE);
    /// Declare Module macro
#define DECLARE_MODULE(API_MACRO, name) \
    extern "C" API_MACRO void Use##name##Module() \
    { \
        Module* s_##name##_module = new Module( \
            #name, initialize_##name, finalize_##name); \
        bzmag::Kernel::instance()->addModule(s_##name##_module); \
    }

    /// Include Module macro
#define INCLUDE_MODULE(API_MACRO, name) extern "C" API_MACRO void Use##name##Module();
    /// Using Module macro
#define USING_MODULE(name) Use##name##Module();

}

#endif // BZMAG_CORE_KERNEL_MODULE_H

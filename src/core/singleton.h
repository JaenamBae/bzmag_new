#ifndef BZMAG_CORE_UTILITY_SINGLETON_H
#define BZMAG_CORE_UTILITY_SINGLETON_H
/**
    @ingroup bzmagCoreUtility
    @class bzmag::Singleton
    @brief 
*/

#include "singletonserver.h"
#include "singletonbase.h"

namespace bzmag
{
    template <typename T>
    class Singleton : public SingletonBase
    {
    public:
        Singleton();
        virtual ~Singleton();

        static T* instance();
        static bool isInitialized();

    private:
        static T* s_instance;
    };

#include "singleton.inl"

}

#endif // BZMAG_CORE_UTILITY_SINGLETON_H

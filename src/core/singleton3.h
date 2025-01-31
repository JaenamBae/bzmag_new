#ifndef BZMAG_CORE_UTILITY_SINGLETON3_H
#define BZMAG_CORE_UTILITY_SINGLETON3_H
/**
    @ingroup TodCoreUtility
    @class tod::Singleton3
    @brief 
*/

#include "path.h"
#include "kernel.h"
#include "singletonbase.h"

namespace bzmag
{
    template <typename T>
    class Singleton3 : public SingletonBase
    {
    public:
        Singleton3();
        virtual ~Singleton3();

        virtual int addRefSingleton();
        virtual int releaseSingleton();
        virtual int getRefSingleton() const;

        static void setSingletonPath(const Path& path, bool init=true);

        static T* instance();
        static bool isInitialized();

    private:
        static Path s_path;
        static T* s_instance;
    };

#include "singleton3.inl"

}

#endif // BZMAG_CORE_UTILITY_SINGLETON3_H

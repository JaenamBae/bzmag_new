#ifndef BZMAG_CORE_UTILITY_CWNGUARD_H
#define BZMAG_CORE_UTILITY_CWNGUARD_H
/**
    @ingroup bzmagCoreUtility
    @class bzmag::CwnGuard
    @brief Current Working Node Guard
*/

#include "platform.h"
#include "exception.h"
#include "ref.h"
#include "kernel.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API CwnGuard
    {
    public:
        CwnGuard(Node* node)
        {
            Kernel::instance()->pushCwn(node);
        }
        CwnGuard(const Path& path)
        {
            Node* node = Kernel::instance()->lookup(path);
            if (0 == node)
                BZMAG_THROW_EXCEPTION(0, String(
                    "\'%s\' not found node in NOH", path.c_str()));
            Kernel::instance()->pushCwn(node);
        }
        ~CwnGuard()
        {
            Kernel::instance()->popCwn();
        }

    private:
    };

#include "cwnguard.inl"

}

#endif // BZMAG_CORE_UTILITY_CWNGUARD_H

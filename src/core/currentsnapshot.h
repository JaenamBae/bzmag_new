#ifndef BZMAG_CORE_UTILITY_CURRENTSNAPSHOT_H
#define BZMAG_CORE_UTILITY_CURRENTSNAPSHOT_H
/**
    @ingroup bzmagCoreException
    @class bzmag::CurrentSnapShot
    @brief 
*/

#include "platform.h"
#include "primitivetype.h"
#include "string.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API CurrentSnapShot
    {
    public:
        CurrentSnapShot();
        CurrentSnapShot(const char* type, const char* date,
            const char* time, const char* file_name,
            const char* func_sig, const char* func_name, int line,
            const char* expression);

        void setSemantic(const char* type, const char* date,
            const char* time, const char* file_name,
            const char* func_sig, const char* func_name, int line,
            const char* expression);

    public:
        String type_;
        String date_;
        String time_;
        String fileName_;
        String functionSignature_;
        String functionName_;
        int line_;
        String expression_;
    };

#include "currentsnapshot.inl"

}

#endif // BZMAG_CORE_UTILITY_CURRENTSNAPSHOT_H

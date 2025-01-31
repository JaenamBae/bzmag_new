#ifndef BZMAG_ENGINE_PLATFORM_H
#define BZMAG_ENGINE_PLATFORM_H

#ifdef _WIN32
    #if defined(BZMAG_SHARED_LIBS) && defined(BZMAGENGINE_EXPORTS)
        #define ENGINELIBRARY_API __declspec(dllexport)
    #elif defined(BZMAG_SHARED_LIBS)
        #define ENGINELIBRARY_API __declspec(dllimport)
    #else
        #define ENGINELIBRARY_API  // 정적 라이브러리일 경우 빈 정의
    #endif
#else
    #define ENGINELIBRARY_API  // 다른 플랫폼에서는 필요 없음
#endif


#endif // BZMAG_ENGINE_PLATFORM_H

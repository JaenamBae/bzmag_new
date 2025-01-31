#include <iostream>

// 전처리기 매크로 설정 (Windows에서 DLL 내보내기)
#ifdef _WIN32
    #define HELLO_API __declspec(dllexport)
#else
    #define HELLO_API
#endif

// hello() 함수를 DLL로 내보내기
extern "C" HELLO_API void hello() {
    std::cout << "Hello, World from DLL!" << std::endl;
}
#ifndef _H_BZMAGEDITOR_PLATFORM_DEPENDENT_H_
#define _H_BZMAGEDITOR_PLATFORM_DEPENDENT_H_

#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h>
#else
    #error "Unsupported platform"
#endif

std::string getExecutablePath();
std::string getExecPath();


#endif //_H_BZMAGEDITOR_PLATFORM_DEPENDENT_H_
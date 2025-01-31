#include "PlatformDepandent.h"

std::string getExecutablePath() {
    #if defined(_WIN32) || defined(_WIN64)
    // Windows: Use GetModuleFileName
    char path[MAX_PATH];
    if (GetModuleFileNameA(nullptr, path, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to get executable path on Windows");
    }
    return std::string(path);

    #elif defined(__linux__)
    // Linux: Use /proc/self/exe
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        throw std::runtime_error("Failed to get executable path on Linux");
    }
    return std::string(path, count);

    #elif defined(__APPLE__)
    // macOS: Use _NSGetExecutablePath
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        throw std::runtime_error("Failed to get executable path on macOS");
    }
    return std::string(path);

    #else
    throw std::runtime_error("Unsupported platform");
    #endif
}

std::string getExecPath()
{
    std::string exec_path = getExecutablePath();
    std::filesystem::path path(exec_path);
    return path.parent_path().string();
}

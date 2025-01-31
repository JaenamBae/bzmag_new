# 최상위 CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
#set(CMAKE_PREFIX_PATH "C:/msys64/mingw64" ${CMAKE_PREFIX_PATH})
#set(CMAKE_PREFIX_PATH "/mingw64/lib/cmake")

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_TOOLCHAIN_FILE "")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

project(bzmag)

# 각 프로젝트를 서브 디렉토리로 추가
add_subdirectory(src/core)              # bzmag_core 프로젝트 (코어 라이브러리)
add_subdirectory(src/bzmagPy)           # bzmag_bzmagPy 프로젝트 (확장 라이브러리)
add_subdirectory(src/engine)            # bzmag_engine 프로젝트 (확장 라이브러리)
add_subdirectory(src/bzmagEditor)       # bzmagEditor 프로젝트 (어플리케이션)

# bzmagPy가 bzmagCore와 bzmagEngine을 빌드 후 참조하도록 설정
add_dependencies(bzmagPy bzmagCore)

# bzmagEngine가 bzmagCore을 빌드 후 참조하도록 설정
add_dependencies(bzmagEngine bzmagCore)

# bzmagEditor가 bzmagCore와 bzmagEngine 그리고 bzmagPy을 빌드 후 참조하도록 설정
add_dependencies(bzmagEditor bzmagCore bzmagEngine bzmagPy)
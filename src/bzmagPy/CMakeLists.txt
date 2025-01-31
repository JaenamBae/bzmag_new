# bzmagCore CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# 프로젝트 이름 설정
project(bzmagPy)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 라이브러리 빌드 타입 옵션 추가 (기본값: OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(HEADERS
  python_extension.h
  pythonscriptserver.h
  platform.h
)

set(SOURCES
  python_common.cc
  python_func.cc
  python_node.cc
  python_object.cc
  pythonscriptserver.cc
  main.cc
)

# 라이브러리 정의
add_library(bzmagPy ${SOURCES} ${HEADERS})

# Python 찾기
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# bzmagCore 및 bzmagEngine을 찾을 수 있도록 설정
target_include_directories(bzmagPy PRIVATE
    ${Python3_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(bzmagPy PRIVATE bzmagCore) # bzmagCore, bzmagEngine를 참조
target_link_libraries(bzmagPy PRIVATE ${Python3_LIBRARIES})

# Windows에서 DLL을 빌드할 때 전처리기 매크로 설정
if(BUILD_SHARED_LIBS)
    target_compile_definitions(bzmagPy PRIVATE BZMAGPY_EXPORTS)
endif()

# 라이브러리의 출력 디렉터리 설정
set_target_properties(bzmagPy PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/Debug"
    
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/Release"
)
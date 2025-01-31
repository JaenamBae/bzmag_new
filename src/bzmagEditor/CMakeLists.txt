# 최상위 CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
#set(CMAKE_EXE_LINKER_FLAGS "-Wl,-subsystem,console")

project(bzmagEditor)

# Windows 환경에서 winsock2.h를 먼저 포함하도록 설정
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# Qt 필요한 모듈을 찾아 설정
set(Qt5_DIR "/mingw64/lib/cmake/Qt5")
find_package(Qt5 COMPONENTS Widgets Core Gui Concurrent Charts REQUIRED)
set(CMAKE_AUTOMOC ON)  # 자동으로 moc 처리
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# spdlog 추가
find_package(spdlog REQUIRED)

# Boost 라이브러리 찾기
find_package(Boost REQUIRED COMPONENTS filesystem system)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Boost not found! Install Boost via MSYS2.")
endif()

# CGAL 라이브러리 찾기
find_package(CGAL REQUIRED)
if(CGAL_FOUND)
    message(STATUS "CGAL found: ${CGAL_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "CGAL not found! Install CGAL via MSYS2.")
endif()

# nlohmann-json 추가
find_package(nlohmann_json REQUIRED)

# Python 찾기
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

# CMAKE_BUILD_TYPE에 따른 경로 설정
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(QtPropertyBrowserLibPath "${CMAKE_SOURCE_DIR}/lib/debug")
else()
    set(QtPropertyBrowserLibPath "${CMAKE_SOURCE_DIR}/lib/release")
endif()

# qtpropertybrowser 라이브러리 찾기
find_library(QtPropertyBrowserLib NAMES libqtpropertybrowser.a PATHS "${QtPropertyBrowserLibPath}")

# Qt 리소스 파일 처리
set(QT_RESOURCES bzMagEditor.qrc)
qt5_add_resources(QT_RESOURCES_GENERATED ${QT_RESOURCES})

message(STATUS "Generated Qt resources: ${QT_RESOURCES_GENERATED}")

# Windows 리소스 파일 추가
set(WINDOWS_RESOURCES bzMagEditor.rc)

# MinGW에서 .rc 파일을 올바르게 처리하도록 설정
if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
endif()

# main.cc를 포함한 모든 소스 파일을 추가
file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

add_executable(bzmagEditor WIN32
    main.cpp
    ${SOURCE_FILES}
    ${QT_RESOURCES_GENERATED}  # Qt 리소스 파일 포함
    ${WINDOWS_RESOURCES}  # Windows 리소스 파일 포함
)

# include 경로 추가 (Boost, CGAL, nlohmann-json 추가)
target_include_directories(bzmagEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/qtpropertybrowser
    ${CMAKE_SOURCE_DIR}/external
    ${Boost_INCLUDE_DIRS}  # Boost 헤더 추가
    ${CGAL_INCLUDE_DIRS}  # CGAL 헤더 추가
    ${Python3_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}  # nlohmann-json 헤더 추가
)

# bzmagEngine, bzmagCore, qtpropertybrowser, Boost, CGAL, Python, nlohmann_json, spdlog를 참조
target_link_libraries(bzmagEditor PRIVATE 
    bzmagCore bzmagPy bzmagEngine Boost::filesystem Boost::system nlohmann_json::nlohmann_json spdlog::spdlog ${QtPropertyBrowserLib} ${CGAL_LIBRARIES} ${Python3_LIBRARIES} 
) 

# Qt 모듈을 bzmagEditor에 연결
target_link_libraries(bzmagEditor PRIVATE
    Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Concurrent Qt5::Charts Qt5::Core
)

target_link_libraries(bzmagEditor PRIVATE ws2_32)

# 라이브러리의 출력 디렉터리 설정
set_target_properties(bzmagEditor PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/Debug"
    
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/Release"
)

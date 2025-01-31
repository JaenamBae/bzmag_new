# 최상위 CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# 프로젝트 이름 설정
project(QtPropertyBrowser)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")

# 라이브러리 빌드 타입 옵션 추가 (기본값: ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Qt 필요한 모듈을 찾아 설정
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
set(CMAKE_AUTOMOC ON)  # 자동으로 moc 처리
find_package(spdlog REQUIRED)

# main.cc를 포함한 모든 소스 파일을 추가
file(GLOB_RECURSE SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# 라이브러리 정의
add_library(QtPropertyBrowser ${SOURCE_FILES})

# Windows에서 DLL을 빌드할 때 전처리기 매크로 설정
if(BUILD_SHARED_LIBS)
    target_compile_definitions(QtPropertyBrowser PRIVATE PROPERTYBROWSER_EXPORTS)
endif()

# Qt 모듈과 spdlog를 QtPropertyBrowser에 연결
target_link_libraries(QtPropertyBrowser PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui spdlog::spdlog)

# 라이브러리의 출력 디렉토리 설정
set_target_properties(QtPropertyBrowser PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib"   # 정적 라이브러리 (.lib 또는 .a)
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib"   # 동적 라이브러리 (.dll 또는 .so)
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin"   # 실행 파일이 있는 경우 (.dll의 경우 Windows에서 필요)
)
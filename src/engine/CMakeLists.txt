# bzmagEngine CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# 프로젝트 이름 설정
project(bzmagEngine)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")

# 라이브러리 빌드 타입 옵션 추가 (기본값: ON)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(HEADERS
  BCNode.h
  CoilNode.h
  DataSetNode.h
  DirichletBCNode.h
  EngineNodeStringConverter.h
  Expression.h
  ExpressionServer.h
  CSNode.h
  GeomBandNode.h
  GeomBooleanNode.h
  GeomClonefromNode.h
  GeomClonetoNode.h
  GeomCoverlineNode.h
  GeomCurveNode.h
  GeometricEntity.h
  GeomBaseNode.h
  GeomCircleNode.h
  GeomHeadNode.h
  GeomHeadRefNode.h
  GeomIntersectionNode.h
  GeomLineNode.h
  GeomMoveNode.h
  GeomPrimitiveNode.h
  GeomRectNode.h
  GeomRotateNode.h
  GeomSplitNode.h
  GeomSubtractNode.h
  GeomToSurfaceMesh.h
  GeomToTriangle.h
  GeomUniteNode.h
  GeomToPath.h
  GMshDataStructure.h
  MaterialNode.h
  MasterPeriodicBCNode.h
  MovingBandNode.h
  SlavePeriodicBCNode.h
  SolutionSetup.h
  platform.h
  Transient.h
  WindingNode.h
  print_utils.h
  engine.h
)

set(INLINE_HEADERS
  BCNode.inl
  CoilNode.inl
  Expression.inl
  GeomBandNode.inl
  GeomCircleNode.inl
  CSNode.inl
  GeomClonefromNode.inl
  GeomClonetoNode.inl
  GeomCoverlineNode.inl
  GeomCurveNode.inl
  GeomHeadNode.inl
  GeomIntersectionNode.inl
  GeomMoveNode.inl
  GeomLineNode.inl
  GeomRectNode.inl
  GeomRotateNode.inl
  GeomSplitNode.inl
  GeomSubtractNode.inl
  GeomToTriangle.inl
  GeomUniteNode.inl
  MaterialNode.inl
  WindingNode.inl
)

set(SOURCES
  BCNode.cc
  BCNode_method.cc
  CoilNode.cc
  CoilNode_method.cc
  DataSetNode.cc
  DataSetNode_method.cc
  DirichletBCNode.cc
  Expression.cc
  ExpressionServer.cc
  Expression_method.cc
  GeomBandNode.cc
  GeomBandNode_method.cc
  GeomBaseNode.cc
  GeomBaseNode_method.cc
  GeomBooleanNode.cc
  GeomCircleNode.cc
  GeomCircleNode_method.cc
  CSNode.cc
  CSNode_method.cc
  GeomClonefromNode.cc
  GeomClonefromNode_method.cc
  GeomClonetoNode.cc
  GeomClonetoNode_method.cc
  GeomCoverlineNode.cc
  GeomCoverlineNode_method.cc
  GeomCurveNode.cc
  GeomCurveNode_method.cc
  GeomHeadNode.cc
  GeomHeadNode_method.cc
  GeomHeadRefNode.cc
  GeomIntersectionNode.cc
  GeomIntersectionNode_method.cc
  GeomLineNode.cc
  GeomLineNode_method.cc
  GeomMoveNode.cc
  GeomMoveNode_method.cc
  GeomPrimitiveNode.cc
  GeomRectNode.cc
  GeomRectNode_method.cc
  GeomRotateNode.cc
  GeomRotateNode_method.cc
  GeomSplitNode.cc
  GeomSplitNode_method.cc
  GeomSubtractNode.cc
  GeomSubtractNode_method.cc
  GeomToSurfaceMesh.cc
  GeomToTriangle.cc
  GeomToTriangle_method.cc
  GeomUniteNode.cc
  GeomUniteNode_method.cc
  GeomToPath.cc
  MaterialNode.cc
  MaterialNode_method.cc
  MasterPeriodicBCNode.cc
  MovingBandNode.cc
  SlavePeriodicBCNode.cc
  SolutionSetup.cc
  SolutionSetup_method.cc
  Transient.cc
  Transient_method.cc
  WindingNode.cc
  WindingNode_method.cc
  main.cc
)

# 라이브러리 정의
add_library(bzmagEngine ${SOURCES} ${HEADERS})
target_link_libraries(bzmagEngine PRIVATE bzmagCore) # bzmagCore를 참조
#target_compile_options(bzmagEngine PRIVATE /bigobj)

# include 경로 추가
#target_include_directories(bzmagEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(bzmagEngine PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 필수 패키지 찾기
#set(Boost_DIR "/mingw64/lib/cmake/Boost")
#set(CMAKE_PREFIX_PATH "/mingw64/lib/cmake")

find_package(Boost REQUIRED)
find_package(CGAL REQUIRED)
find_package(muparser REQUIRED)
find_package(spdlog REQUIRED)

# Windows에서 DLL을 빌드할 때 전처리기 매크로 설정
if(BUILD_SHARED_LIBS)
    target_compile_definitions(bzmagEngine PRIVATE BZMAGENGINE_EXPORTS)
endif()

target_link_libraries(bzmagEngine PRIVATE CGAL::CGAL muparser spdlog::spdlog)

# 라이브러리의 출력 디렉터리 설정
set_target_properties(bzmagEngine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/Debug"
    
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/Release"
)
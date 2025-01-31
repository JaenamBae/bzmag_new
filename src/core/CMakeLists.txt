# bzmagCore CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

# 프로젝트 이름 설정
project(bzmagCore)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 라이브러리 빌드 타입 옵션 추가 (기본값: OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(HEADERS
  autoreleasepool.h
  builtinmodule.h
  color.h
  compositeproperty.h
  compositepropertybinder.h
  containerproperty.h
  currentsnapshot.h
  cwnguard.h
  dataset.h
  define.h
  enumeration.h
  enumproperty.h
  enumpropertybinder.h
  exception.h
  kernel.h
  method.h
  methodbinder.h
  methods.h
  module.h
  node.h
  nodeeventpublisher.h
  nodeeventsubscriber.h
  object.h
  parameter.h
  path.h
  platform.h
  primitivetype.h
  primitive_stringconverter.h
  properties.h
  property.h
  ref.h
  scriptserver.h
  serializer.h
  serializerevent.h
  simpleproperty.h
  simplepropertybinder.h
  simplevariable.h
  singleton.h
  singleton3.h
  singletonbase.h
  singletonserver.h
  string.h
  stringconverter.h
  structproperty.h
  testcaller.h
  testcallerbase.h
  testcase.h
  testsuite.h
  tokenizer.h
  tostring.h
  tuple2.h
  tuple3.h
  type.h
  typeid.h
  unifiedserializer.h
  unittest.h
  uri.h
  variable.h
  variables.h
  vector2.h
  vector3.h
)

set(INLINE_HEADERS
  compositeproperty.inl
  compositepropertybinder.inl
  containerproperty.inl
  currentsnapshot.inl
  cwnguard.inl
  enumproperty.inl
  enumpropertybinder.inl
  kernel.inl
  method.inl
  methodbinder.inl
  module.inl
  node.inl
  parameter.inl
  path.inl
  property.inl
  ref.inl
  simpleproperty.inl
  simplepropertybinder.inl
  simplevariable.inl
  singleton.inl
  singleton3.inl
  singletonbase.inl
  string.inl
  structproperty.inl
  testcaller.inl
  testcase.inl
  testsuite.inl
  tuple2.inl
  tuple3.inl
  type.inl
  uri.inl
  variables.inl
)

set(SOURCES
  autoreleasepool.cc
  builtinmodule.cc
  color.cc
  cwnguard.cc
  exception.cc
  kernel.cc
  method.cc
  module.cc
  node.cc
  nodeeventpublisher.cc
  node_method.cc
  object.cc
  object_method.cc
  path.cc
  property.cc
  scriptserver.cc
  scriptserver_method.cc
  singletonserver.cc
  string.cc
  structproperty.cc
  testsuite.cc
  tokenizer.cc
  tostring.cc
  type.cc
  unifiedserializer.cc
  uri.cc
  variables.cc
)

# 라이브러리 정의
add_library(bzmagCore ${SOURCES} ${HEADERS})

# iconv 라이브러리를 필수로 설정하고 링크

find_package(Iconv REQUIRED)
target_link_libraries(bzmagCore PRIVATE Iconv::Iconv)

# Windows에서 DLL을 빌드할 때 전처리기 매크로 설정
if(BUILD_SHARED_LIBS)
    target_compile_definitions(bzmagCore PRIVATE BZMAGCORE_EXPORTS)
endif()

# 라이브러리의 출력 디렉터리 설정
set_target_properties(bzmagCore PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../lib/Debug"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/../bin/Debug"
    
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../lib/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/../bin/Release"
)
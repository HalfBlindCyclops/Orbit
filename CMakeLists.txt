cmake_minimum_required(VERSION 3.24)

project(Orbit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(OrbitConfig)
include(Dependencies)

option(ORBIT_BUILD_TESTS "Build unit tests" ON)
option(ORBIT_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(ORBIT_BUILD_PROTO "Build protobuf telemetry library" ON)
option(ORBIT_ENABLE_ASAN "Enable AddressSanitizer and UBSan" OFF)

if(ORBIT_BUILD_PROTO)
  find_package(Protobuf CONFIG QUIET)
  if(NOT Protobuf_FOUND)
    message(WARNING "Protobuf not found; disabling ORBIT_BUILD_PROTO")
    set(ORBIT_BUILD_PROTO OFF CACHE BOOL "" FORCE)
  endif()
endif()

add_subdirectory(src)

if(ORBIT_BUILD_TESTS)
  enable_testing()
  include(CTest)
  add_subdirectory(tests)
endif()

if(ORBIT_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

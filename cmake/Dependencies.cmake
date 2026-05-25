include(FetchContent)

function(orbit_find_or_fetch_gtest)
  find_package(GTest CONFIG QUIET)
  if(GTest_FOUND)
    return()
  endif()

  message(STATUS "GTest not found via vcpkg; fetching with FetchContent")
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
  )
  FetchContent_MakeAvailable(googletest)
endfunction()

function(orbit_find_or_fetch_eigen)
  find_package(Eigen3 CONFIG QUIET)
  if(Eigen3_FOUND)
    return()
  endif()

  message(STATUS "Eigen3 not found via vcpkg; fetching with FetchContent")
  FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.bz2
  )
  FetchContent_MakeAvailable(eigen)
  if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3_Eigen INTERFACE)
    target_include_directories(Eigen3_Eigen INTERFACE ${eigen_SOURCE_DIR})
    add_library(Eigen3::Eigen ALIAS Eigen3_Eigen)
  endif()
endfunction()

find_package(benchmark CONFIG QUIET)
if(NOT benchmark_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googlebenchmark)
endif()

add_executable(orbit_bench bench_tick.cpp)
target_link_libraries(orbit_bench PRIVATE orbit_sim_lib benchmark::benchmark)
target_compile_features(orbit_bench PRIVATE cxx_std_20)
orbit_apply_warnings(orbit_bench)

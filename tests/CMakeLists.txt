orbit_find_or_fetch_gtest()

add_executable(orbit_tests
  smoke_tests.cpp
  physics_tests.cpp
  flight_computer_tests.cpp
  telemetry_tests.cpp
  sensor_tests.cpp
  replay_tests.cpp
)

target_link_libraries(orbit_tests
  PRIVATE
    orbit_sim_lib
    GTest::gtest_main
)

target_compile_definitions(orbit_tests
  PRIVATE
    ORBIT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

if(ORBIT_BUILD_PROTO)
  target_compile_definitions(orbit_tests PRIVATE ORBIT_HAS_PROTO=1)
endif()

orbit_apply_warnings(orbit_tests)
orbit_apply_sanitizers(orbit_tests)

include(GoogleTest)
gtest_discover_tests(orbit_tests DISCOVERY_MODE PRE_TEST)

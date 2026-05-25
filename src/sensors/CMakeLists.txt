add_library(orbit_sensors
  sensor_model.cpp
  tracker.cpp
  sensor_system.cpp
)

target_include_directories(orbit_sensors PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/engine
)
target_link_libraries(orbit_sensors PUBLIC orbit_core)
target_compile_features(orbit_sensors PUBLIC cxx_std_20)

orbit_apply_warnings(orbit_sensors)
orbit_apply_sanitizers(orbit_sensors)

add_library(orbit_core
  clock.cpp
  sha256.cpp
)

target_include_directories(orbit_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(orbit_core PUBLIC cxx_std_20)
orbit_apply_warnings(orbit_core)
orbit_apply_sanitizers(orbit_core)

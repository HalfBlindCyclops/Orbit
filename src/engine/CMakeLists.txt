add_library(orbit_engine
  entity.cpp
  physics.cpp
  perturbations.cpp
  gravity_model.cpp
  gravity_worker_pool.cpp
  flight_computer.cpp
  simulation.cpp
  scenario.cpp
  state_hash.cpp
  snapshot_builder.cpp
  replay.cpp
)

target_include_directories(orbit_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orbit_engine PUBLIC orbit_core)

target_link_libraries(orbit_engine PUBLIC orbit_sensors)

if(ORBIT_BUILD_PROTO)
  target_link_libraries(orbit_engine PUBLIC orbit_network)
  target_compile_definitions(orbit_engine PUBLIC ORBIT_HAS_PROTO=1)
endif()
target_compile_features(orbit_engine PUBLIC cxx_std_20)

orbit_apply_warnings(orbit_engine)
orbit_apply_physics_fp(orbit_engine)
orbit_apply_sanitizers(orbit_engine)

orbit_find_or_fetch_eigen()
target_link_libraries(orbit_engine PRIVATE Eigen3::Eigen)

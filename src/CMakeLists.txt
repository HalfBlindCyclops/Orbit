add_subdirectory(core)

if(ORBIT_BUILD_PROTO)
  add_subdirectory(${CMAKE_SOURCE_DIR}/proto ${CMAKE_BINARY_DIR}/proto)
endif()

add_subdirectory(network)
add_subdirectory(sensors)
add_subdirectory(engine)

add_library(orbit_sim_lib INTERFACE)
target_link_libraries(orbit_sim_lib
  INTERFACE
    orbit_engine
    orbit_sensors
    orbit_network
)

add_executable(orbit_sim main.cpp)
target_link_libraries(orbit_sim PRIVATE orbit_sim_lib)
orbit_apply_warnings(orbit_sim)
orbit_apply_sanitizers(orbit_sim)

set(ORBIT_NETWORK_SOURCES telemetry_hub.cpp telemetry_encoder.cpp)

if(ORBIT_BUILD_PROTO)
  find_package(asio CONFIG REQUIRED)
else()
  set(ORBIT_NETWORK_SOURCES telemetry_hub_stub.cpp)
endif()

add_library(orbit_network ${ORBIT_NETWORK_SOURCES})

target_include_directories(orbit_network PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(orbit_network PUBLIC orbit_core)
target_compile_features(orbit_network PUBLIC cxx_std_20)

orbit_apply_warnings(orbit_network)
orbit_apply_sanitizers(orbit_network)

if(ORBIT_BUILD_PROTO)
  target_compile_definitions(orbit_network PUBLIC ASIO_STANDALONE ORBIT_HAS_PROTO=1)
  target_link_libraries(orbit_network PUBLIC orbit_proto asio::asio)
else()
  message(STATUS "orbit_network: telemetry stub (ORBIT_BUILD_PROTO=OFF)")
endif()

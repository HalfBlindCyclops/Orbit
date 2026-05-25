find_package(Protobuf CONFIG REQUIRED)

set(ORBIT_PROTO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/telemetry.proto")

protobuf_generate_cpp(ORBIT_PROTO_SRCS ORBIT_PROTO_HDRS ${ORBIT_PROTO_FILE})

add_library(orbit_proto STATIC ${ORBIT_PROTO_SRCS} ${ORBIT_PROTO_HDRS})
target_include_directories(orbit_proto
  PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
target_link_libraries(orbit_proto PUBLIC protobuf::libprotobuf)
target_compile_features(orbit_proto PUBLIC cxx_std_20)

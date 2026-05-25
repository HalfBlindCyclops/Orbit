# Phase 2+: generates orbit_proto from proto/telemetry.proto when ORBIT_BUILD_PROTO=ON.

function(orbit_add_proto_library target_name proto_file)
  find_package(Protobuf REQUIRED)
  set(proto_src "${CMAKE_SOURCE_DIR}/proto/${proto_file}")
  get_filename_component(proto_name "${proto_file}" NAME_WE)
  set(gen_dir "${CMAKE_BINARY_DIR}/generated")
  set(gen_src "${gen_dir}/${proto_name}.pb.cc")
  set(gen_hdr "${gen_dir}/${proto_name}.pb.h")

  add_custom_command(
    OUTPUT "${gen_src}" "${gen_hdr}"
    COMMAND protobuf::protoc
      --cpp_out=${gen_dir}
      --proto_path=${CMAKE_SOURCE_DIR}/proto
      ${proto_src}
    DEPENDS "${proto_src}"
    COMMENT "Generating ${proto_name} protobuf sources"
  )

  add_library(${target_name} STATIC "${gen_src}")
  target_include_directories(${target_name} PUBLIC "${gen_dir}")
  target_link_libraries(${target_name} PUBLIC protobuf::libprotobuf)
endfunction()

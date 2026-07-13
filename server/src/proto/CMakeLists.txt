set(PROTO_ROOT    "${CMAKE_SOURCE_DIR}/../proto")
set(PROTO_FILES   "${PROTO_ROOT}/robot/v1/robot.proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")

add_library(robot_proto STATIC)

protobuf_generate(
  TARGET   robot_proto
  LANGUAGE cpp
  PROTOS   ${PROTO_FILES}
  IMPORT_DIRS    ${PROTO_ROOT}
  PROTOC_OUT_DIR ${PROTO_GEN_DIR}
)

protobuf_generate(
  TARGET   robot_proto
  LANGUAGE grpc
  PROTOS   ${PROTO_FILES}
  IMPORT_DIRS    ${PROTO_ROOT}
  PROTOC_OUT_DIR ${PROTO_GEN_DIR}
  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
  PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
)

target_include_directories(robot_proto PUBLIC ${PROTO_GEN_DIR})
target_link_libraries(robot_proto PUBLIC
  protobuf::libprotobuf
  gRPC::grpc++
)

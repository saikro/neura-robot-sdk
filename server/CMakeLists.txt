cmake_minimum_required(VERSION 3.20)
project(robot_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC     CONFIG REQUIRED)

# --- Proto generation ------------------------------------------------------
# The .proto lives at the repo root, owned by neither server nor client.
# Sources are regenerated at build time; nothing generated is committed.

set(PROTO_ROOT    "${CMAKE_SOURCE_DIR}/../proto")
set(PROTO_FILES   "${PROTO_ROOT}/robot/v1/robot.proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")

set(GENERATED_PROTO_SRCS
  "${PROTO_GEN_DIR}/robot/v1/robot.pb.cc"
  "${PROTO_GEN_DIR}/robot/v1/robot.pb.h"
  "${PROTO_GEN_DIR}/robot/v1/robot.grpc.pb.cc"
  "${PROTO_GEN_DIR}/robot/v1/robot.grpc.pb.h"
)

add_custom_command(
  OUTPUT  ${GENERATED_PROTO_SRCS}
  COMMAND $<TARGET_FILE:protobuf::protoc>
    --proto_path=${PROTO_ROOT}
    --cpp_out=${PROTO_GEN_DIR}
    --grpc_out=${PROTO_GEN_DIR}
    --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    ${PROTO_FILES}
  DEPENDS ${PROTO_FILES} protobuf::protoc gRPC::grpc_cpp_plugin
  COMMENT "Generating robot.proto sources"
  VERBATIM
)

add_library(robot_proto STATIC ${GENERATED_PROTO_SRCS})
target_include_directories(robot_proto PUBLIC ${PROTO_GEN_DIR})
target_link_libraries(robot_proto PUBLIC
  protobuf::libprotobuf
  gRPC::grpc++
)

# --- Domain layer ----------------------------------------------------------
# Pure robot state, thread-safe, no proto dependency.

add_subdirectory(src/domain)

add_library(robot_service STATIC
  proto_mapping.cc
  robot_service_impl.cc
)

target_include_directories(robot_service PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(robot_service
  PUBLIC  robot_proto
  PRIVATE robot_model
)

target_compile_features(robot_service PUBLIC cxx_std_20)

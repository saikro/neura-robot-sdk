add_executable(robot_server main.cc)

target_link_libraries(robot_server PRIVATE
  robot_service
  robot_model
)

target_compile_features(robot_server PRIVATE cxx_std_20)

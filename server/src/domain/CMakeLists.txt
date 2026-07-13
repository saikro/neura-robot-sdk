add_library(robot_model STATIC
  motion_command.cc
  robot_config.cc
  robot_model.cc
)

target_include_directories(robot_model PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(robot_model PUBLIC cxx_std_20)

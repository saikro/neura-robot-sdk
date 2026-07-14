find_package(GTest CONFIG REQUIRED)

add_executable(robot_server_tests
  robot_model_test.cc
  robot_service_impl_test.cc
)

target_link_libraries(robot_server_tests PRIVATE
  robot_model
  robot_service
  GTest::gtest_main
)

target_compile_features(robot_server_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(robot_server_tests)

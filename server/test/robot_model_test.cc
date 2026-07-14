#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <robot/domain/motion_command.h>
#include <robot/domain/robot_config.h>
#include <robot/domain/robot_mode.h>
#include <robot/domain/robot_model.h>

namespace {

robot::domain::RobotConfig MakeConfig(std::size_t num_joints = 6) {
  return robot::domain::RobotConfig(num_joints, "SN-TEST", "Model-TEST");
}

}  // namespace

TEST(RobotModelTest, ApplyCommandRejectsOutOfRangeJointIndex) {
  robot::domain::RobotModel model(MakeConfig(6));
  robot::domain::MotionCommand cmd{
      .targets = {{.joint_index = 6, .position_rad = 0.0, .velocity_rad_per_s = 0.0}}};
  EXPECT_EQ(model.ApplyCommand(cmd),
            robot::domain::RobotModel::ApplyResult::INVALID_JOINT_INDEX);
}

TEST(RobotModelTest, ApplyCommandRejectsWhenInErrorState) {
  robot::domain::RobotModel model(MakeConfig(6));
  model.ForceError();
  robot::domain::MotionCommand cmd{
      .targets = {{.joint_index = 0, .position_rad = 0.5, .velocity_rad_per_s = 0.1}}};
  EXPECT_EQ(model.ApplyCommand(cmd),
            robot::domain::RobotModel::ApplyResult::ROBOT_IN_ERROR);
}

TEST(RobotModelTest, ConcurrentReadAndApplyIsSafe) {
  robot::domain::RobotModel model(MakeConfig(6));
  std::atomic<bool> stop{false};

  auto reader = [&] {
    while (!stop.load(std::memory_order_acquire)) {
      auto s = model.Read();
      EXPECT_EQ(s.joint_positions_rad.size(), 6u);
      EXPECT_EQ(s.joint_velocities_rad_per_s.size(), 6u);
    }
  };
  auto writer = [&] {
    robot::domain::MotionCommand cmd{
        .targets = {{.joint_index = 0, .position_rad = 0.5, .velocity_rad_per_s = 0.1},
                    {.joint_index = 3, .position_rad = -0.3, .velocity_rad_per_s = 0.05}}};
    while (!stop.load(std::memory_order_acquire)) {
      (void)model.ApplyCommand(cmd);
    }
  };

  std::vector<std::thread> threads;
  for (int i = 0; i < 4; ++i) threads.emplace_back(reader);
  for (int i = 0; i < 2; ++i) threads.emplace_back(writer);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  stop.store(true, std::memory_order_release);
  for (auto& t : threads) t.join();
}

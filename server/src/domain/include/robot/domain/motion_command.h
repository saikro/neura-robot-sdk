#pragma once

#include <cstdint>
#include <vector>

#include <robot/domain/robot_config.h>

namespace robot::domain {

struct JointTarget {
  std::uint32_t joint_index;
  double position_rad;
  double velocity_rad_per_s;
};

struct MotionCommand {
  std::vector<JointTarget> targets;

  static bool IsValid(const MotionCommand& command, const RobotConfig& config);
};

}  // namespace robot::domain

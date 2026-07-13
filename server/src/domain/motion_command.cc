#include <robot/domain/motion_command.h>

namespace robot::domain {

bool MotionCommand::IsValid(const MotionCommand& command,
                            const RobotConfig& config) {
  for (const auto& target : command.targets) {
    if (target.joint_index >= config.NumJoints()) {
      return false;
    }
  }
  return true;
}

}  // namespace robot::domain

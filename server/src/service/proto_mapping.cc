#include <robot/service/proto_mapping.h>

namespace robot::service {

robot::v1::RobotState ToProto(robot::domain::RobotMode mode) {
  switch (mode) {
    case robot::domain::RobotMode::IDLE:    return robot::v1::ROBOT_STATE_IDLE;
    case robot::domain::RobotMode::WORKING: return robot::v1::ROBOT_STATE_WORKING;
    case robot::domain::RobotMode::ERROR:   return robot::v1::ROBOT_STATE_ERROR;
  }
  return robot::v1::ROBOT_STATE_UNSPECIFIED;
}

}  // namespace robot::service

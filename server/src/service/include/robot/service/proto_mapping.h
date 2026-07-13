#pragma once

#include <robot/domain/robot_mode.h>
#include <robot/v1/robot.pb.h>

namespace robot::service {

robot::v1::RobotState ToProto(robot::domain::RobotMode mode);

}  // namespace robot::service

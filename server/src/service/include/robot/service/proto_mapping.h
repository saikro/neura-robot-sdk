#pragma once

#include <robot/domain/motion_command.h>
#include <robot/domain/robot_mode.h>
#include <robot/domain/robot_model.h>
#include <robot/v1/robot.pb.h>

namespace robot::service {

robot::v1::RobotState        ToProto(robot::domain::RobotMode mode);
robot::v1::Telemetry         ToProto(const robot::domain::RobotModel::Snapshot& snapshot);
robot::domain::MotionCommand FromProto(const robot::v1::MotionCommand& cmd);

}  // namespace robot::service

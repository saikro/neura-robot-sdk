#include <robot/service/proto_mapping.h>

#include <chrono>

#include <google/protobuf/util/time_util.h>

namespace robot::service {

robot::v1::RobotState ToProto(robot::domain::RobotMode mode) {
  switch (mode) {
    case robot::domain::RobotMode::IDLE:    return robot::v1::ROBOT_STATE_IDLE;
    case robot::domain::RobotMode::WORKING: return robot::v1::ROBOT_STATE_WORKING;
    case robot::domain::RobotMode::ERROR:   return robot::v1::ROBOT_STATE_ERROR;
  }
  return robot::v1::ROBOT_STATE_UNSPECIFIED;
}

robot::v1::Telemetry ToProto(const robot::domain::RobotModel::Snapshot& snapshot) {
  robot::v1::Telemetry telemetry;
  const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                         snapshot.timestamp.time_since_epoch())
                         .count();
  *telemetry.mutable_timestamp() =
      google::protobuf::util::TimeUtil::NanosecondsToTimestamp(nanos);
  auto* positions = telemetry.mutable_joint_positions();
  positions->Assign(snapshot.joint_positions_rad.begin(),
                    snapshot.joint_positions_rad.end());
  auto* velocities = telemetry.mutable_joint_velocities();
  velocities->Assign(snapshot.joint_velocities_rad_per_s.begin(),
                     snapshot.joint_velocities_rad_per_s.end());
  telemetry.set_state(ToProto(snapshot.mode));
  return telemetry;
}

}  // namespace robot::service

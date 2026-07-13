#include <robot/service/robot_service_impl.h>

#include <robot/domain/robot_model.h>
#include <robot/service/proto_mapping.h>

namespace robot::service {

RobotServiceImpl::RobotServiceImpl(robot::domain::RobotModel& model)
    : model_(model) {}

grpc::Status RobotServiceImpl::GetRobotInfo(
    grpc::ServerContext* /*context*/,
    const robot::v1::GetRobotInfoRequest* /*request*/,
    robot::v1::GetRobotInfoResponse* response) {
  const auto snapshot = model_.Read();

  auto* info = response->mutable_info();
  info->set_serial_number(model_.SerialNumber());
  info->set_model(model_.ModelName());
  if (const auto& name = model_.Name(); name.has_value()) {
    info->set_name(*name);
  }
  info->set_state(ToProto(snapshot.mode));
  return grpc::Status::OK;
}

}  // namespace robot::service

#include <robot/service/robot_service_impl.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <thread>

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

grpc::Status RobotServiceImpl::StreamTelemetry(
    grpc::ServerContext* context,
    const robot::v1::StreamTelemetryRequest* request,
    grpc::ServerWriter<robot::v1::Telemetry>* writer) {
  constexpr uint32_t kMinHz = 1;
  constexpr uint32_t kMaxHz = 100;

  const uint32_t requested = request->rate_hz();
  if (requested == 0) {
    return {grpc::StatusCode::INVALID_ARGUMENT, "rate_hz must be >= 1"};
  }
  const uint32_t rate_hz = std::clamp(requested, kMinHz, kMaxHz);
  const auto period = std::chrono::nanoseconds(std::chrono::seconds(1)) / rate_hz;

  while (!context->IsCancelled()) {
    if (!writer->Write(ToProto(model_.Read()))) break;
    std::this_thread::sleep_for(period);
  }
  return grpc::Status::OK;
}

}  // namespace robot::service

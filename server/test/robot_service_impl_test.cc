#include <memory>
#include <utility>

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <robot/domain/robot_config.h>
#include <robot/domain/robot_model.h>
#include <robot/service/robot_service_impl.h>
#include <robot/v1/robot.grpc.pb.h>

namespace {

class InProcessServer {
 public:
  explicit InProcessServer(robot::domain::RobotModel& model) : service_(model) {
    grpc::ServerBuilder builder;
    builder.RegisterService(&service_);
    server_ = builder.BuildAndStart();
    stub_   = robot::v1::RobotService::NewStub(
        server_->InProcessChannel(grpc::ChannelArguments{}));
  }
  ~InProcessServer() { server_->Shutdown(); }

  robot::v1::RobotService::Stub* stub() { return stub_.get(); }

 private:
  robot::service::RobotServiceImpl                 service_;
  std::unique_ptr<grpc::Server>                    server_;
  std::unique_ptr<robot::v1::RobotService::Stub>   stub_;
};

}  // namespace

TEST(GetRobotInfoTest, OmitsNameWhenUnset) {
  robot::domain::RobotModel model(robot::domain::RobotConfig(6, "SN-1", "Model-A"));
  InProcessServer server(model);

  grpc::ClientContext ctx;
  robot::v1::GetRobotInfoRequest  req;
  robot::v1::GetRobotInfoResponse resp;
  auto status = server.stub()->GetRobotInfo(&ctx, req, &resp);

  ASSERT_TRUE(status.ok()) << status.error_message();
  EXPECT_EQ(resp.info().serial_number(), "SN-1");
  EXPECT_EQ(resp.info().model(),         "Model-A");
  EXPECT_FALSE(resp.info().has_name());
}

TEST(GetRobotInfoTest, PopulatesNameWhenSet) {
  robot::domain::RobotModel model(
      robot::domain::RobotConfig(6, "SN-1", "Model-A", std::string("arm-01")));
  InProcessServer server(model);

  grpc::ClientContext ctx;
  robot::v1::GetRobotInfoRequest  req;
  robot::v1::GetRobotInfoResponse resp;
  auto status = server.stub()->GetRobotInfo(&ctx, req, &resp);

  ASSERT_TRUE(status.ok()) << status.error_message();
  ASSERT_TRUE(resp.info().has_name());
  EXPECT_EQ(resp.info().name(), "arm-01");
}

TEST(StreamTelemetryTest, RejectsZeroRate) {
  robot::domain::RobotModel model;
  InProcessServer server(model);

  grpc::ClientContext ctx;
  robot::v1::StreamTelemetryRequest req;
  req.set_rate_hz(0);
  auto reader = server.stub()->StreamTelemetry(&ctx, req);

  robot::v1::Telemetry msg;
  EXPECT_FALSE(reader->Read(&msg));
  auto status = reader->Finish();
  EXPECT_EQ(status.error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST(StreamTelemetryTest, DeliversSamplesAndTerminatesOnClientCancel) {
  robot::domain::RobotModel model;
  InProcessServer server(model);

  grpc::ClientContext ctx;
  robot::v1::StreamTelemetryRequest req;
  req.set_rate_hz(50);
  auto reader = server.stub()->StreamTelemetry(&ctx, req);

  constexpr int kExpectedSamples = 5;
  int received = 0;
  robot::v1::Telemetry msg;
  while (received < kExpectedSamples && reader->Read(&msg)) ++received;

  ctx.TryCancel();
  while (reader->Read(&msg)) {}   // drain anything already in-flight
  auto status = reader->Finish();

  EXPECT_GE(received, kExpectedSamples);
  EXPECT_EQ(status.error_code(), grpc::StatusCode::CANCELLED);
}

#pragma once

#include <robot/v1/robot.grpc.pb.h>

namespace robot::domain {
class RobotModel;
}

namespace robot::service {

class RobotServiceImpl final : public robot::v1::RobotService::Service {
 public:
  explicit RobotServiceImpl(robot::domain::RobotModel& model);

  grpc::Status GetRobotInfo(grpc::ServerContext* context,
                            const robot::v1::GetRobotInfoRequest* request,
                            robot::v1::GetRobotInfoResponse* response) override;

 private:
  robot::domain::RobotModel& model_;
};

}  // namespace robot::service

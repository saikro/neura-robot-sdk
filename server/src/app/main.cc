#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include <robot/domain/robot_model.h>
#include <robot/service/robot_service_impl.h>

int main() {
  robot::domain::RobotModel model;
  robot::service::RobotServiceImpl service(model);

  const std::string address = "0.0.0.0:50051";

  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "Failed to start server on " << address << std::endl;
    return 1;
  }

  std::cout << "robot_server listening on " << address << std::endl;
  server->Wait();
  return 0;
}

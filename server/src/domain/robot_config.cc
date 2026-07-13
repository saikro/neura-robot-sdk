#include <robot/domain/robot_config.h>

#include <utility>

namespace robot::domain {

RobotConfig::RobotConfig()
    : RobotConfig(6, "SN-0000", "neura-arm-v1", std::nullopt) {}

RobotConfig::RobotConfig(std::size_t num_joints,
                         std::string serial_number,
                         std::string model,
                         std::optional<std::string> name)
    : num_joints_(num_joints),
      serial_number_(std::move(serial_number)),
      model_(std::move(model)),
      name_(std::move(name)) {}

std::size_t RobotConfig::NumJoints() const noexcept {
  return num_joints_;
}

const std::string& RobotConfig::SerialNumber() const noexcept {
  return serial_number_;
}

const std::string& RobotConfig::Model() const noexcept {
  return model_;
}

const std::optional<std::string>& RobotConfig::Name() const noexcept {
  return name_;
}

}  // namespace robot::domain

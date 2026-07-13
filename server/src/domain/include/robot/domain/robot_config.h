#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace robot::domain {

class RobotConfig {
 public:
  RobotConfig();

  RobotConfig(std::size_t num_joints,
              std::string serial_number,
              std::string model,
              std::optional<std::string> name = std::nullopt);

  std::size_t                       NumJoints()    const noexcept;
  const std::string&                SerialNumber() const noexcept;
  const std::string&                Model()        const noexcept;
  const std::optional<std::string>& Name()         const noexcept;

 private:
  std::size_t num_joints_;
  std::string serial_number_;
  std::string model_;
  std::optional<std::string> name_;
};

}  // namespace robot::domain

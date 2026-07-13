#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include <robot/domain/motion_command.h>
#include <robot/domain/robot_config.h>
#include <robot/domain/robot_mode.h>

namespace robot::domain {

// Approximates one physical robot. Thread-safe: many concurrent readers
// (telemetry, LiveControl writer) plus one writer (LiveControl reader) may
// touch it simultaneously.
class RobotModel {
 public:
  struct Snapshot {
    std::vector<double> joint_positions_rad;
    std::vector<double> joint_velocities_rad_per_s;
    RobotMode mode;
    std::chrono::system_clock::time_point timestamp;
  };

  enum class ApplyResult {
    OK,
    INVALID_JOINT_INDEX,
    ROBOT_IN_ERROR,
  };

  RobotModel();
  explicit RobotModel(RobotConfig config);
  ~RobotModel();

  RobotModel(const RobotModel&)            = delete;
  RobotModel& operator=(const RobotModel&) = delete;
  RobotModel(RobotModel&&)                 = delete;
  RobotModel& operator=(RobotModel&&)      = delete;

  // Returns a copy — no reference to internal state escapes.
  Snapshot Read() const;

  // Last-write-wins: overwrites the current target for the referenced joints.
  // Rejects the whole command if any joint_index is out of range.
  ApplyResult ApplyCommand(const MotionCommand& cmd);

  // Immutable after construction; safe to read without locking.
  const std::string&                SerialNumber() const noexcept;
  const std::string&                ModelName()    const noexcept;
  const std::optional<std::string>& Name()         const noexcept;

  // Force the robot into ERROR — exercises the error-handling paths.
  void ForceError();

 private:
  void RunTicker();
  void Step(std::chrono::duration<double> dt);

  RobotConfig config_;

  mutable std::shared_mutex m_;
  std::vector<double> positions_rad_;
  std::vector<double> velocities_rad_per_s_;
  std::vector<double> target_positions_rad_;
  std::vector<double> target_velocities_rad_per_s_;
  RobotMode mode_ = RobotMode::IDLE;

  std::atomic<bool> stop_{false};
  std::thread ticker_;
};

}  // namespace robot::domain

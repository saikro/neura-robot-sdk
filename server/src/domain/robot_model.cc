#include <robot/domain/robot_model.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <utility>

namespace robot::domain {

namespace {

// 100 Hz internal simulation. Faster than any client is likely to sample; keeps
// the observable state smooth without burning CPU.
constexpr auto kTickPeriod = std::chrono::milliseconds(10);

// Applied when a command omits a velocity (velocity == 0) but requests motion.
constexpr double kDefaultSpeedLimit = 1.0;   // rad/s
constexpr double kVelocityDeadband  = 1e-6;  // treat |v| below this as at-rest

}  // namespace

RobotModel::RobotModel() : RobotModel(RobotConfig{}) {}

RobotModel::RobotModel(RobotConfig config)
    : config_(std::move(config)),
      positions_rad_(config_.NumJoints(), 0.0),
      velocities_rad_per_s_(config_.NumJoints(), 0.0),
      target_positions_rad_(config_.NumJoints(), 0.0),
      target_velocities_rad_per_s_(config_.NumJoints(), 0.0) {
  ticker_ = std::jthread([this](std::stop_token stop) { RunTicker(std::move(stop)); });
}

RobotModel::~RobotModel() = default;

const std::string& RobotModel::SerialNumber() const noexcept {
  return config_.SerialNumber();
}

const std::string& RobotModel::ModelName() const noexcept {
  return config_.Model();
}

const std::optional<std::string>& RobotModel::Name() const noexcept {
  return config_.Name();
}

RobotModel::Snapshot RobotModel::Read() const {
  std::shared_lock lock(m_);
  return Snapshot{
      .joint_positions_rad        = positions_rad_,
      .joint_velocities_rad_per_s = velocities_rad_per_s_,
      .mode                       = mode_,
      .timestamp                  = std::chrono::system_clock::now(),
  };
}

RobotModel::ApplyResult RobotModel::ApplyCommand(const MotionCommand& cmd) {
  std::unique_lock lock(m_);
  if (mode_ == RobotMode::ERROR) return ApplyResult::ROBOT_IN_ERROR;

  if (!MotionCommand::IsValid(cmd, config_)) {
    return ApplyResult::INVALID_JOINT_INDEX;
  }
  for (const auto& t : cmd.targets) {
    target_positions_rad_[t.joint_index]        = t.position_rad;
    target_velocities_rad_per_s_[t.joint_index] = t.velocity_rad_per_s;
  }
  return ApplyResult::OK;
}

void RobotModel::ForceError() {
  std::unique_lock lock(m_);
  mode_ = RobotMode::ERROR;
  std::fill(velocities_rad_per_s_.begin(), velocities_rad_per_s_.end(), 0.0);
}

void RobotModel::RunTicker(std::stop_token stop) {
  auto next = std::chrono::steady_clock::now();
  while (!stop.stop_requested()) {
    next += kTickPeriod;
    std::this_thread::sleep_until(next);
    Step(kTickPeriod);
  }
}

void RobotModel::Step(std::chrono::duration<double> dt) {
  std::unique_lock lock(m_);
  if (mode_ == RobotMode::ERROR) return;

  const double dt_s = dt.count();
  bool moving = false;
  for (std::size_t i = 0; i < positions_rad_.size(); ++i) {
    const double err      = target_positions_rad_[i] - positions_rad_[i];
    const double speed    = std::max(std::abs(target_velocities_rad_per_s_[i]), kDefaultSpeedLimit);
    const double max_step = speed * dt_s;
    const double step     = std::clamp(err, -max_step, max_step);
    positions_rad_[i]        += step;
    velocities_rad_per_s_[i]  = step / dt_s;
    if (std::abs(velocities_rad_per_s_[i]) > kVelocityDeadband) moving = true;
  }
  mode_ = moving ? RobotMode::WORKING : RobotMode::IDLE;
}

}  // namespace robot::domain

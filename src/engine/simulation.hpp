#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "entity.hpp"
#include "flight_computer.hpp"
#include "gravity_model.hpp"
#include "physics.hpp"
#include "sensor_system.hpp"

namespace orbit {

class TelemetryHub;
class ReplayRecorder;

struct SimulationConfig {
  std::string scenario{"smoke"};
  std::uint32_t rate_hz{60};
  std::uint32_t max_ticks{60};
  bool realtime{true};
  bool telemetry_enabled{false};
  std::string udp_endpoint{"127.0.0.1:9000"};
  bool flight_computer_actuation{true};
  bool sensors_enabled{true};
  GravityModel gravity{GravityModel::TwoBody};
  bool parallel_gravity{false};
  std::string record_path;
  std::string replay_path;
};

class Simulation {
 public:
  explicit Simulation(SimulationConfig config);
  ~Simulation();

  int run();
  void step();

  [[nodiscard]] std::uint64_t tick_index() const { return tick_index_; }
  [[nodiscard]] double sim_time_s() const { return sim_time_s_; }
  [[nodiscard]] const EntityPool& entities() const { return pool_; }
  [[nodiscard]] const FlightComputerRegistry& flight_computers() const {
    return flight_computers_;
  }
  [[nodiscard]] const SensorSystem& sensors() const { return sensors_; }
  [[nodiscard]] std::string state_hash_hex() const;
  [[nodiscard]] std::uint64_t telemetry_dropped_frames() const;
  [[nodiscard]] std::uint64_t telemetry_sent_frames() const;

  static std::string run_headless_hash(const std::string& scenario,
                                       std::uint32_t ticks,
                                       std::uint32_t rate_hz = 60);

  /// Playback a recorded session (no physics integration).
  static int run_replay(const std::string& replay_path);

 private:
  void configure_physics_from_scenario();
  void run_flight_computers(double dt);
  void run_sensors(double dt);
  void advance_physics();
  void publish_telemetry();

  SimulationConfig config_;
  EntityPool pool_;
  PhysicsEngine physics_;
  FlightComputerRegistry flight_computers_;
  SensorSystem sensors_;
  std::unique_ptr<TelemetryHub> telemetry_;
  std::unique_ptr<ReplayRecorder> recorder_;
  std::uint64_t tick_index_{0};
  double sim_time_s_{0.0};
};

}  // namespace orbit

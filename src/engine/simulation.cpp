#include "simulation.hpp"

#include <iostream>

#include "clock.hpp"
#include "scenario.hpp"
#include "snapshot_builder.hpp"
#include "state_hash.hpp"
#include "replay.hpp"
#include "telemetry_hub.hpp"

namespace orbit {

Simulation::Simulation(SimulationConfig config) : config_(std::move(config)) {
  if (!load_scenario(config_.scenario, pool_)) {
    load_circular_leo_preset(pool_);
  }

  configure_physics_from_scenario();

  flight_computers_.set_actuation_enabled(config_.flight_computer_actuation);
  flight_computers_.sync_pool(pool_);

  if (config_.sensors_enabled) {
    sensors_.reset_for_pool(pool_);
  }

  if (config_.telemetry_enabled) {
    telemetry_ = std::make_unique<TelemetryHub>(config_.udp_endpoint);
    telemetry_->start();
  }

  if (!config_.record_path.empty()) {
    recorder_ = std::make_unique<ReplayRecorder>();
    if (!recorder_->open(config_.record_path)) {
      std::cerr << "Failed to open replay recorder: " << config_.record_path << '\n';
    }
  }
}

void Simulation::configure_physics_from_scenario() {
  PhysicsConfig physics_config;
  physics_config.gravity = config_.gravity;
  physics_config.use_parallel_gravity = config_.parallel_gravity;

  if (config_.scenario == "n_body_sanity") {
    physics_config.gravity = GravityModel::NBodyJ2Moon;
    physics_config.use_parallel_gravity = true;
  }

  physics_.set_config(physics_config);
}

Simulation::~Simulation() {
  if (telemetry_) {
    telemetry_->stop();
  }
}

void Simulation::run_flight_computers(const double dt) {
  flight_computers_.begin_tick(tick_index_, sim_time_s_);
  flight_computers_.tick(pool_, dt);
}

void Simulation::run_sensors(const double dt) {
  if (!config_.sensors_enabled) {
    return;
  }
  sensors_.update(pool_, tick_index_, dt);
}

void Simulation::advance_physics() {
  const double dt = 1.0 / static_cast<double>(config_.rate_hz);
  physics_.step(pool_, dt, sim_time_s_);
  ++tick_index_;
  sim_time_s_ += dt;
}

void Simulation::publish_telemetry() {
  if (!telemetry_ && !recorder_) {
    return;
  }

  const auto& tracks = config_.sensors_enabled ? sensors_.tracks()
                                                 : std::vector<TrackReportSnap>{};
  const SimulationSnapshot snapshot = make_snapshot(
      pool_, tick_index_, sim_time_s_, flight_computers_.statuses(), tracks);

  if (recorder_) {
    const std::uint64_t dropped =
        telemetry_ ? telemetry_->dropped_frames() : 0;
    (void)recorder_->write_tick(snapshot, dropped);
  }

  if (!telemetry_) {
    return;
  }
  telemetry_->try_enqueue(snapshot);
}

void Simulation::step() {
  const double dt = 1.0 / static_cast<double>(config_.rate_hz);
  run_flight_computers(dt);
  advance_physics();
  run_sensors(dt);
  publish_telemetry();
}

std::string Simulation::state_hash_hex() const { return state_sha256_hex(pool_); }

std::uint64_t Simulation::telemetry_dropped_frames() const {
  return telemetry_ ? telemetry_->dropped_frames() : 0;
}

std::uint64_t Simulation::telemetry_sent_frames() const {
  return telemetry_ ? telemetry_->sent_frames() : 0;
}

std::string Simulation::run_headless_hash(const std::string& scenario,
                                          const std::uint32_t ticks,
                                          const std::uint32_t rate_hz) {
  SimulationConfig config;
  config.scenario = scenario;
  config.rate_hz = rate_hz;
  config.max_ticks = ticks;
  config.realtime = false;
  config.telemetry_enabled = false;
  config.flight_computer_actuation = false;
  config.sensors_enabled = false;
  config.gravity = GravityModel::TwoBody;

  Simulation sim(config);
  for (std::uint32_t i = 0; i < ticks; ++i) {
    sim.step();
  }
  return sim.state_hash_hex();
}

int Simulation::run() {
  if (!physics_.initialized() || !flight_computers_.ready()) {
    return 1;
  }

  const double dt = 1.0 / static_cast<double>(config_.rate_hz);

  if (config_.realtime) {
    FixedTickClock clock(config_.rate_hz);
    for (std::uint32_t i = 0; i < config_.max_ticks; ++i) {
      run_flight_computers(dt);
      advance_physics();
      run_sensors(dt);
      publish_telemetry();
      clock.wait_next_tick();
    }
  } else {
    for (std::uint32_t i = 0; i < config_.max_ticks; ++i) {
      step();
    }
  }

  if (telemetry_) {
    telemetry_->stop();
    telemetry_.reset();
  }

  if (recorder_) {
    std::cout << "Recorded " << recorder_->frames_written() << " frames to "
              << config_.record_path << '\n';
    recorder_->close();
    recorder_.reset();
  }

  return 0;
}

int Simulation::run_replay(const std::string& replay_path) {
  return ReplayPlayer::print_summary(replay_path);
}

}  // namespace orbit

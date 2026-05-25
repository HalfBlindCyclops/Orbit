#include <cstdlib>
#include <iostream>
#include <string>

#include "simulation.hpp"

namespace {

void print_usage() {
  std::cerr << "Usage: orbit_sim [--scenario NAME] [--duration SEC] [--rate HZ] [--headless]\n"
               "              [--udp HOST:PORT] [--no-fc-actuation] [--no-sensors]\n"
               "              [--advanced-physics] [--record FILE.orbitreplay]\n"
               "              [--replay FILE.orbitreplay]\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  orbit::SimulationConfig config;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--scenario" && i + 1 < argc) {
      config.scenario = argv[++i];
    } else if (arg == "--duration" && i + 1 < argc) {
      const int seconds = std::atoi(argv[++i]);
      if (seconds > 0) {
        config.max_ticks = static_cast<std::uint32_t>(seconds * config.rate_hz);
      }
    } else if (arg == "--rate" && i + 1 < argc) {
      const int rate = std::atoi(argv[++i]);
      if (rate > 0) {
        config.rate_hz = static_cast<std::uint32_t>(rate);
      }
    } else if (arg == "--headless") {
      config.realtime = false;
    } else if (arg == "--udp" && i + 1 < argc) {
      config.telemetry_enabled = true;
      config.udp_endpoint = argv[++i];
    } else if (arg == "--no-fc-actuation") {
      config.flight_computer_actuation = false;
    } else if (arg == "--no-sensors") {
      config.sensors_enabled = false;
    } else if (arg == "--advanced-physics") {
      config.gravity = orbit::GravityModel::NBodyJ2Moon;
      config.parallel_gravity = true;
    } else if (arg == "--record" && i + 1 < argc) {
      config.record_path = argv[++i];
    } else if (arg == "--replay" && i + 1 < argc) {
      config.replay_path = argv[++i];
    } else if (arg == "--help" || arg == "-h") {
      print_usage();
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << '\n';
      print_usage();
      return 1;
    }
  }

  if (!config.replay_path.empty()) {
    return orbit::Simulation::run_replay(config.replay_path);
  }

  orbit::Simulation sim(config);
  return sim.run();
}

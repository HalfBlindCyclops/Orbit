#include <benchmark/benchmark.h>

#include "simulation.hpp"

namespace {

orbit::SimulationConfig bench_config() {
  orbit::SimulationConfig config;
  config.scenario = "circular_leo";
  config.rate_hz = 60;
  config.max_ticks = 1;
  config.realtime = false;
  config.telemetry_enabled = false;
  config.flight_computer_actuation = false;
  config.sensors_enabled = true;
  return config;
}

}  // namespace

static void BM_SimulationFullTick(benchmark::State& state) {
  orbit::SimulationConfig config = bench_config();
  orbit::Simulation sim(config);

  for (auto _ : state) {
    sim.step();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_SimulationFullTick);

static void BM_SimulationAdvancedPhysicsTick(benchmark::State& state) {
  orbit::SimulationConfig config = bench_config();
  config.scenario = "n_body_sanity";
  config.gravity = orbit::GravityModel::NBodyJ2Moon;
  config.parallel_gravity = true;
  orbit::Simulation sim(config);

  for (auto _ : state) {
    sim.step();
  }

  state.SetItemsProcessed(static_cast<int64_t>(state.iterations()));
}
BENCHMARK(BM_SimulationAdvancedPhysicsTick);

BENCHMARK_MAIN();

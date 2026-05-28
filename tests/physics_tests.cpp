#include <gtest/gtest.h>

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include "clock.hpp"
#include "constants.hpp"
#include "physics.hpp"
#include "scenario.hpp"
#include "simulation.hpp"
#include "types.hpp"

namespace {

constexpr double kPi = 3.14159265358979323846;

std::string read_golden_file(const char* relative_path) {
  const std::string candidates[] = {
      relative_path,
      std::string("../") + relative_path,
      std::string(ORBIT_SOURCE_DIR) + "/" + relative_path,
  };

  std::ifstream file;
  for (const auto& path : candidates) {
    file.open(path);
    if (file) {
      break;
    }
  }
  if (!file) {
    return {};
  }
  std::string line;
  std::getline(file, line);
  return line;
}

}  // namespace

TEST(Physics, Vec3Norm) {
  const orbit::Vec3 v{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(v.norm(), 5.0);
}

TEST(Physics, FixedTickClockPeriod) {
  EXPECT_EQ(orbit::FixedTickClock::kTickPeriodNs, 16'666'666);
}

TEST(Physics, TwoBodyAccelerationMagnitude) {
  const orbit::Vec3 position{orbit::constants::kEarthRadius + 400'000.0, 0.0, 0.0};
  const orbit::Vec3 accel = orbit::two_body_earth_acceleration(position);
  const double expected = orbit::constants::kEarthMu /
                          (position.norm() * position.norm());
  EXPECT_NEAR(accel.norm(), expected, 1e-6 * expected);
}

TEST(Physics, CircularOrbitPeriodOneRevolution) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);
  ASSERT_EQ(pool.entities().size(), 3);

  const auto& ref = pool.entities().front();
  const double semi_major = ref.position.norm();
  const double period = orbit::orbital_period_from_semi_major_axis(semi_major);

  constexpr std::uint32_t rate_hz = 60;
  const auto ticks = static_cast<std::uint32_t>(period * static_cast<double>(rate_hz));

  orbit::SimulationConfig config;
  config.scenario = "circular_leo";
  config.rate_hz = rate_hz;
  config.max_ticks = ticks;
  config.realtime = false;

  orbit::Simulation sim(config);
  const double e0 = orbit::specific_orbital_energy(ref.position, ref.velocity);
  ASSERT_EQ(sim.run(), 0);

  const auto& ref_after = sim.entities().entities().front();
  const double e1 =
      orbit::specific_orbital_energy(ref_after.position, ref_after.velocity);

  const double radius_error =
      std::abs(ref_after.position.norm() - ref.position.norm()) / ref.position.norm();
  EXPECT_LT(radius_error, 0.001);

  const double energy_drift = std::abs((e1 - e0) / e0);
  EXPECT_LT(energy_drift, 1e-4);
}

TEST(Physics, Rk4IntegratorPreservesEnergyShortStep) {
  orbit::OrbitalState state{{6'778'137.0, 0.0, 0.0}, {0.0, 7668.0, 0.0}};
  const double e0 =
      orbit::specific_orbital_energy(state.position, state.velocity);

  const orbit::AccelerationFn accel = orbit::two_body_earth_acceleration;
  state = orbit::integrate_rk4(state, 1.0 / 60.0, accel);
  const double e1 =
      orbit::specific_orbital_energy(state.position, state.velocity);

  EXPECT_NEAR(e0, e1, std::abs(e0) * 1e-9);
}

TEST(Determinism, HeadlessRunIsRepeatable) {
  const auto h1 = orbit::Simulation::run_headless_hash("circular_leo", 10'000, 60);
  const auto h2 = orbit::Simulation::run_headless_hash("circular_leo", 10'000, 60);
  EXPECT_EQ(h1, h2);
  EXPECT_EQ(h1.size(), 64U);
}

TEST(Determinism, GoldenHash10kTicks) {
  const auto hash = orbit::Simulation::run_headless_hash("circular_leo", 10'000, 60);
  const auto golden = read_golden_file("tests/data/golden_10k_tick.sha256");
  ASSERT_FALSE(golden.empty()) << "Missing tests/data/golden_10k_tick.sha256";
  EXPECT_EQ(hash, golden);
}

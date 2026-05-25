#include <gtest/gtest.h>

#include "simulation.hpp"

TEST(Smoke, SimulationRunsZeroExit) {
  orbit::SimulationConfig config;
  config.scenario = "smoke";
  config.max_ticks = 2;
  config.rate_hz = 1000;
  config.realtime = false;
  orbit::Simulation sim(config);
  EXPECT_EQ(sim.run(), 0);
  EXPECT_EQ(sim.tick_index(), 2U);
}

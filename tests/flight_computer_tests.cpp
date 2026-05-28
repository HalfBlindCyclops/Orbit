#include <gtest/gtest.h>

#include <algorithm>

#include "entity.hpp"
#include "flight_computer.hpp"
#include "scenario.hpp"
#include "simulation.hpp"

#ifdef ORBIT_HAS_PROTO
#include "snapshot_builder.hpp"
#include "telemetry_encoder.hpp"
#endif

TEST(FlightComputer, PriorityOrdering) {
  EXPECT_LT(static_cast<int>(orbit::Priority::Critical),
            static_cast<int>(orbit::Priority::High));
  EXPECT_LT(static_cast<int>(orbit::Priority::High),
            static_cast<int>(orbit::Priority::Normal));
}

TEST(FlightComputer, StagingBoostToCoast) {
  orbit::Entity entity;
  entity.id = 3;
  entity.kind = orbit::EntityKind::Interceptor;
  entity.fuel_kg = 400.0;
  entity.mass_kg = 500.0;

  orbit::EntityFlightComputer fc;
  fc.configure_for_entity(entity);
  fc.begin_tick(0, 0.0);
  fc.tick(entity, 0.0, false);
  EXPECT_EQ(fc.status().propulsion_stage, "Boost");

  fc.begin_tick(121, 2.0);
  fc.tick(entity, 0.0, false);
  EXPECT_EQ(fc.status().propulsion_stage, "Coast");
}

TEST(FlightComputer, TerminalPhaseStarvesStationKeeping) {
  orbit::Entity entity;
  entity.id = 3;
  entity.name = "leo_interceptor";
  entity.kind = orbit::EntityKind::Interceptor;
  entity.fuel_kg = 250.0;
  entity.mass_kg = 350.0;

  orbit::EntityFlightComputer fc;
  fc.configure_for_entity(entity);

  fc.begin_tick(0, 0.0);
  fc.tick(entity, 0.0, false);
  EXPECT_EQ(fc.status().propulsion_stage, "Boost");

  fc.begin_tick(121, 2.0);
  fc.tick(entity, 0.0, false);
  EXPECT_EQ(fc.status().propulsion_stage, "Coast");

  fc.begin_tick(5000, 35.0);
  entity.fuel_kg = 250.0;
  fc.tick(entity, 0.0, false);
  EXPECT_EQ(fc.status().propulsion_stage, "Terminal");
  EXPECT_GE(fc.status().power_allocated_w, 400.0);

  const bool starved = std::any_of(
      fc.status().active_faults.begin(), fc.status().active_faults.end(),
      [](const std::string& fault) {
        return fault == "POWER_STARVED:station_keeping";
      });
  EXPECT_TRUE(starved);
}

TEST(FlightComputer, ActuationAppliesThrustAcceleration) {
  orbit::Entity entity;
  entity.id = 3;
  entity.kind = orbit::EntityKind::Interceptor;
  entity.fuel_kg = 500.0;
  entity.mass_kg = 600.0;
  entity.velocity = {0.0, 7668.0, 0.0};

  orbit::EntityFlightComputer fc;
  fc.configure_for_entity(entity);
  fc.begin_tick(1, 0.01);
  fc.tick(entity, 0.01, true);

  EXPECT_GT(fc.status().thrust_n, 0.0);
  EXPECT_GT(entity.thrust_acceleration.norm(), 0.0);
  EXPECT_LT(entity.fuel_kg, 500.0);
}

TEST(FlightComputer, SimulationIntegration) {
  orbit::SimulationConfig config;
  config.scenario = "circular_leo";
  config.max_ticks = 2000;
  config.rate_hz = 60;
  config.realtime = false;
  config.flight_computer_actuation = false;

  orbit::Simulation sim(config);
  ASSERT_EQ(sim.run(), 0);
  ASSERT_FALSE(sim.flight_computers().statuses().empty());

  const auto* interceptor = [&]() -> const orbit::FlightComputerStatusSnap* {
    for (const auto& status : sim.flight_computers().statuses()) {
      if (status.entity_id == 3) {
        return &status;
      }
    }
    return nullptr;
  }();

  ASSERT_NE(interceptor, nullptr);
  EXPECT_EQ(interceptor->propulsion_stage, "Terminal");
}

#ifdef ORBIT_HAS_PROTO

TEST(FlightComputer, TelemetryIncludesFlightComputerStatus) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);

  orbit::FlightComputerRegistry registry;
  registry.sync_pool(pool);
  registry.begin_tick(10, 0.5);
  registry.tick(pool, 1.0 / 60.0);

  const orbit::SimulationSnapshot snap =
      orbit::make_snapshot(pool, 10, 0.5, registry.statuses());
  const std::string wire = orbit::encode_simulation_tick(snap, 0);

  orbit::SimulationSnapshot decoded;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  ASSERT_TRUE(orbit::decode_simulation_tick(wire, decoded, schema, dropped));
  ASSERT_FALSE(decoded.flight_computers.empty());
  EXPECT_EQ(decoded.flight_computers.front().entity_id, 3U);
  EXPECT_FALSE(decoded.flight_computers.front().propulsion_stage.empty());
}

#endif

#include <gtest/gtest.h>

#include <algorithm>

#include "entity.hpp"
#include "gravity_model.hpp"
#include "perturbations.hpp"
#include "scenario.hpp"
#include "sensor_model.hpp"
#include "sensor_system.hpp"
#include "simulation.hpp"
#include "tracker.hpp"

#ifdef ORBIT_HAS_PROTO
#include "snapshot_builder.hpp"
#include "telemetry_encoder.hpp"
#endif

TEST(Sensors, LineOfSightBlockedByEarth) {
  const orbit::Vec3 observer{orbit::constants::kEarthRadius + 500'000.0, 0.0, 0.0};
  const orbit::Vec3 target{-orbit::constants::kEarthRadius - 500'000.0, 0.0, 0.0};
  EXPECT_FALSE(orbit::has_line_of_sight(observer, target));
}

TEST(Sensors, LineOfSightClearAboveEarth) {
  const orbit::Vec3 observer{orbit::constants::kEarthRadius + 700'000.0, 0.0, 0.0};
  const orbit::Vec3 target{orbit::constants::kEarthRadius + 800'000.0, 100'000.0, 0.0};
  EXPECT_TRUE(orbit::has_line_of_sight(observer, target));
}

TEST(Sensors, RadarDetectsCoOrbitalTarget) {
  orbit::Entity observer;
  observer.id = 3;
  observer.kind = orbit::EntityKind::Interceptor;
  observer.position = {orbit::constants::kEarthRadius + 400'000.0, 0.0, 0.0};
  observer.velocity = {0.0, 7668.0, 0.0};

  orbit::Entity target = observer;
  target.id = 1;
  target.kind = orbit::EntityKind::Satellite;
  target.position = {orbit::constants::kEarthRadius + 405'000.0, 20'000.0, 0.0};

  orbit::RadarSensor sensor({3, 2'500'000.0, 0.5});
  const auto measurement = sensor.measure(observer, target, 10);
  EXPECT_TRUE(measurement.valid);
}

TEST(Sensors, TimeToInterceptClosing) {
  orbit::Vec3 interceptor_pos{0.0, 0.0, 0.0};
  orbit::Vec3 interceptor_vel{1000.0, 0.0, 0.0};
  orbit::Vec3 target_pos{100'000.0, 0.0, 0.0};
  orbit::Vec3 target_vel{500.0, 0.0, 0.0};

  const double tti = orbit::compute_time_to_intercept(interceptor_pos, interceptor_vel,
                                                      target_pos, target_vel);
  EXPECT_NEAR(tti, 200.0, 1e-3);
}

TEST(Sensors, SensorSystemProducesTracks) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);

  for (orbit::Entity& entity : pool.entities_mut()) {
    if (entity.id == 3) {
      entity.position = {orbit::constants::kEarthRadius + 400'000.0, 0.0, 0.0};
      entity.velocity = {0.0, 7668.0, 0.0};
    } else if (entity.id == 1) {
      entity.position = {orbit::constants::kEarthRadius + 405'000.0, 20'000.0, 0.0};
    }
  }

  orbit::SensorSystem system;
  system.reset_for_pool(pool);
  system.update(pool, 100, 1.0 / 60.0);

  EXPECT_FALSE(system.tracks().empty());
  const bool has_intercept_track = std::any_of(
      system.tracks().begin(), system.tracks().end(), [](const orbit::TrackReportSnap& track) {
        return track.source_sensor_id == 3 && track.track_id == 1;
      });
  EXPECT_TRUE(has_intercept_track);
}

TEST(Sensors, J2PerturbationNonZeroAtLeo) {
  const orbit::Vec3 position{orbit::constants::kEarthRadius + 400'000.0, 0.0, 0.0};
  const orbit::Vec3 j2 = orbit::j2_acceleration(position);
  EXPECT_GT(j2.norm(), 0.0);
}

TEST(Sensors, ParallelGravityMatchesSerial) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);

  std::vector<orbit::Vec3> serial;
  std::vector<orbit::Vec3> parallel;
  orbit::compute_gravity_batch(pool, orbit::GravityModel::NBodyJ2Moon, 100.0, false, serial);
  orbit::compute_gravity_batch(pool, orbit::GravityModel::NBodyJ2Moon, 100.0, true, parallel);

  ASSERT_EQ(serial.size(), parallel.size());
  for (std::size_t i = 0; i < serial.size(); ++i) {
    EXPECT_NEAR(serial[i].x, parallel[i].x, 1e-12);
    EXPECT_NEAR(serial[i].y, parallel[i].y, 1e-12);
    EXPECT_NEAR(serial[i].z, parallel[i].z, 1e-12);
  }
}

TEST(Sensors, NBodyGravityDiffersFromTwoBody) {
  const orbit::Vec3 position{orbit::constants::kEarthRadius + 400'000.0, 0.0, 0.0};
  const orbit::Vec3 two_body = orbit::compute_gravity_acceleration(position,
                                                                 orbit::GravityModel::TwoBody, 0.0);
  const orbit::Vec3 n_body = orbit::compute_gravity_acceleration(
      position, orbit::GravityModel::NBodyJ2Moon, 0.0);
  EXPECT_NE(two_body.x, n_body.x);
}

TEST(Sensors, NBodySanitySimulationRuns) {
  orbit::SimulationConfig config;
  config.scenario = "n_body_sanity";
  config.max_ticks = 120;
  config.rate_hz = 60;
  config.realtime = false;
  config.sensors_enabled = true;
  config.flight_computer_actuation = false;

  orbit::Simulation sim(config);
  EXPECT_EQ(sim.run(), 0);
  EXPECT_FALSE(sim.sensors().tracks().empty());
}

#ifdef ORBIT_HAS_PROTO

TEST(Sensors, TelemetryIncludesTracks) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);

  orbit::SensorSystem sensors;
  sensors.reset_for_pool(pool);
  sensors.update(pool, 5, 1.0 / 60.0);

  const orbit::SimulationSnapshot snap =
      orbit::make_snapshot(pool, 5, 0.1, {}, sensors.tracks());
  const std::string wire = orbit::encode_simulation_tick(snap, 0);

  orbit::SimulationSnapshot decoded;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  ASSERT_TRUE(orbit::decode_simulation_tick(wire, decoded, schema, dropped));
  ASSERT_FALSE(decoded.tracks.empty());
}

#endif

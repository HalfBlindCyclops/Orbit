#include <gtest/gtest.h>

#include <filesystem>

#include "replay.hpp"
#include "scenario.hpp"
#include "simulation.hpp"
#include "snapshot_builder.hpp"

#ifdef ORBIT_HAS_PROTO
#include "telemetry_encoder.hpp"
#endif

namespace fs = std::filesystem;

#ifdef ORBIT_HAS_PROTO

TEST(Replay, RecordAndPlaybackRoundtrip) {
  const fs::path path = fs::temp_directory_path() / "orbit_test_replay.orbitreplay";

  orbit::SimulationConfig config;
  config.scenario = "smoke";
  config.max_ticks = 30;
  config.rate_hz = 60;
  config.realtime = false;
  config.telemetry_enabled = false;
  config.flight_computer_actuation = false;
  config.sensors_enabled = false;
  config.record_path = path.string();

  {
    orbit::Simulation sim(config);
    ASSERT_EQ(sim.run(), 0);
  }

  ASSERT_TRUE(fs::exists(path));

  orbit::ReplayPlayer player;
  ASSERT_TRUE(player.open(path.string()));

  orbit::SimulationSnapshot snapshot;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  std::uint64_t frames = 0;
  while (player.next_tick(snapshot, schema, dropped)) {
    EXPECT_EQ(schema, 1U);
    EXPECT_GT(snapshot.tick_index, 0U);
    ++frames;
  }

  EXPECT_EQ(frames, 30U);
  fs::remove(path);
}

TEST(Replay, EncodeDecodeMatchesRecorderFrame) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);
  const orbit::SimulationSnapshot snap = orbit::make_snapshot(pool, 7, 0.25);

  orbit::ReplayRecorder recorder;
  const fs::path path = fs::temp_directory_path() / "orbit_single_frame.orbitreplay";
  ASSERT_TRUE(recorder.open(path.string()));
  ASSERT_TRUE(recorder.write_tick(snap, 2));
  recorder.close();

  orbit::ReplayPlayer player;
  ASSERT_TRUE(player.open(path.string()));
  orbit::SimulationSnapshot decoded;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  ASSERT_TRUE(player.next_tick(decoded, schema, dropped));
  EXPECT_EQ(decoded.tick_index, 7U);
  EXPECT_EQ(dropped, 2U);
  EXPECT_EQ(decoded.entities.size(), snap.entities.size());

  fs::remove(path);
}

#else

TEST(Replay, RequiresProtobuf) {
  GTEST_SKIP() << "ORBIT_BUILD_PROTO=OFF";
}

#endif

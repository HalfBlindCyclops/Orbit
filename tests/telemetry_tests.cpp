#include <gtest/gtest.h>

#include "entity.hpp"
#include "flight_computer.hpp"
#include "scenario.hpp"
#include "spsc_queue.hpp"
#include "snapshot_builder.hpp"
#include "telemetry_hub.hpp"

#ifdef ORBIT_HAS_PROTO
#include "telemetry_encoder.hpp"
#endif

TEST(Telemetry, HubStartStop) {
  orbit::TelemetryHub hub("127.0.0.1:9000");
  EXPECT_FALSE(hub.running());
  hub.start();
  EXPECT_TRUE(hub.running());
  hub.stop();
  EXPECT_FALSE(hub.running());
}

TEST(Telemetry, SpscQueuePushPop) {
  orbit::SpscQueue<int, 4> queue;
  EXPECT_TRUE(queue.try_push(42));
  auto value = queue.try_pop();
  ASSERT_TRUE(value.has_value());
  EXPECT_EQ(*value, 42);
}

TEST(Telemetry, SpscQueueFull) {
  // Ring buffer holds Capacity - 1 elements (one slot reserved).
  orbit::SpscQueue<int, 3> queue;
  EXPECT_TRUE(queue.try_push(1));
  EXPECT_TRUE(queue.try_push(2));
  EXPECT_FALSE(queue.try_push(3));
}

#ifdef ORBIT_HAS_PROTO

TEST(Telemetry, ProtobufRoundtrip) {
  orbit::EntityPool pool;
  orbit::load_circular_leo_preset(pool);
  orbit::FlightComputerRegistry registry;
  registry.sync_pool(pool);
  registry.begin_tick(42, 0.7);
  registry.tick(pool, 1.0 / 60.0);

  const orbit::SimulationSnapshot original =
      orbit::make_snapshot(pool, 42, 0.7, registry.statuses());

  const std::string wire = orbit::encode_simulation_tick(original, 3);
  ASSERT_FALSE(wire.empty());

  orbit::SimulationSnapshot decoded;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  ASSERT_TRUE(orbit::decode_simulation_tick(wire, decoded, schema, dropped));

  EXPECT_EQ(schema, 1U);
  EXPECT_EQ(dropped, 3U);
  EXPECT_EQ(decoded.tick_index, original.tick_index);
  EXPECT_EQ(decoded.sim_time_s, original.sim_time_s);
  ASSERT_EQ(decoded.entities.size(), original.entities.size());
  EXPECT_EQ(decoded.entities.front().name, original.entities.front().name);
  EXPECT_DOUBLE_EQ(decoded.entities.front().position.x,
                   original.entities.front().position.x);
}

TEST(Telemetry, HubDropsWhenQueueFull) {
  orbit::TelemetryHub hub("127.0.0.1:9000");
  hub.start();

  orbit::EntityPool pool;
  ASSERT_TRUE(orbit::load_scenario("smoke", pool));

  for (std::size_t i = 0; i < 64; ++i) {
    (void)hub.try_enqueue(orbit::make_snapshot(pool, i, static_cast<double>(i)));
  }

  hub.stop();
  EXPECT_GE(hub.dropped_frames(), 1U);
  EXPECT_GE(hub.sent_frames(), 1U);
}

#else

TEST(Telemetry, ProtobufSkippedWithoutProto) {
  GTEST_SKIP() << "ORBIT_BUILD_PROTO=OFF";
}

#endif

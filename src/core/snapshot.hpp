#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "types.hpp"

namespace orbit {

enum class EntityKind { Satellite, Interceptor };

struct EntitySnapshot {
  EntityId id{0};
  std::string name;
  EntityKind kind{EntityKind::Satellite};
  Vec3 position{};
  Vec3 velocity{};
  double mass_kg{0.0};
  double fuel_kg{0.0};
};

struct FlightComputerStatusSnap {
  EntityId entity_id{0};
  double power_allocated_w{0.0};
  double thrust_n{0.0};
  std::string propulsion_stage;
  std::vector<std::string> active_faults;
};

struct TrackReportSnap {
  std::uint32_t track_id{0};
  std::uint32_t source_sensor_id{0};
  EntityId estimated_entity_id{0};
  Vec3 estimated_position_m{};
  double time_to_intercept_s{-1.0};
};

struct SimulationSnapshot {
  std::uint64_t tick_index{0};
  double sim_time_s{0.0};
  std::uint64_t wall_time_ns{0};
  std::vector<EntitySnapshot> entities;
  std::vector<FlightComputerStatusSnap> flight_computers;
  std::vector<TrackReportSnap> tracks;
};

}  // namespace orbit

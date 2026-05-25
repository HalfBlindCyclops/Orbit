#include "snapshot_builder.hpp"

#include <chrono>

namespace orbit {

SimulationSnapshot make_snapshot(
    const EntityPool& pool, const std::uint64_t tick_index, const double sim_time_s,
    const std::vector<FlightComputerStatusSnap>& flight_computers,
    const std::vector<TrackReportSnap>& tracks) {
  SimulationSnapshot snap;
  snap.tick_index = tick_index;
  snap.sim_time_s = sim_time_s;
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  snap.wall_time_ns = static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
  snap.entities.reserve(pool.entities().size());
  snap.flight_computers = flight_computers;
  snap.tracks = tracks;

  for (const Entity& entity : pool.entities()) {
    EntitySnapshot entity_snap;
    entity_snap.id = entity.id;
    entity_snap.name = entity.name;
    entity_snap.kind = entity.kind;
    entity_snap.position = entity.position;
    entity_snap.velocity = entity.velocity;
    entity_snap.mass_kg = entity.mass_kg;
    entity_snap.fuel_kg = entity.fuel_kg;
    snap.entities.push_back(std::move(entity_snap));
  }

  return snap;
}

}  // namespace orbit

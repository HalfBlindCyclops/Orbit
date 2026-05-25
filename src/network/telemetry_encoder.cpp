#include "telemetry_encoder.hpp"

#include "telemetry.pb.h"

namespace orbit {
namespace {

void fill_vec3(orbit::telemetry::Vec3d* out, const Vec3& v) {
  out->set_x(v.x);
  out->set_y(v.y);
  out->set_z(v.z);
}

Vec3 read_vec3(const orbit::telemetry::Vec3d& v) { return {v.x(), v.y(), v.z()}; }

}  // namespace

std::string encode_simulation_tick(const SimulationSnapshot& snapshot,
                                   const std::uint64_t dropped_frames) {
  orbit::telemetry::SimulationTick tick;
  tick.set_schema_version(1);
  tick.set_tick_index(snapshot.tick_index);
  tick.set_sim_time_s(snapshot.sim_time_s);
  tick.set_wall_time_ns(snapshot.wall_time_ns);
  tick.set_dropped_frames(dropped_frames);

  for (const EntitySnapshot& entity : snapshot.entities) {
    auto* out = tick.add_entities();
    out->set_entity_id(entity.id);
    out->set_name(entity.name);
    fill_vec3(out->mutable_position_m(), entity.position);
    fill_vec3(out->mutable_velocity_mps(), entity.velocity);
    out->set_mass_kg(entity.mass_kg);
    out->set_fuel_kg(entity.fuel_kg);
  }

  for (const FlightComputerStatusSnap& fc : snapshot.flight_computers) {
    auto* out = tick.add_flight_computers();
    out->set_entity_id(fc.entity_id);
    out->set_power_allocated_w(fc.power_allocated_w);
    out->set_thrust_n(fc.thrust_n);
    out->set_propulsion_stage(fc.propulsion_stage);
    for (const std::string& fault : fc.active_faults) {
      out->add_active_faults(fault);
    }
  }

  for (const TrackReportSnap& track : snapshot.tracks) {
    auto* out = tick.add_tracks();
    out->set_track_id(track.track_id);
    out->set_source_sensor_id(track.source_sensor_id);
    fill_vec3(out->mutable_estimated_position_m(), track.estimated_position_m);
    out->set_time_to_intercept_s(track.time_to_intercept_s);
  }

  std::string wire;
  tick.SerializeToString(&wire);
  return wire;
}

bool decode_simulation_tick(const std::string& wire, SimulationSnapshot& out,
                            std::uint32_t& schema_version,
                            std::uint64_t& dropped_frames) {
  orbit::telemetry::SimulationTick tick;
  if (!tick.ParseFromString(wire)) {
    return false;
  }

  schema_version = tick.schema_version();
  dropped_frames = tick.dropped_frames();
  out.tick_index = tick.tick_index();
  out.sim_time_s = tick.sim_time_s();
  out.wall_time_ns = tick.wall_time_ns();
  out.entities.clear();
  out.entities.reserve(static_cast<std::size_t>(tick.entities_size()));

  for (const auto& entity : tick.entities()) {
    EntitySnapshot snap;
    snap.id = entity.entity_id();
    snap.name = entity.name();
    snap.position = read_vec3(entity.position_m());
    snap.velocity = read_vec3(entity.velocity_mps());
    snap.mass_kg = entity.mass_kg();
    snap.fuel_kg = entity.fuel_kg();
    out.entities.push_back(std::move(snap));
  }

  out.flight_computers.clear();
  for (const auto& fc : tick.flight_computers()) {
    FlightComputerStatusSnap snap;
    snap.entity_id = fc.entity_id();
    snap.power_allocated_w = fc.power_allocated_w();
    snap.thrust_n = fc.thrust_n();
    snap.propulsion_stage = fc.propulsion_stage();
    snap.active_faults.reserve(static_cast<std::size_t>(fc.active_faults_size()));
    for (const std::string& fault : fc.active_faults()) {
      snap.active_faults.push_back(fault);
    }
    out.flight_computers.push_back(std::move(snap));
  }

  out.tracks.clear();
  for (const auto& track : tick.tracks()) {
    TrackReportSnap snap;
    snap.track_id = track.track_id();
    snap.source_sensor_id = track.source_sensor_id();
    snap.estimated_position_m = read_vec3(track.estimated_position_m());
    snap.time_to_intercept_s = track.time_to_intercept_s();
    out.tracks.push_back(std::move(snap));
  }

  return true;
}

}  // namespace orbit

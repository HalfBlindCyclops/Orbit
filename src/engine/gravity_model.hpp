#pragma once

#include <cstddef>
#include <vector>

#include "entity.hpp"
#include "types.hpp"

namespace orbit {

enum class GravityModel {
  TwoBody,
  TwoBodyJ2,
  NBodyJ2Moon,
};

struct PhysicsConfig {
  GravityModel gravity{GravityModel::TwoBody};
  bool use_parallel_gravity{false};
  std::size_t parallel_threshold{4};
};

/// Compute total gravitational acceleration for one entity.
Vec3 compute_gravity_acceleration(const Vec3& position, GravityModel model, double sim_time_s);

/// Compute gravity for all entities (optional deterministic worker pool).
void compute_gravity_batch(const EntityPool& pool, GravityModel model, double sim_time_s,
                           bool use_parallel, std::vector<Vec3>& out_accelerations);

}  // namespace orbit

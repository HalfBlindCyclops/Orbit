#include "gravity_model.hpp"

#include "gravity_worker_pool.hpp"
#include "perturbations.hpp"
#include "physics.hpp"

namespace orbit {

Vec3 compute_gravity_acceleration(const Vec3& position, const GravityModel model,
                                  const double sim_time_s) {
  Vec3 accel = two_body_earth_acceleration(position);

  if (model == GravityModel::TwoBody) {
    return accel;
  }

  accel = accel + j2_acceleration(position);

  if (model == GravityModel::NBodyJ2Moon) {
    const Vec3 moon_position = moon_position_eci(sim_time_s);
    accel = accel + n_body_point_mass_acceleration(position, moon_position, constants::kMoonMu);
  }

  return accel;
}

void compute_gravity_batch(const EntityPool& pool, const GravityModel model,
                           const double sim_time_s, const bool use_parallel,
                           std::vector<Vec3>& out_accelerations) {
  const auto& entities = pool.entities();
  out_accelerations.resize(entities.size());

  const auto compute_index = [&](const std::size_t index) {
    out_accelerations[index] =
        compute_gravity_acceleration(entities[index].position, model, sim_time_s);
  };

  if (!use_parallel || entities.size() < 4) {
    for (std::size_t i = 0; i < entities.size(); ++i) {
      compute_index(i);
    }
    return;
  }

  static const DeterministicWorkerPool pool_workers;
  pool_workers.parallel_for(entities.size(), compute_index);
}

}  // namespace orbit

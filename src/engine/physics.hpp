#pragma once

#include <functional>

#include "entity.hpp"
#include "gravity_model.hpp"
#include "types.hpp"

namespace orbit {

using AccelerationFn = std::function<Vec3(const Vec3& position)>;

OrbitalState integrate_rk4(const OrbitalState& state, double dt,
                           const AccelerationFn& acceleration);

Vec3 two_body_earth_acceleration(const Vec3& position_eci);

double specific_orbital_energy(const Vec3& position, const Vec3& velocity);
double orbital_period_from_semi_major_axis(double semi_major_axis);

class PhysicsEngine {
 public:
  void set_config(PhysicsConfig config) { config_ = config; }
  [[nodiscard]] const PhysicsConfig& config() const { return config_; }

  void step(EntityPool& pool, double dt, double sim_time_s);

  [[nodiscard]] bool initialized() const { return true; }

 private:
  PhysicsConfig config_{};
};

}  // namespace orbit

#include "physics.hpp"

#include <cmath>

#include "constants.hpp"
#include "gravity_model.hpp"

namespace orbit {
namespace {

constexpr double kPi = 3.14159265358979323846;

OrbitalDerivative evaluate(const OrbitalState& state,
                           const AccelerationFn& acceleration) {
  return {state.velocity, acceleration(state.position)};
}

}  // namespace

Vec3 two_body_earth_acceleration(const Vec3& position_eci) {
  const double r2 = position_eci.norm_squared();
  if (r2 < 1.0) {
    return {};
  }
  const double r3 = r2 * std::sqrt(r2);
  return position_eci * (-constants::kEarthMu / r3);
}

OrbitalState integrate_rk4(const OrbitalState& state, double dt,
                           const AccelerationFn& acceleration) {
  const auto k1 = evaluate(state, acceleration);
  const OrbitalState s2 = state + OrbitalState{k1.position_dot, k1.velocity_dot} * (dt * 0.5);
  const auto k2 = evaluate(s2, acceleration);
  const OrbitalState s3 = state + OrbitalState{k2.position_dot, k2.velocity_dot} * (dt * 0.5);
  const auto k3 = evaluate(s3, acceleration);
  const OrbitalState s4 =
      state + OrbitalState{k3.position_dot, k3.velocity_dot} * dt;
  const auto k4 = evaluate(s4, acceleration);

  const OrbitalDerivative combined = (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (dt / 6.0);
  return state + OrbitalState{combined.position_dot, combined.velocity_dot};
}

double specific_orbital_energy(const Vec3& position, const Vec3& velocity) {
  const double r = position.norm();
  const double v2 = velocity.norm_squared();
  return 0.5 * v2 - constants::kEarthMu / r;
}

double orbital_period_from_semi_major_axis(const double semi_major_axis) {
  return 2.0 * kPi * std::sqrt((semi_major_axis * semi_major_axis * semi_major_axis) /
                               constants::kEarthMu);
}

void PhysicsEngine::step(EntityPool& pool, const double dt, const double sim_time_s) {
  for (Entity& entity : pool.entities_mut()) {
    const Vec3 thrust = entity.thrust_acceleration;
    const GravityModel model = config_.gravity;
    const AccelerationFn accel = [model, sim_time_s, thrust](const Vec3& position) {
      return compute_gravity_acceleration(position, model, sim_time_s) + thrust;
    };

    const OrbitalState initial{entity.position, entity.velocity};
    const OrbitalState next = integrate_rk4(initial, dt, accel);
    entity.position = next.position;
    entity.velocity = next.velocity;
    entity.thrust_acceleration = {};
  }
}

}  // namespace orbit

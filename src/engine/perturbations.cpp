#include "perturbations.hpp"

#include <cmath>

#include "constants.hpp"

namespace orbit {
namespace {

constexpr double kPi = 3.14159265358979323846;

}  // namespace

Vec3 j2_acceleration(const Vec3& position_eci) {
  const double x = position_eci.x;
  const double y = position_eci.y;
  const double z = position_eci.z;
  const double r2 = x * x + y * y + z * z;
  if (r2 < 1.0) {
    return {};
  }
  const double r = std::sqrt(r2);
  const double r5 = r2 * r2 * r;
  const double z2_r2 = (z * z) / r2;

  const double factor =
      1.5 * constants::kEarthJ2 * constants::kEarthMu * (constants::kEarthRadius * constants::kEarthRadius) / r5;

  const double ax = factor * x * (5.0 * z2_r2 - 1.0);
  const double ay = factor * y * (5.0 * z2_r2 - 1.0);
  const double az = factor * z * (5.0 * z2_r2 - 3.0);
  return {ax, ay, az};
}

Vec3 n_body_point_mass_acceleration(const Vec3& target_position,
                                    const Vec3& source_position,
                                    const double source_mu) {
  const Vec3 delta = source_position - target_position;
  const double r2 = delta.norm_squared();
  if (r2 < 1.0) {
    return {};
  }
  const double r3 = r2 * std::sqrt(r2);
  return delta * (source_mu / r3);
}

Vec3 moon_position_eci(const double sim_time_s) {
  const double omega = 2.0 * kPi / constants::kMoonOrbitalPeriod;
  const double angle = omega * sim_time_s;
  return {constants::kMoonOrbitRadius * std::cos(angle),
          constants::kMoonOrbitRadius * std::sin(angle), 0.0};
}

}  // namespace orbit

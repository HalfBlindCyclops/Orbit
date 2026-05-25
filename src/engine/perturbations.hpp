#pragma once

#include "types.hpp"

namespace orbit {

/// J2 oblateness perturbation acceleration (ECI, spherical Earth).
Vec3 j2_acceleration(const Vec3& position_eci);

/// Point-mass N-body acceleration on `target` from `source` at `source_position`.
Vec3 n_body_point_mass_acceleration(const Vec3& target_position,
                                    const Vec3& source_position, double source_mu);

/// Simplified circular lunar ephemeris in Earth-centered ECI (m).
Vec3 moon_position_eci(double sim_time_s);

}  // namespace orbit

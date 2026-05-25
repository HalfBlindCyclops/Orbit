#pragma once

#include <cstdint>

#include "constants.hpp"
#include "entity.hpp"
#include "types.hpp"

namespace orbit {

struct SensorConfig {
  EntityId sensor_entity_id{0};
  double max_range_m{2'000'000.0};
  double fov_half_angle_rad{0.35};  // ~20 deg
};

struct SensorMeasurement {
  EntityId target_id{0};
  Vec3 position_m{};
  bool valid{false};
};

/// Line-of-sight test against spherical Earth.
bool has_line_of_sight(const Vec3& observer, const Vec3& target,
                       double earth_radius = constants::kEarthRadius);

/// Abstract radar sensor with FOV + LOS checks.
class RadarSensor {
 public:
  explicit RadarSensor(SensorConfig config);

  [[nodiscard]] SensorMeasurement measure(const Entity& observer, const Entity& target,
                                          std::uint64_t tick_index) const;

 private:
  SensorConfig config_;
};

}  // namespace orbit

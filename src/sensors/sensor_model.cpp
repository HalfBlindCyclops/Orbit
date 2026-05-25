#include "sensor_model.hpp"

#include <cmath>

#include "constants.hpp"

namespace orbit {
namespace {

constexpr double kPi = 3.14159265358979323846;

Vec3 boresight_from_observer(const Entity& observer) {
  const double speed = observer.velocity.norm();
  if (speed > 1.0) {
    return observer.velocity / speed;
  }
  const double radius = observer.position.norm();
  if (radius > 1.0) {
    return observer.position / radius;
  }
  return {1.0, 0.0, 0.0};
}

double deterministic_noise(const std::uint64_t tick, const EntityId a, const EntityId b,
                           const double scale) {
  const std::uint64_t seed = tick * 1'000'003ULL + static_cast<std::uint64_t>(a) * 9'173ULL +
                             static_cast<std::uint64_t>(b) * 3'7ULL;
  const double unit = static_cast<double>((seed % 10'000) - 5'000) / 5'000.0;
  return unit * scale;
}

}  // namespace

bool has_line_of_sight(const Vec3& observer, const Vec3& target, const double earth_radius) {
  const Vec3 segment = target - observer;
  const double denom = segment.norm_squared();
  if (denom < 1.0) {
    return true;
  }

  double s = -observer.dot(segment) / denom;
  if (s < 0.0) {
    s = 0.0;
  } else if (s > 1.0) {
    s = 1.0;
  }

  const Vec3 closest = observer + segment * s;
  return closest.norm() > earth_radius * 1.01;
}

RadarSensor::RadarSensor(SensorConfig config) : config_(config) {}

SensorMeasurement RadarSensor::measure(const Entity& observer, const Entity& target,
                                     const std::uint64_t tick_index) const {
  SensorMeasurement measurement;
  measurement.target_id = target.id;

  if (observer.id == target.id) {
    return measurement;
  }

  const Vec3 relative = target.position - observer.position;
  const double range = relative.norm();
  if (range > config_.max_range_m || range < 1.0) {
    return measurement;
  }

  if (!has_line_of_sight(observer.position, target.position)) {
    return measurement;
  }

  const Vec3 boresight = boresight_from_observer(observer);
  const Vec3 line_of_sight = relative / range;
  const double cos_angle = boresight.dot(line_of_sight);
  const double cos_fov = std::cos(config_.fov_half_angle_rad);
  if (cos_angle < cos_fov) {
    return measurement;
  }

  const double pos_noise = 25.0;
  measurement.position_m = target.position;
  measurement.position_m.x +=
      deterministic_noise(tick_index, observer.id, target.id, pos_noise);
  measurement.position_m.y +=
      deterministic_noise(tick_index, target.id, observer.id, pos_noise * 0.5);
  measurement.valid = true;
  return measurement;
}

}  // namespace orbit

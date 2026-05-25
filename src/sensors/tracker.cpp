#include "tracker.hpp"

#include <algorithm>
#include <cmath>

namespace orbit {

void Tracker::set_alpha_beta(const double alpha, const double beta) {
  alpha_ = alpha;
  beta_ = beta;
}

double compute_time_to_intercept(const Vec3& interceptor_position,
                                 const Vec3& interceptor_velocity,
                                 const Vec3& target_position,
                                 const Vec3& target_velocity) {
  const Vec3 relative_position = target_position - interceptor_position;
  const Vec3 relative_velocity = target_velocity - interceptor_velocity;
  const double range = relative_position.norm();
  if (range < 1.0) {
    return 0.0;
  }

  const Vec3 range_hat = relative_position / range;
  const double closing_speed = relative_velocity.dot(range_hat);
  if (closing_speed >= 0.0) {
    return -1.0;
  }

  return range / -closing_speed;
}

void Tracker::update(const SensorMeasurement& measurement, const Entity& observer,
                     const Entity& target, const double dt) {
  track_.source_sensor_id = observer.id;
  track_.estimated_entity_id = target.id;

  if (!measurement.valid) {
    return;
  }

  if (!initialized_) {
    track_.estimated_position_m = measurement.position_m;
    initialized_ = true;
  } else {
    const Vec3 residual = measurement.position_m - track_.estimated_position_m;
    track_.estimated_position_m += residual * alpha_;
  }

  if (observer.kind == EntityKind::Interceptor) {
    track_.time_to_intercept_s = compute_time_to_intercept(
        observer.position, observer.velocity, target.position, target.velocity);
  } else {
    track_.time_to_intercept_s = -1.0;
  }
}

}  // namespace orbit

#pragma once

#include <cstdint>
#include <unordered_map>

#include "sensor_model.hpp"
#include "snapshot.hpp"

namespace orbit {

/// Alpha-beta style track filter with intercept geometry.
class Tracker {
 public:
  void set_alpha_beta(double alpha, double beta);

  void update(const SensorMeasurement& measurement, const Entity& observer,
              const Entity& target, double dt);

  [[nodiscard]] const TrackReportSnap& track() const { return track_; }

 private:
  double alpha_{0.35};
  double beta_{0.08};
  bool initialized_{false};
  TrackReportSnap track_{};
};

double compute_time_to_intercept(const Vec3& interceptor_position,
                                 const Vec3& interceptor_velocity,
                                 const Vec3& target_position,
                                 const Vec3& target_velocity);

}  // namespace orbit

#include "sensor_system.hpp"

#include <algorithm>

namespace orbit {

void SensorSystem::reset_for_pool(const EntityPool& pool) {
  sensors_.clear();
  trackers_.clear();
  tracks_.clear();

  for (const Entity& entity : pool.entities()) {
    if (entity.kind == EntityKind::Interceptor || entity.kind == EntityKind::Satellite) {
      SensorConfig config;
      config.sensor_entity_id = entity.id;
      config.max_range_m = 2'500'000.0;
      sensors_.push_back({entity.id, RadarSensor(config)});
    }
  }
}

void SensorSystem::update(const EntityPool& pool, const std::uint64_t tick_index,
                          const double dt) {
  tracks_.clear();

  const auto& entities = pool.entities();
  for (const auto& mount : sensors_) {
    const Entity* observer = nullptr;
    for (const Entity& entity : entities) {
      if (entity.id == mount.observer_id) {
        observer = &entity;
        break;
      }
    }
    if (observer == nullptr) {
      continue;
    }

    for (const Entity& target : entities) {
      if (target.id == observer->id) {
        continue;
      }

      const SensorMeasurement measurement =
          mount.sensor.measure(*observer, target, tick_index);
      if (!measurement.valid) {
        continue;
      }

      Tracker& tracker = trackers_[target.id];
      tracker.update(measurement, *observer, target, dt);

      TrackReportSnap report = tracker.track();
      report.track_id = target.id;
      report.source_sensor_id = observer->id;
      tracks_.push_back(report);
    }
  }

  std::sort(tracks_.begin(), tracks_.end(),
            [](const TrackReportSnap& a, const TrackReportSnap& b) {
              if (a.track_id != b.track_id) {
                return a.track_id < b.track_id;
              }
              return a.source_sensor_id < b.source_sensor_id;
            });
}

}  // namespace orbit

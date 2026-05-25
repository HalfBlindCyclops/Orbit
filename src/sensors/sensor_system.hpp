#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "entity.hpp"
#include "sensor_model.hpp"
#include "tracker.hpp"

namespace orbit {

/// Coordinates radar sensors on entities and maintains track reports.
class SensorSystem {
 public:
  void reset_for_pool(const EntityPool& pool);
  void update(const EntityPool& pool, std::uint64_t tick_index, double dt);

  [[nodiscard]] const std::vector<TrackReportSnap>& tracks() const { return tracks_; }

 private:
  struct SensorMount {
    EntityId observer_id{0};
    RadarSensor sensor;
  };

  std::vector<SensorMount> sensors_;
  std::unordered_map<EntityId, Tracker> trackers_;
  std::vector<TrackReportSnap> tracks_;
};

}  // namespace orbit

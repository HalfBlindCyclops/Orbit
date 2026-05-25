#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "entity.hpp"
#include "snapshot.hpp"

namespace orbit {

enum class Priority : std::uint8_t { Critical = 0, High = 1, Normal = 2, Low = 3 };

enum class PropulsionStage {
  Idle,
  Boost,
  Coast,
  Terminal,
  Depleted,
};

std::string propulsion_stage_name(PropulsionStage stage);

/// Per-entity avionics: power budgeting, fuel flow, propulsion staging.
class EntityFlightComputer {
 public:
  void configure_for_entity(const Entity& entity);
  void begin_tick(std::uint64_t tick_index, double sim_time_s);
  void tick(Entity& entity, double dt, bool apply_actuation);

  [[nodiscard]] const FlightComputerStatusSnap& status() const { return status_; }

 private:
  struct ResourceRequest {
    std::string subsystem;
    Priority priority{Priority::Normal};
    std::uint64_t deadline_tick{0};
    double watts_requested{0.0};
    bool granted{false};
  };

  void update_propulsion_stage(Entity& entity);
  std::vector<ResourceRequest> build_requests(const Entity& entity) const;
  void allocate_power(std::vector<ResourceRequest>& requests);

  EntityId entity_id_{0};
  EntityKind kind_{EntityKind::Satellite};
  PropulsionStage stage_{PropulsionStage::Idle};
  double power_budget_w_{500.0};
  double max_fuel_flow_kg_s_{0.5};
  std::uint64_t tick_index_{0};
  double sim_time_s_{0.0};
  FlightComputerStatusSnap status_;
};

/// Manages flight computers for all controllable entities in the pool.
class FlightComputerRegistry {
 public:
  void set_actuation_enabled(const bool enabled) { actuation_enabled_ = enabled; }

  void sync_pool(const EntityPool& pool);
  void begin_tick(const std::uint64_t tick_index, const double sim_time_s);
  void tick(EntityPool& pool, double dt);

  [[nodiscard]] const std::vector<FlightComputerStatusSnap>& statuses() const {
    return statuses_;
  }

  [[nodiscard]] bool ready() const { return ready_; }

 private:
  bool actuation_enabled_{true};
  bool ready_{false};
  std::unordered_map<EntityId, EntityFlightComputer> computers_;
  std::vector<FlightComputerStatusSnap> statuses_;
};

}  // namespace orbit

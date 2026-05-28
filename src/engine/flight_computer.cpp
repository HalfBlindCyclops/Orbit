#include "flight_computer.hpp"

#include <algorithm>
#include <cmath>

namespace orbit {

namespace {

constexpr double kIspSeconds = 280.0;
constexpr double kG0 = 9.80665;

double desired_thrust_newtons(const EntityKind kind, const PropulsionStage stage) {
  if (kind == EntityKind::Interceptor) {
    switch (stage) {
      case PropulsionStage::Boost:
        return 4000.0;
      case PropulsionStage::Terminal:
        return 8000.0;
      default:
        return 0.0;
    }
  }
  if (stage != PropulsionStage::Depleted) {
    return 50.0;
  }
  return 0.0;
}

}  // namespace

std::string propulsion_stage_name(const PropulsionStage stage) {
  switch (stage) {
    case PropulsionStage::Idle:
      return "Idle";
    case PropulsionStage::Boost:
      return "Boost";
    case PropulsionStage::Coast:
      return "Coast";
    case PropulsionStage::Terminal:
      return "Terminal";
    case PropulsionStage::Depleted:
      return "Depleted";
  }
  return "Unknown";
}

void EntityFlightComputer::configure_for_entity(const Entity& entity) {
  entity_id_ = entity.id;
  kind_ = entity.kind;
  stage_ = PropulsionStage::Idle;
  status_ = {};
  status_.entity_id = entity.id;
  status_.propulsion_stage = propulsion_stage_name(stage_);
}

void EntityFlightComputer::begin_tick(const std::uint64_t tick_index,
                                      const double sim_time_s) {
  tick_index_ = tick_index;
  sim_time_s_ = sim_time_s;
  status_.active_faults.clear();
  status_.power_allocated_w = 0.0;
  status_.thrust_n = 0.0;
}

void EntityFlightComputer::update_propulsion_stage(Entity& entity) {
  if (kind_ != EntityKind::Interceptor) {
    stage_ = entity.fuel_kg > 0.0 ? PropulsionStage::Idle : PropulsionStage::Depleted;
    status_.propulsion_stage = propulsion_stage_name(stage_);
    return;
  }

  if (entity.fuel_kg <= 0.0) {
    stage_ = PropulsionStage::Depleted;
    status_.propulsion_stage = propulsion_stage_name(stage_);
    return;
  }

  switch (stage_) {
    case PropulsionStage::Idle:
      stage_ = PropulsionStage::Boost;
      break;
    case PropulsionStage::Boost:
      if (entity.fuel_kg < 350.0 || tick_index_ > 120) {
        stage_ = PropulsionStage::Coast;
      }
      break;
    case PropulsionStage::Coast:
      if (entity.fuel_kg < 200.0 || sim_time_s_ > 30.0) {
        stage_ = PropulsionStage::Terminal;
      }
      break;
    case PropulsionStage::Terminal:
    case PropulsionStage::Depleted:
      break;
  }

  status_.propulsion_stage = propulsion_stage_name(stage_);
}

std::vector<EntityFlightComputer::ResourceRequest>
EntityFlightComputer::build_requests(const Entity& /*entity*/) const {
  std::vector<ResourceRequest> requests;
  requests.push_back({"guidance", Priority::Normal, tick_index_, 50.0});

  if (kind_ == EntityKind::Interceptor) {
    switch (stage_) {
      case PropulsionStage::Boost:
        requests.push_back({"propulsion", Priority::Critical, tick_index_, 300.0});
        requests.push_back({"sensors", Priority::High, tick_index_, 120.0});
        break;
      case PropulsionStage::Coast:
        requests.push_back({"propulsion", Priority::Low, tick_index_, 80.0});
        requests.push_back({"sensors", Priority::High, tick_index_, 200.0});
        break;
      case PropulsionStage::Terminal:
        requests.push_back({"propulsion", Priority::Critical, tick_index_, 400.0});
        requests.push_back({"sensors", Priority::High, tick_index_, 180.0});
        requests.push_back({"station_keeping", Priority::Low, tick_index_, 100.0});
        break;
      case PropulsionStage::Depleted:
        requests.push_back({"sensors", Priority::High, tick_index_, 150.0});
        break;
      default:
        break;
    }
  } else {
    requests.push_back({"station_keeping", Priority::Low, tick_index_, 100.0});
    requests.push_back({"propulsion", Priority::Low, tick_index_, 60.0});
  }

  return requests;
}

void EntityFlightComputer::allocate_power(std::vector<ResourceRequest>& requests) {
  std::sort(requests.begin(), requests.end(),
            [](const ResourceRequest& a, const ResourceRequest& b) {
              if (static_cast<int>(a.priority) != static_cast<int>(b.priority)) {
                return static_cast<int>(a.priority) < static_cast<int>(b.priority);
              }
              return a.deadline_tick < b.deadline_tick;
            });

  double allocated = 0.0;
  for (ResourceRequest& request : requests) {
    if (allocated + request.watts_requested <= power_budget_w_) {
      request.granted = true;
      allocated += request.watts_requested;
    } else {
      request.granted = false;
      status_.active_faults.push_back("POWER_STARVED:" + request.subsystem);
    }
  }

  status_.power_allocated_w = allocated;
}

void EntityFlightComputer::tick(Entity& entity, const double dt,
                                const bool apply_actuation) {
  update_propulsion_stage(entity);

  auto requests = build_requests(entity);
  allocate_power(requests);

  bool propulsion_granted = false;
  for (const ResourceRequest& request : requests) {
    if (request.subsystem == "propulsion" && request.granted) {
      propulsion_granted = true;
      break;
    }
  }

  double thrust_n = desired_thrust_newtons(kind_, stage_);
  if (thrust_n > 0.0 && !propulsion_granted) {
    thrust_n = 0.0;
    status_.active_faults.push_back("THRUST_INHIBITED");
  }

  if (entity.fuel_kg <= 0.0) {
    thrust_n = 0.0;
    stage_ = PropulsionStage::Depleted;
    status_.propulsion_stage = propulsion_stage_name(stage_);
    if (std::find(status_.active_faults.begin(), status_.active_faults.end(),
                  "FUEL_DEPLETED") == status_.active_faults.end()) {
      status_.active_faults.push_back("FUEL_DEPLETED");
    }
  }

  status_.thrust_n = thrust_n;
  entity.thrust_acceleration = {};

  if (!apply_actuation) {
    return;
  }

  double fuel_flow_kg_s = 0.0;
  if (propulsion_granted && thrust_n > 0.0) {
    fuel_flow_kg_s = thrust_n / (kIspSeconds * kG0);
    fuel_flow_kg_s = std::min(fuel_flow_kg_s, max_fuel_flow_kg_s_);
  }

  const double fuel_burn = fuel_flow_kg_s * dt;
  if (fuel_burn > 0.0) {
    const double burned = std::min(fuel_burn, entity.fuel_kg);
    entity.fuel_kg -= burned;
    entity.mass_kg -= burned;
  }

  if (thrust_n > 0.0 && entity.mass_kg > 0.0) {
    const double speed = entity.velocity.norm();
    const Vec3 direction = speed > 1.0 ? entity.velocity / speed : Vec3{1.0, 0.0, 0.0};
    entity.thrust_acceleration = direction * (thrust_n / entity.mass_kg);
  }
}

void FlightComputerRegistry::sync_pool(const EntityPool& pool) {
  computers_.clear();
  statuses_.clear();
  for (const Entity& entity : pool.entities()) {
    if (entity.kind == EntityKind::Interceptor ||
        (entity.kind == EntityKind::Satellite && entity.fuel_kg > 0.0)) {
      EntityFlightComputer computer;
      computer.configure_for_entity(entity);
      computers_.emplace(entity.id, std::move(computer));
    }
  }
  ready_ = true;
}

void FlightComputerRegistry::begin_tick(const std::uint64_t tick_index,
                                        const double sim_time_s) {
  for (auto& [id, computer] : computers_) {
    (void)id;
    computer.begin_tick(tick_index, sim_time_s);
  }
}

void FlightComputerRegistry::tick(EntityPool& pool, const double dt) {
  statuses_.clear();
  for (Entity& entity : pool.entities_mut()) {
    auto it = computers_.find(entity.id);
    if (it == computers_.end()) {
      continue;
    }
    it->second.tick(entity, dt, actuation_enabled_);
    statuses_.push_back(it->second.status());
  }
  std::sort(statuses_.begin(), statuses_.end(),
            [](const FlightComputerStatusSnap& a, const FlightComputerStatusSnap& b) {
              return a.entity_id < b.entity_id;
            });
}

}  // namespace orbit

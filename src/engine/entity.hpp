#pragma once

#include <string>
#include <vector>

#include "snapshot.hpp"
#include "types.hpp"

namespace orbit {

struct Entity {
  EntityId id{0};
  std::string name;
  EntityKind kind{EntityKind::Satellite};
  Vec3 position{};
  Vec3 velocity{};
  double mass_kg{0.0};
  double fuel_kg{0.0};
  Vec3 thrust_acceleration{};  // m/s^2, set by flight computer each tick
};

class EntityPool {
 public:
  void clear() { entities_.clear(); }
  void add(Entity entity);
  const std::vector<Entity>& entities() const { return entities_; }
  std::vector<Entity>& entities_mut() { return entities_; }

 private:
  std::vector<Entity> entities_;
};

}  // namespace orbit

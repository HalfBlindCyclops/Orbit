#include "entity.hpp"

#include <algorithm>

namespace orbit {

void EntityPool::add(Entity entity) {
  entities_.push_back(std::move(entity));
  std::sort(entities_.begin(), entities_.end(),
            [](const Entity& a, const Entity& b) { return a.id < b.id; });
}

}  // namespace orbit

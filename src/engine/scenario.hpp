#pragma once

#include <string>

#include "entity.hpp"

namespace orbit {

/// Load a named scenario into the entity pool.
/// Tries `scenarios/<name>.json` then built-in presets.
bool load_scenario(const std::string& name, EntityPool& pool);

/// Built-in circular LEO at 400 km with three co-planar objects.
void load_circular_leo_preset(EntityPool& pool);
void load_n_body_sanity_preset(EntityPool& pool);

}  // namespace orbit

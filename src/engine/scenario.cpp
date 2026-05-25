#include "scenario.hpp"

#include <cmath>
#include <fstream>
#include <sstream>

#include "constants.hpp"

namespace orbit {
namespace {

Entity make_leo_entity(EntityId id, const std::string& name, EntityKind kind,
                       double altitude_m, double phase_rad, double mass_kg) {
  const double r = constants::kEarthRadius + altitude_m;
  const double speed = std::sqrt(constants::kEarthMu / r);
  Entity entity;
  entity.id = id;
  entity.name = name;
  entity.kind = kind;
  entity.mass_kg = mass_kg;
  entity.fuel_kg = (kind == EntityKind::Interceptor) ? 500.0 : 0.0;
  entity.position = {r * std::cos(phase_rad), r * std::sin(phase_rad), 0.0};
  entity.velocity = {-speed * std::sin(phase_rad), speed * std::cos(phase_rad), 0.0};
  return entity;
}

constexpr double kPi = 3.14159265358979323846;

}  // namespace

void load_circular_leo_preset(EntityPool& pool) {
  pool.clear();
  pool.add(make_leo_entity(1, "leo_sat_1", EntityKind::Satellite, 400'000.0, 0.0, 800.0));
  pool.add(make_leo_entity(2, "leo_sat_2", EntityKind::Satellite, 420'000.0,
                           2.0 * kPi / 3.0, 750.0));
  pool.add(make_leo_entity(3, "leo_interceptor", EntityKind::Interceptor, 380'000.0,
                           4.0 * kPi / 3.0, 600.0));
}

void load_n_body_sanity_preset(EntityPool& pool) {
  load_circular_leo_preset(pool);
}

bool load_scenario(const std::string& name, EntityPool& pool) {
  if (name == "n_body_sanity") {
    load_n_body_sanity_preset(pool);
    return true;
  }

  if (name == "circular_leo" || name == "leo_three" ||
      name == "coplanar_intercept") {
    load_circular_leo_preset(pool);
    return true;
  }

  if (name == "smoke") {
    pool.clear();
    pool.add(make_leo_entity(1, "smoke_sat", EntityKind::Satellite, 400'000.0, 0.0, 500.0));
    return true;
  }

  const std::string path = "scenarios/" + name + ".json";
  std::ifstream file(path);
  if (!file) {
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  const std::string content = buffer.str();
  if (content.find("\"circular_leo\"") != std::string::npos ||
      content.find("circular_leo") != std::string::npos) {
    load_circular_leo_preset(pool);
    return true;
  }

  return false;
}

}  // namespace orbit

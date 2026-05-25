#include "state_hash.hpp"

#include <cstring>

#include "sha256.hpp"

namespace orbit {
namespace {

void append_f64(std::vector<std::uint8_t>& bytes, double value) {
  static_assert(sizeof(double) == 8, "IEEE-754 double required");
  std::uint8_t raw[8];
  std::memcpy(raw, &value, 8);
  bytes.insert(bytes.end(), raw, raw + 8);
}

}  // namespace

std::string state_sha256_hex(const EntityPool& pool) {
  std::vector<std::uint8_t> bytes;
  bytes.reserve(pool.entities().size() * 80);

  for (const Entity& entity : pool.entities()) {
    const auto id = static_cast<std::uint32_t>(entity.id);
    bytes.push_back(static_cast<std::uint8_t>((id >> 24) & 0xFF));
    bytes.push_back(static_cast<std::uint8_t>((id >> 16) & 0xFF));
    bytes.push_back(static_cast<std::uint8_t>((id >> 8) & 0xFF));
    bytes.push_back(static_cast<std::uint8_t>(id & 0xFF));
    append_f64(bytes, entity.position.x);
    append_f64(bytes, entity.position.y);
    append_f64(bytes, entity.position.z);
    append_f64(bytes, entity.velocity.x);
    append_f64(bytes, entity.velocity.y);
    append_f64(bytes, entity.velocity.z);
    append_f64(bytes, entity.mass_kg);
  }

  return sha256::hex_digest(bytes);
}

}  // namespace orbit

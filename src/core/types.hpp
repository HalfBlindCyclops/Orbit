#pragma once

#include <cstdint>
#include <cmath>

namespace orbit {

using EntityId = std::uint32_t;

struct Vec3 {
  double x{0.0};
  double y{0.0};
  double z{0.0};

  Vec3() = default;
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

  Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vec3 operator-() const { return {-x, -y, -z}; }
  Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
  Vec3 operator/(double s) const { return {x / s, y / s, z / s}; }

  Vec3& operator+=(const Vec3& o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }

  double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
  double norm() const { return std::sqrt(dot(*this)); }
  double norm_squared() const { return dot(*this); }
};

/// Gravitational state for RK4 (position + velocity).
struct OrbitalState {
  Vec3 position;
  Vec3 velocity;
};

struct OrbitalDerivative {
  Vec3 position_dot;
  Vec3 velocity_dot;
};

inline OrbitalState operator+(const OrbitalState& a, const OrbitalState& b) {
  return {a.position + b.position, a.velocity + b.velocity};
}

inline OrbitalState operator*(const OrbitalState& s, double k) {
  return {s.position * k, s.velocity * k};
}

inline OrbitalDerivative operator+(const OrbitalDerivative& a,
                                   const OrbitalDerivative& b) {
  return {a.position_dot + b.position_dot, a.velocity_dot + b.velocity_dot};
}

inline OrbitalDerivative operator*(const OrbitalDerivative& d, double k) {
  return {d.position_dot * k, d.velocity_dot * k};
}

}  // namespace orbit

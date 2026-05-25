#pragma once

namespace orbit::constants {

/// Earth gravitational parameter (m^3/s^2), WGS84-compatible.
inline constexpr double kEarthMu = 3.986004418e14;

/// Earth equatorial radius (m).
inline constexpr double kEarthRadius = 6'378'137.0;

/// Earth J2 coefficient (dimensionless).
inline constexpr double kEarthJ2 = 1.08263e-3;

/// Moon gravitational parameter (m^3/s^2).
inline constexpr double kMoonMu = 4.9048695e12;

/// Mean Earth-Moon distance (m) for simplified lunar ephemeris.
inline constexpr double kMoonOrbitRadius = 384'400'000.0;

/// Moon orbital period around Earth (s).
inline constexpr double kMoonOrbitalPeriod = 27.321661 * 86'400.0;

}  // namespace orbit::constants

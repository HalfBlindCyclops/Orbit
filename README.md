# Orbit

Headless, deterministic 3D orbital mechanics and sensor-tracking simulator (C++20).  
Broadcasts simulation state over UDP using Protocol Buffers; models flight-computer resource priorities under a strict 60 Hz loop.

## Requirements

- CMake 3.24+
- C++20 compiler (MSVC 2022, GCC 12+, or Clang 14+)
- [vcpkg](https://vcpkg.io/) with `VCPKG_ROOT` set
- Ninja (recommended)

## Build

**With vcpkg** (recommended for CI parity):

```bash
git clone <repo-url> Orbit && cd Orbit
cmake --preset default
cmake --build --preset default
ctest --preset default
```

**Without vcpkg** (FetchContent for GTest + Eigen; protobuf/telemetry disabled):

```bash
cmake --preset standalone
cmake --build --preset standalone
ctest --preset standalone
```

### Presets

| Preset | Purpose |
|--------|---------|
| `default` | Debug build + tests |
| `release` | Optimized build + tests + benchmarks |
| `asan` | AddressSanitizer + UBSan (GCC/Clang) |

## Run

```bash
./build/default/orbit_sim --scenario circular_leo --duration 10 --headless

# Live telemetry
./build/default/orbit_sim --scenario circular_leo --duration 5 --udp 127.0.0.1:9000 --headless

# Record / replay session
./build/default/orbit_sim --scenario coplanar_intercept --duration 20 --headless --record demo.orbitreplay
./build/default/orbit_sim --replay demo.orbitreplay

# Advanced physics (J2 + Moon) + sensors
./build/default/orbit_sim --scenario n_body_sanity --duration 30 --headless --advanced-physics
```

## Test

```bash
ctest --test-dir build/default --output-on-failure
```

## Benchmarks

```bash
cmake --preset release
cmake --build --preset release
./build/release/orbit_bench --benchmark_min_time=0.5s
```

Example targets:

- `BM_SimulationFullTick` — full 60 Hz tick (FC + physics + sensors)
- `BM_SimulationAdvancedPhysicsTick` — J2 + lunar gravity path

## Project layout

```
src/core/       FixedTickClock, types, SPSC queue, SHA-256
src/engine/     Physics, flight computer, simulation, replay
src/sensors/    Radar model, tracker, intercept TTI
src/network/    TelemetryHub (UDP + Protobuf)
proto/          telemetry.proto schema
tests/          GoogleTest suites
benchmarks/     Google Benchmark tick harness
docs/           Architecture, telemetry, replay, profiling
```

## Memory safety verification

Orbit treats **zero memory leaks** and **stable heap usage** as release criteria.

### AddressSanitizer (primary — runs in CI on Linux)

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
./build/asan/orbit_sim --scenario smoke --duration 2 --headless --no-sensors
```

**Expected:** clean exit with no `LeakSanitizer` or `ERROR: AddressSanitizer` output.

### Valgrind (Linux — CI `valgrind` job + manual)

```bash
cmake --preset default && cmake --build --preset default
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
  ./build/default/orbit_tests
valgrind --leak-check=full --error-exitcode=1 \
  ./build/default/orbit_sim --scenario smoke --duration 10 --headless --no-sensors
```

**CI:** GitHub Actions job `Valgrind Memcheck` runs both commands on every push to `main`.

**Results (local):** run the commands above and record output in [docs/MEMORY_PROFILING.md](docs/MEMORY_PROFILING.md).

### Design choices for stable heap

- SPSC ring buffer: fixed capacity, no per-tick allocation on the hot path
- Entity pool: pre-sized at scenario load
- Protobuf message encoding on telemetry / replay I/O paths only
- No `std::async`; fixed deterministic worker pool for gravity batching

See [docs/MEMORY_PROFILING.md](docs/MEMORY_PROFILING.md) for methodology.

## Documentation

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — design patterns, threading, data flow, requirements traceability
- [docs/TELEMETRY.md](docs/TELEMETRY.md) — Protobuf wire format
- [docs/REPLAY.md](docs/REPLAY.md) — `.orbitreplay` session format
- [docs/MEMORY_PROFILING.md](docs/MEMORY_PROFILING.md) — ASan / Valgrind methodology

## CI

GitHub Actions on every push and pull request to `main`:

| Job | Platform | Purpose |
|-----|----------|---------|
| `build-test` | Ubuntu, Windows | Build + full `ctest` |
| `determinism` | Ubuntu | Golden 10k-tick SHA256 |
| `sanitizer` | Ubuntu | ASan + UBSan tests |
| `valgrind` | Ubuntu | Memcheck on tests + short sim |

## Development phases

| Phase | Status |
|-------|--------|
| 0 — Tooling & docs | Done |
| 1 — RK4 physics kernel | Done |
| 2 — UDP / Protobuf telemetry | Done |
| 3 — Flight computer priorities | Done |
| 4 — Sensors & tracking | Done |
| 5 — Replay, Valgrind CI, benchmarks | Done |

## License

TBD

# Orbit — System Architecture

## 1. Purpose and scope

Orbit is a **headless** orbital trajectory and sensor-tracking simulator. It integrates equations of motion with fixed timesteps, models avionics-style resource contention on a flight computer, and streams state to external consumers over UDP/Protobuf.

**In scope:** deterministic physics, 60 Hz realtime loop, telemetry export, unit tests, CI, memory-safety verification.

**Out of scope (core binary):** 3D rendering, distributed simulation, operator UI.

## 2. Requirements traceability

| REQ ID | Requirement | Module | Test |
|--------|-------------|--------|------|
| REQ-PHY-01 | RK4 state integration | `orbit_engine` / `physics.cpp` | `physics_tests` |
| REQ-RT-01 | Fixed-rate sim tick (60 Hz default) | `orbit_core` / `clock.cpp` | `physics_tests` |
| REQ-TELEM-01 | UDP Protobuf telemetry stream | `orbit_network` / `telemetry_hub.cpp` | `telemetry_tests` |
| REQ-FC-01 | Resource priority & staging | `flight_computer.cpp` | `flight_computer_tests` |
| REQ-MEM-01 | Zero leaks on tests and short sim | all | CI `sanitizer` job, Valgrind (Phase 5) |
| REQ-DET-01 | Reproducible golden hash | `simulation.cpp` | `Determinism.*` tests |
| REQ-SENS-01 | LOS + FOV sensor model | `orbit_sensors` / `sensor_model.cpp` | `sensor_tests` |
| REQ-SENS-02 | Track + intercept TTI | `tracker.cpp` | `sensor_tests` |
| REQ-PHY-02 | J2 + lunar N-body gravity | `perturbations.cpp`, `gravity_model.cpp` | `sensor_tests` |
| REQ-REPLAY-01 | Record/playback tick stream | `replay.cpp` | `replay_tests` |
| REQ-PERF-01 | Tick latency benchmarks | `benchmarks/bench_tick.cpp` | `orbit_bench` (manual) |

## 3. Design patterns

| Pattern | Usage |
|---------|--------|
| **Facade** | `Simulation` orchestrates clock, engine, FC, telemetry per tick |
| **Strategy** | Gravity models: two-body → N-body + J2 (Phase 4) |
| **State machine** | Propulsion staging: Idle → Boost → Coast → Terminal → Depleted (Phase 3) |
| **Producer–consumer** | Main thread enqueues snapshots; `TelemetryHub` thread sends UDP (Phase 2) |
| **Fixed-priority scheduling** | Flight computer grants power/fuel by `Priority` enum |

## 4. Multi-threading model

| Thread | Owns | Must not |
|--------|------|----------|
| **Main** | Entity state, physics integration, FC decisions | Block on network I/O |
| **Telemetry** | Protobuf encode, UDP send, hub lifecycle | Mutate `Entity` state |
| **Workers** (Phase 4) | N-body acceleration writes to preallocated buffer | Unordered writes to shared state |

**Rules:**

- Single writer to simulation state per tick (main thread).
- `SpscQueue` connects main → telemetry (lock-free, fixed capacity).
- Shutdown: stop flag → join telemetry → join workers → exit.

## 5. Data flow pipelines

### 5.1 Physics pipeline (Phase 1+)

```
Entity states → compute accelerations → RK4 integrate → updated states
```

### 5.2 Simulation tick pipeline (target)

```
FixedTickClock → FC.begin_tick → allocate resources → physics → sensors → snapshot → SPSC enqueue → FC.end_tick → wait_next_tick
```

### 5.3 Telemetry pipeline (Phase 2+)

```
SimulationSnapshot → TelemetryHub → telemetry.proto encode → Asio UDP send
```

## 6. Determinism contract

- Fixed `dt = 1 / rate_hz`; no adaptive timestep in production.
- Entities iterated in ascending `entity_id` order.
- Flight computer logic uses `(t_sim, state)` only — no wall clock, no randomness in sim logic.
- `wall_time_ns` in telemetry is **observability only**, not used for integration.
- Physics translation units compiled with `-fno-fast-math` (or MSVC `/fp:precise`).

## 7. Error handling

- Per-tick path avoids exceptions.
- Resource starvation sets fault strings in `FlightComputerStatus` (Phase 3).
- Telemetry queue overflow increments `dropped_frames` without blocking the sim.

## 8. CMake target map

| Target | Directory | Role |
|--------|-----------|------|
| `orbit_core` | `src/core/` | Types, clock, SPSC queue |
| `orbit_engine` | `src/engine/` | Physics, FC, entities, simulation |
| `orbit_network` | `src/network/` | TelemetryHub |
| `orbit_proto` | `proto/` (optional) | Generated protobuf |
| `orbit_sim_lib` | INTERFACE | Aggregates engine + network |
| `orbit_sim` | `src/main.cpp` | CLI entry |
| `orbit_tests` | `tests/` | GoogleTest binary |

## 9. Replay pipeline (Phase 5)

```
SimulationSnapshot → encode SimulationTick → [uint32 length][protobuf bytes]*
```

- CLI: `--record FILE.orbitreplay`, `--replay FILE.orbitreplay`
- Format documented in [REPLAY.md](REPLAY.md)

## 10. Performance benchmarks (Phase 5)

- Target: `orbit_bench` (Google Benchmark)
- Metrics: full tick latency, advanced-physics tick latency
- Build: `cmake --preset release` with `ORBIT_BUILD_BENCHMARKS=ON`

## 11. Future work

- Bit-identical fixed-point mode (`ORBIT_USE_FIXED_DECIMAL`)
- Replay → live UDP re-broadcast mode
- Drag model with deterministic test profile

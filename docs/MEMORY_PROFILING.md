# Memory profiling methodology

## Goals

1. **Zero definite leaks** on unit tests and a short `orbit_sim` run.
2. **Stable heap** during sustained 60 Hz operation (no unbounded growth after warmup).

## Tools

| Tool | When | Platform |
|------|------|----------|
| AddressSanitizer + UBSan | Every PR (`sanitizer` job) | Linux |
| Valgrind Memcheck | Every PR (`valgrind` job) + manual | Linux |
| Visual Studio Diagnostic Tools | Optional manual | Windows |

## Procedure — AddressSanitizer

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
./build/asan/orbit_sim --scenario smoke --duration 120 --headless --no-sensors
```

Record whether LeakSanitizer reports **0 bytes definitely lost**.

## Procedure — Valgrind

```bash
cmake --preset default && cmake --build --preset default

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
  ./build/default/orbit_tests 2>&1 | tee valgrind-tests.log

valgrind --leak-check=full --error-exitcode=1 \
  ./build/default/orbit_sim --scenario smoke --duration 60 --headless --no-sensors \
  2>&1 | tee valgrind-sim.log
```

### CI integration

Workflow job **Valgrind Memcheck (Linux)** in [`.github/workflows/ci.yml`](../.github/workflows/ci.yml) runs:

1. Full `orbit_tests` suite under Valgrind
2. `orbit_sim` for 5 seconds (smoke scenario, headless, sensors off)

Both steps use `--error-exitcode=1` so leaks fail the build.

## Hot-path allocation policy

| Component | Policy |
|-----------|--------|
| `SpscQueue` | Fixed `std::array`, no heap |
| `EntityPool` | `vector` reserved at load |
| `Simulation::publish_telemetry` | Skips snapshot build if no UDP and no recorder |
| Telemetry thread | Encodes protobuf per dequeued frame |
| Replay recorder | File I/O per tick when `--record` enabled |
| Gravity worker pool | Threads created once per batch call site (static pool) |

## Results log

| Date | Target | Definitely lost | Errors | Notes |
|------|--------|-----------------|--------|-------|
| _CI_ | `orbit_tests` | 0 (expected) | 0 | `valgrind` job |
| _CI_ | `orbit_sim` 5s | 0 (expected) | 0 | `valgrind` job |
| _local_ | fill after run | — | — | |

# Telemetry wire format

**Status:** UDP streaming active when running `orbit_sim --udp HOST:PORT`.

## Schema

Source: [`proto/telemetry.proto`](../proto/telemetry.proto)  
Package: `orbit.telemetry`

Primary message: **`SimulationTick`** — one logical frame of simulation output.

| Field | Type | Description |
|-------|------|-------------|
| `schema_version` | `uint32` | Wire format version (start at `1`) |
| `tick_index` | `uint64` | Monotonic sim tick counter |
| `sim_time_s` | `double` | Simulation time in seconds |
| `wall_time_ns` | `uint64` | Host clock (not used for physics) |
| `dropped_frames` | `uint64` | Telemetry queue overflow count |
| `entities` | `repeated EntityState` | Positions, velocities, mass, fuel |
| `flight_computers` | `repeated FlightComputerStatus` | Power, thrust, stage, faults |
| `tracks` | `repeated TrackReport` | Sensor tracks (Phase 4) |

## Transport

- Protocol: **UDP** (default `127.0.0.1:9000`, set via `--udp HOST:PORT`)
- Encoding: **Protobuf** binary (`SimulationTick`)
- Thread: `TelemetryHub` worker thread reads `SpscQueue` (capacity 8)
- Overflow: non-blocking enqueue; `dropped_frames` counter in next packet

### Run

```bash
# Terminal 1
py -3 tools/telemetry_receiver.py --host 127.0.0.1 --port 9000

# Terminal 2 (generate Python stubs first on first use)
tools\generate_python_proto.bat
build\default\orbit_sim --scenario circular_leo --duration 5 --udp 127.0.0.1:9000 --headless
```

## Chunking

If datagram size exceeds MTU, `SimulationTick` will be split into chunk messages (Phase 2 design).

## Consumer

`tools/telemetry_receiver.py` — planned in Phase 2 for CSV logging and parse-rate validation.

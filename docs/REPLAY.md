# Session replay format

## File extension

`.orbitreplay` — length-prefixed stream of `SimulationTick` protobuf messages.

## Header

| Field | Size | Value |
|-------|------|-------|
| Magic | 8 bytes | `ORBRPLY\0` |
| Version | uint32 LE | `1` |

## Frames

Each frame:

| Field | Size |
|-------|------|
| Length | uint32 LE |
| Payload | `SimulationTick` protobuf bytes |

## Usage

Record during simulation:

```bash
orbit_sim --scenario circular_leo --duration 10 --headless \
  --record runs/demo.orbitreplay
```

Playback summary (no physics):

```bash
orbit_sim --replay runs/demo.orbitreplay
```

Requires `ORBIT_BUILD_PROTO=ON` (default with vcpkg).

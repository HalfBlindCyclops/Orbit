#!/usr/bin/env python3
"""Listen for Orbit SimulationTick UDP protobuf datagrams and log CSV."""

from __future__ import annotations

import argparse
import socket
import sys
from pathlib import Path

# Generated in CI/build: protoc --python_out=tools proto/telemetry.proto
try:
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    import telemetry_pb2  # type: ignore
except ImportError:
    telemetry_pb2 = None


def main() -> int:
    parser = argparse.ArgumentParser(description="Orbit telemetry UDP receiver")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--max", type=int, default=0, help="Stop after N packets (0=forever)")
    args = parser.parse_args()

    if telemetry_pb2 is None:
        print(
            "telemetry_pb2 not found. Generate with:\n"
            "  protoc --python_out=tools -Iproto proto/telemetry.proto",
            file=sys.stderr,
        )
        return 1

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.host, args.port))
    print(f"listening udp://{args.host}:{args.port}")

    count = 0
    while args.max == 0 or count < args.max:
        data, addr = sock.recvfrom(65535)
        tick = telemetry_pb2.SimulationTick()
        tick.ParseFromString(data)
        count += 1
        entity_count = len(tick.entities)
        print(
            f"[{count}] from {addr[0]}:{addr[1]} tick={tick.tick_index} "
            f"t={tick.sim_time_s:.3f}s entities={entity_count} "
            f"dropped={tick.dropped_frames}"
        )
        if entity_count > 0:
            e = tick.entities[0]
            print(
                f"    e0 id={e.entity_id} name={e.name} "
                f"pos=({e.position_m.x:.1f},{e.position_m.y:.1f},{e.position_m.z:.1f})"
            )
        for tr in tick.tracks[:3]:
            print(
                f"    track id={tr.track_id} sensor={tr.source_sensor_id} "
                f"tti={tr.time_to_intercept_s:.1f}s"
            )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

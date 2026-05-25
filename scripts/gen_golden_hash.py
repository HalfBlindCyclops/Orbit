#!/usr/bin/env python3
"""Reference integrator to generate tests/data/golden_10k_tick.sha256."""

import hashlib
import math
import struct
from pathlib import Path

MU = 3.986004418e14
R_EARTH = 6_378_137.0


def accel(pos):
    x, y, z = pos
    r2 = x * x + y * y + z * z
    r3 = r2 * math.sqrt(r2)
    return (-MU * x / r3, -MU * y / r3, -MU * z / r3)


def rk4_step(pos, vel, dt):
    def deriv(p, v):
        ax, ay, az = accel(p)
        return v, (ax, ay, az)

    k1_p, k1_v = deriv(pos, vel)
    k2_p, k2_v = deriv(
        tuple(p + 0.5 * dt * k for p, k in zip(pos, k1_p)),
        tuple(v + 0.5 * dt * k for v, k in zip(vel, k1_v)),
    )
    k3_p, k3_v = deriv(
        tuple(p + 0.5 * dt * k for p, k in zip(pos, k2_p)),
        tuple(v + 0.5 * dt * k for v, k in zip(vel, k2_v)),
    )
    k4_p, k4_v = deriv(
        tuple(p + dt * k for p, k in zip(pos, k3_p)),
        tuple(v + dt * k for v, k in zip(vel, k3_v)),
    )

    new_pos = tuple(
        p + dt / 6.0 * (k1 + 2 * k2 + 2 * k3 + k4)
        for p, k1, k2, k3, k4 in zip(pos, k1_p, k2_p, k3_p, k4_p)
    )
    new_vel = tuple(
        v + dt / 6.0 * (k1 + 2 * k2 + 2 * k3 + k4)
        for v, k1, k2, k3, k4 in zip(vel, k1_v, k2_v, k3_v, k4_v)
    )
    return new_pos, new_vel


def make_entity(eid, altitude, phase, mass):
    r = R_EARTH + altitude
    speed = math.sqrt(MU / r)
    pos = (r * math.cos(phase), r * math.sin(phase), 0.0)
    vel = (-speed * math.sin(phase), speed * math.cos(phase), 0.0)
    return eid, pos, vel, mass


def state_hash(entities):
    blob = bytearray()
    for eid, pos, vel, mass in sorted(entities, key=lambda e: e[0]):
        blob += struct.pack(">I", eid)
        for value in (*pos, *vel, mass):
            blob += struct.pack("<d", value)
    return hashlib.sha256(blob).hexdigest()


def main():
    entities = [
        make_entity(1, 400_000.0, 0.0, 800.0),
        make_entity(2, 420_000.0, 2.0 * math.pi / 3.0, 750.0),
        make_entity(3, 380_000.0, 4.0 * math.pi / 3.0, 600.0),
    ]
    dt = 1.0 / 60.0
    for _ in range(10_000):
        updated = []
        for eid, pos, vel, mass in entities:
            new_pos, new_vel = rk4_step(pos, vel, dt)
            updated.append((eid, new_pos, new_vel, mass))
        entities = updated

    digest = state_hash(entities)
    out = Path(__file__).resolve().parent.parent / "tests" / "data" / "golden_10k_tick.sha256"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(digest + "\n", encoding="utf-8")
    print(digest)


if __name__ == "__main__":
    main()

#pragma once

#include <string>

#include "snapshot.hpp"  // orbit::SimulationSnapshot in core/

namespace orbit {

/// Serialize a snapshot to protobuf wire bytes.
std::string encode_simulation_tick(const SimulationSnapshot& snapshot,
                                   std::uint64_t dropped_frames = 0);

/// Parse wire bytes back into a snapshot (for tests).
bool decode_simulation_tick(const std::string& wire, SimulationSnapshot& out,
                            std::uint32_t& schema_version,
                            std::uint64_t& dropped_frames);

}  // namespace orbit

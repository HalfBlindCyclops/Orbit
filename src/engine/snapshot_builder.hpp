#pragma once

#include <vector>

#include "entity.hpp"
#include "snapshot.hpp"

namespace orbit {

SimulationSnapshot make_snapshot(
    const EntityPool& pool, std::uint64_t tick_index, double sim_time_s,
    const std::vector<FlightComputerStatusSnap>& flight_computers = {},
    const std::vector<TrackReportSnap>& tracks = {});

}  // namespace orbit

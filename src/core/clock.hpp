#pragma once

#include <chrono>
#include <cstdint>

namespace orbit {

/// Fixed-rate wall-clock scheduler for the 60 Hz simulation loop.
class FixedTickClock {
 public:
  static constexpr std::int64_t kTickPeriodNs = 16'666'666;  // 1/60 s

  explicit FixedTickClock(std::uint32_t rate_hz = 60);

  std::uint64_t tick_index() const { return tick_index_; }
  double sim_time_s() const { return sim_time_s_; }

  /// Advance one simulation tick and sleep until the next boundary.
  void wait_next_tick();

 private:
  std::uint32_t rate_hz_;
  std::chrono::nanoseconds period_;
  std::uint64_t tick_index_{0};
  double sim_time_s_{0.0};
  std::chrono::steady_clock::time_point next_tick_;
};

}  // namespace orbit

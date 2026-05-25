#include "clock.hpp"

#include <thread>

namespace orbit {

FixedTickClock::FixedTickClock(std::uint32_t rate_hz)
    : rate_hz_(rate_hz),
      period_(std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::duration<double>(1.0 / static_cast<double>(rate_hz_)))),
      next_tick_(std::chrono::steady_clock::now()) {}

void FixedTickClock::wait_next_tick() {
  ++tick_index_;
  sim_time_s_ += 1.0 / static_cast<double>(rate_hz_);
  next_tick_ += period_;

  const auto now = std::chrono::steady_clock::now();
  if (next_tick_ > now) {
    std::this_thread::sleep_until(next_tick_);
  } else {
    next_tick_ = now;
  }
}

}  // namespace orbit

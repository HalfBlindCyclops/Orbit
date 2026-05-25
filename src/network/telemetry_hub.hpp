#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include "snapshot.hpp"
#include "spsc_queue.hpp"

namespace orbit {

/// Protobuf encode + UDP broadcast on a dedicated thread.
class TelemetryHub {
 public:
  static constexpr std::size_t kQueueCapacity = 8;

  explicit TelemetryHub(std::string destination_endpoint = "127.0.0.1:9000");

  ~TelemetryHub();

  TelemetryHub(const TelemetryHub&) = delete;
  TelemetryHub& operator=(const TelemetryHub&) = delete;

  void start();
  void stop();

  /// Non-blocking enqueue from the simulation thread.
  bool try_enqueue(SimulationSnapshot snapshot);

  [[nodiscard]] bool running() const {
    return running_.load(std::memory_order_acquire);
  }

  [[nodiscard]] std::uint64_t dropped_frames() const {
    return dropped_frames_.load(std::memory_order_relaxed);
  }

  [[nodiscard]] std::uint64_t sent_frames() const {
    return sent_frames_.load(std::memory_order_relaxed);
  }

 private:
  void thread_main();

  std::string endpoint_;
  SpscQueue<SimulationSnapshot, kQueueCapacity> queue_;
  std::thread worker_;
  std::atomic<bool> running_{false};
  std::atomic<bool> stop_requested_{false};
  std::atomic<std::uint64_t> dropped_frames_{0};
  std::atomic<std::uint64_t> sent_frames_{0};
};

}  // namespace orbit

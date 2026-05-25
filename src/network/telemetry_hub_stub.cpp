#include "telemetry_hub.hpp"

namespace orbit {

TelemetryHub::TelemetryHub(std::string destination_endpoint)
    : endpoint_(std::move(destination_endpoint)) {}

TelemetryHub::~TelemetryHub() { stop(); }

void TelemetryHub::start() {
  running_.store(true, std::memory_order_release);
}

void TelemetryHub::stop() {
  running_.store(false, std::memory_order_release);
}

bool TelemetryHub::try_enqueue(SimulationSnapshot /*snapshot*/) {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  sent_frames_.fetch_add(1, std::memory_order_relaxed);
  return true;
}

void TelemetryHub::thread_main() {}

}  // namespace orbit

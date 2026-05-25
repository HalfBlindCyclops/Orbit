#include "telemetry_hub.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

#include <asio.hpp>

#include "telemetry_encoder.hpp"

namespace orbit {
namespace {

struct Endpoint {
  std::string host;
  unsigned short port{9000};
};

Endpoint parse_endpoint(const std::string& text) {
  const auto colon = text.find(':');
  if (colon == std::string::npos) {
    throw std::invalid_argument("endpoint must be host:port");
  }
  Endpoint endpoint;
  endpoint.host = text.substr(0, colon);
  endpoint.port = static_cast<unsigned short>(std::stoul(text.substr(colon + 1)));
  return endpoint;
}

}  // namespace

TelemetryHub::TelemetryHub(std::string destination_endpoint)
    : endpoint_(std::move(destination_endpoint)) {}

TelemetryHub::~TelemetryHub() { stop(); }

void TelemetryHub::start() {
  if (running_.exchange(true, std::memory_order_acq_rel)) {
    return;
  }
  stop_requested_.store(false, std::memory_order_release);
  worker_ = std::thread(&TelemetryHub::thread_main, this);
}

void TelemetryHub::stop() {
  if (!running_.load(std::memory_order_acquire)) {
    return;
  }
  stop_requested_.store(true, std::memory_order_release);
  if (worker_.joinable()) {
    worker_.join();
  }
  running_.store(false, std::memory_order_release);
}

bool TelemetryHub::try_enqueue(SimulationSnapshot snapshot) {
  if (!running_.load(std::memory_order_acquire)) {
    return false;
  }
  if (queue_.try_push(std::move(snapshot))) {
    return true;
  }
  dropped_frames_.fetch_add(1, std::memory_order_relaxed);
  return false;
}

void TelemetryHub::thread_main() {
  try {
    const Endpoint endpoint = parse_endpoint(endpoint_);
    asio::io_context io;
    asio::ip::udp::socket socket(io);
    socket.open(asio::ip::udp::v4());
    const auto destination = asio::ip::udp::endpoint(
        asio::ip::make_address(endpoint.host), endpoint.port);

    while (!stop_requested_.load(std::memory_order_acquire)) {
      if (auto snapshot = queue_.try_pop()) {
        const auto dropped = dropped_frames_.load(std::memory_order_relaxed);
        const std::string wire =
            encode_simulation_tick(*snapshot, dropped);
        socket.send_to(asio::buffer(wire), destination);
        sent_frames_.fetch_add(1, std::memory_order_relaxed);
        continue;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    while (auto snapshot = queue_.try_pop()) {
      const auto dropped = dropped_frames_.load(std::memory_order_relaxed);
      const std::string wire = encode_simulation_tick(*snapshot, dropped);
      socket.send_to(asio::buffer(wire), destination);
      sent_frames_.fetch_add(1, std::memory_order_relaxed);
    }
  } catch (...) {
    stop_requested_.store(true, std::memory_order_release);
    running_.store(false, std::memory_order_release);
  }
}

}  // namespace orbit

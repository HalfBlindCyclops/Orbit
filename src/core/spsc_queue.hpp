#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <optional>
#include <utility>

namespace orbit {

/// Single-producer single-consumer ring buffer (Phase 2 telemetry path).
template <typename T, std::size_t Capacity>
class SpscQueue {
 public:
  static_assert(Capacity > 0, "Capacity must be positive");

  bool try_push(const T& item) { return try_push_impl(item); }

  bool try_push(T&& item) { return try_push_impl(std::move(item)); }

 private:
  template <typename U>
  bool try_push_impl(U&& item) {
    const auto head = head_.load(std::memory_order_relaxed);
    const auto next = (head + 1) % Capacity;
    if (next == tail_.load(std::memory_order_acquire)) {
      return false;
    }
    buffer_[head] = std::forward<U>(item);
    head_.store(next, std::memory_order_release);
    return true;
  }

 public:

  std::optional<T> try_pop() {
    const auto tail = tail_.load(std::memory_order_relaxed);
    if (tail == head_.load(std::memory_order_acquire)) {
      return std::nullopt;
    }
    T item = buffer_[tail];
    tail_.store((tail + 1) % Capacity, std::memory_order_release);
    return item;
  }

  std::size_t capacity() const { return Capacity; }

 private:
  std::array<T, Capacity> buffer_{};
  std::atomic<std::size_t> head_{0};
  std::atomic<std::size_t> tail_{0};
};

}  // namespace orbit

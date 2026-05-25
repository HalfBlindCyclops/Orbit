#include "gravity_worker_pool.hpp"

#include <algorithm>
#include <thread>
#include <vector>

namespace orbit {

DeterministicWorkerPool::DeterministicWorkerPool(const std::size_t thread_count) {
  if (thread_count == 0) {
    const auto hw = std::thread::hardware_concurrency();
    thread_count_ = hw > 2 ? hw - 2 : 1;
  } else {
    thread_count_ = thread_count;
  }
  thread_count_ = std::max<std::size_t>(1, thread_count_);
}

void DeterministicWorkerPool::parallel_for(
    const std::size_t count, const std::function<void(std::size_t)>& work) const {
  if (count == 0) {
    return;
  }
  if (thread_count_ == 1 || count < thread_count_) {
    for (std::size_t i = 0; i < count; ++i) {
      work(i);
    }
    return;
  }

  std::vector<std::thread> threads;
  threads.reserve(thread_count_);

  const std::size_t chunk = (count + thread_count_ - 1) / thread_count_;
  for (std::size_t t = 0; t < thread_count_; ++t) {
    const std::size_t begin = t * chunk;
    const std::size_t end = std::min(begin + chunk, count);
    if (begin >= end) {
      break;
    }
    threads.emplace_back([&work, begin, end]() {
      for (std::size_t i = begin; i < end; ++i) {
        work(i);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

}  // namespace orbit

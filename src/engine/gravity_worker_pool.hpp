#pragma once

#include <cstddef>
#include <functional>

namespace orbit {

/// Fixed-size thread pool with deterministic ordered index ranges.
class DeterministicWorkerPool {
 public:
  explicit DeterministicWorkerPool(std::size_t thread_count = 0);

  void parallel_for(std::size_t count,
                    const std::function<void(std::size_t)>& work) const;

 private:
  std::size_t thread_count_{1};
};

}  // namespace orbit

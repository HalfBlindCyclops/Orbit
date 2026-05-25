#pragma once

#include <cstdint>
#include <fstream>
#include <string>

#include "snapshot.hpp"

namespace orbit {

/// Length-prefixed protobuf tick stream (.orbitreplay).
class ReplayRecorder {
 public:
  bool open(const std::string& path);
  void close();

  bool write_tick(const SimulationSnapshot& snapshot, std::uint64_t dropped_frames = 0);

  [[nodiscard]] std::uint64_t frames_written() const { return frames_written_; }
  [[nodiscard]] bool is_open() const { return stream_.is_open(); }

 private:
  std::ofstream stream_;
  std::uint64_t frames_written_{0};
};

class ReplayPlayer {
 public:
  bool open(const std::string& path);
  void close();

  /// Reads the next tick; returns false at EOF.
  bool next_tick(SimulationSnapshot& snapshot, std::uint32_t& schema_version,
                 std::uint64_t& dropped_frames);

  [[nodiscard]] std::uint64_t frames_read() const { return frames_read_; }

  /// Print human-readable summary to stdout.
  static int print_summary(const std::string& path);

 private:
  std::ifstream stream_;
  std::uint64_t frames_read_{0};
};

}  // namespace orbit

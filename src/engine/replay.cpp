#include "replay.hpp"

#include <cstring>
#include <iostream>

#ifdef ORBIT_HAS_PROTO
#include "telemetry_encoder.hpp"
#endif

namespace orbit {
namespace {

constexpr char kReplayMagic[8] = {'O', 'R', 'B', 'R', 'P', 'L', 'Y', '\0'};
constexpr std::uint32_t kReplayVersion = 1;

bool write_u32(std::ofstream& stream, const std::uint32_t value) {
  const unsigned char bytes[4] = {
      static_cast<unsigned char>(value & 0xFF),
      static_cast<unsigned char>((value >> 8) & 0xFF),
      static_cast<unsigned char>((value >> 16) & 0xFF),
      static_cast<unsigned char>((value >> 24) & 0xFF),
  };
  stream.write(reinterpret_cast<const char*>(bytes), 4);
  return static_cast<bool>(stream);
}

bool read_u32(std::ifstream& stream, std::uint32_t& value) {
  unsigned char bytes[4];
  stream.read(reinterpret_cast<char*>(bytes), 4);
  if (stream.gcount() != 4) {
    return false;
  }
  value = static_cast<std::uint32_t>(bytes[0]) |
          (static_cast<std::uint32_t>(bytes[1]) << 8) |
          (static_cast<std::uint32_t>(bytes[2]) << 16) |
          (static_cast<std::uint32_t>(bytes[3]) << 24);
  return true;
}

}  // namespace

bool ReplayRecorder::open(const std::string& path) {
  close();
  stream_.open(path, std::ios::binary | std::ios::trunc);
  if (!stream_) {
    return false;
  }
  stream_.write(kReplayMagic, 8);
  return write_u32(stream_, kReplayVersion);
}

void ReplayRecorder::close() {
  if (stream_.is_open()) {
    stream_.close();
  }
}

bool ReplayRecorder::write_tick(const SimulationSnapshot& snapshot,
                                const std::uint64_t dropped_frames) {
#ifdef ORBIT_HAS_PROTO
  if (!stream_.is_open()) {
    return false;
  }
  const std::string wire = encode_simulation_tick(snapshot, dropped_frames);
  if (!write_u32(stream_, static_cast<std::uint32_t>(wire.size()))) {
    return false;
  }
  stream_.write(wire.data(), static_cast<std::streamsize>(wire.size()));
  ++frames_written_;
  return static_cast<bool>(stream_);
#else
  (void)snapshot;
  (void)dropped_frames;
  return false;
#endif
}

bool ReplayPlayer::open(const std::string& path) {
  close();
  stream_.open(path, std::ios::binary);
  if (!stream_) {
    return false;
  }

  char magic[8];
  stream_.read(magic, 8);
  if (std::strncmp(magic, kReplayMagic, 8) != 0) {
    close();
    return false;
  }

  std::uint32_t version = 0;
  if (!read_u32(stream_, version) || version != kReplayVersion) {
    close();
    return false;
  }

  frames_read_ = 0;
  return true;
}

void ReplayPlayer::close() {
  if (stream_.is_open()) {
    stream_.close();
  }
}

bool ReplayPlayer::next_tick(SimulationSnapshot& snapshot, std::uint32_t& schema_version,
                             std::uint64_t& dropped_frames) {
#ifdef ORBIT_HAS_PROTO
  if (!stream_.is_open()) {
    return false;
  }

  std::uint32_t length = 0;
  if (!read_u32(stream_, length)) {
    return false;
  }

  std::string wire(length, '\0');
  stream_.read(wire.data(), static_cast<std::streamsize>(length));
  if (static_cast<std::uint32_t>(stream_.gcount()) != length) {
    return false;
  }

  if (!decode_simulation_tick(wire, snapshot, schema_version, dropped_frames)) {
    return false;
  }

  ++frames_read_;
  return true;
#else
  (void)snapshot;
  (void)schema_version;
  (void)dropped_frames;
  return false;
#endif
}

int ReplayPlayer::print_summary(const std::string& path) {
  ReplayPlayer player;
  if (!player.open(path)) {
    std::cerr << "Failed to open replay: " << path << '\n';
    return 1;
  }

  SimulationSnapshot snapshot;
  std::uint32_t schema = 0;
  std::uint64_t dropped = 0;
  std::uint64_t count = 0;
  double last_time = 0.0;

  while (player.next_tick(snapshot, schema, dropped)) {
    ++count;
    last_time = snapshot.sim_time_s;
    if (count == 1 || count % 60 == 0) {
      std::cout << "tick=" << snapshot.tick_index << " t=" << snapshot.sim_time_s
                << "s entities=" << snapshot.entities.size()
                << " tracks=" << snapshot.tracks.size() << " fc=" << snapshot.flight_computers.size()
                << '\n';
    }
  }

  std::cout << "Replay summary: " << path << '\n';
  std::cout << "  frames=" << count << " duration=" << last_time << "s schema=" << schema << '\n';
  return count == 0 ? 1 : 0;
}

}  // namespace orbit

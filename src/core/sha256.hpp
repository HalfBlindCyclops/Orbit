#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace orbit::sha256 {

std::string hex_digest(const std::vector<std::uint8_t>& data);

}  // namespace orbit::sha256

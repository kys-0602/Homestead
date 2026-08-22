#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace Homestead {
[[nodiscard]] bool DecodeAdpcm2(const std::uint8_t* data, std::size_t size,
                                std::vector<std::int16_t>& samples) noexcept;
}

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Homestead::AssetPacker {

struct MapStats {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::size_t byteCount = 0;
};

[[nodiscard]] bool BuildMapPayload(
    const std::filesystem::path& path,
    std::vector<std::uint8_t>& bytes,
    MapStats& stats,
    std::string& error);

} // namespace Homestead::AssetPacker

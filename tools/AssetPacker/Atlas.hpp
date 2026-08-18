#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace Homestead::AssetPacker {

struct AtlasStats {
    std::size_t logicalSpriteCount = 0;
    std::size_t uniqueSpriteCount = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::size_t paletteColorCount = 0;
    std::uintmax_t pixelBytes = 0;
    std::uintmax_t paletteBytes = 0;
};

[[nodiscard]] bool BuildAtlas(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& outputDirectory,
    AtlasStats& stats,
    std::string& error);

} // namespace Homestead::AssetPacker

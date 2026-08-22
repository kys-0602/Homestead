#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

namespace Homestead::AssetPacker {

struct PakStats {
    std::size_t entryCount = 0;
    std::size_t spriteCount = 0;
    std::size_t mapBytes = 0;
    std::size_t audioBytes = 0;
    std::uintmax_t byteCount = 0;
};

[[nodiscard]] bool BuildPak(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& atlasDirectory,
    const std::filesystem::path& outputPath,
    PakStats& stats,
    std::string& error);

} // namespace Homestead::AssetPacker

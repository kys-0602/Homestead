#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

namespace Homestead::AssetPacker {

struct ManifestStats {
    std::size_t sourceCount = 0;
    std::size_t spriteCount = 0;
    std::size_t playerFrameCount = 0;
};

[[nodiscard]] bool ValidateManifests(
    const std::filesystem::path& assetRoot,
    ManifestStats& stats,
    std::string& error);

} // namespace Homestead::AssetPacker

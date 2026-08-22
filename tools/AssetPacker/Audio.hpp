#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Homestead::AssetPacker {

[[nodiscard]] bool BuildAdpcm2(
    const std::filesystem::path& inputPath,
    std::vector<std::uint8_t>& output,
    std::string& error);

} // namespace Homestead::AssetPacker

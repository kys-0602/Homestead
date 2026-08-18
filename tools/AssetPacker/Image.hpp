#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Homestead::AssetPacker {

struct Image {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> pixels;
};

[[nodiscard]] bool InitializeImageDecoder(std::string& error);
void ShutdownImageDecoder() noexcept;
[[nodiscard]] bool LoadPng(
    const std::filesystem::path& path,
    Image& image,
    std::string& error);

} // namespace Homestead::AssetPacker

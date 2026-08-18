#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Homestead {

using AssetId = std::uint32_t;

struct SpriteAsset {
    AssetId id = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint16_t trimX = 0;
    std::uint16_t trimY = 0;
    std::uint16_t sourceWidth = 0;
    std::uint16_t sourceHeight = 0;
};

class AssetStore final {
public:
    [[nodiscard]] bool LoadFile(const wchar_t* path) noexcept;
    [[nodiscard]] bool LoadMemory(const std::uint8_t* data, std::size_t size) noexcept;
    void Clear() noexcept;

    [[nodiscard]] const SpriteAsset* FindSprite(AssetId id) const noexcept;
    [[nodiscard]] const std::uint8_t* AtlasPixels() const noexcept { return atlasPixels_.data(); }
    [[nodiscard]] std::uint16_t AtlasWidth() const noexcept { return atlasWidth_; }
    [[nodiscard]] std::uint16_t AtlasHeight() const noexcept { return atlasHeight_; }
    [[nodiscard]] std::size_t SpriteCount() const noexcept { return sprites_.size(); }

private:
    std::vector<std::uint8_t> atlasPixels_;
    std::vector<SpriteAsset> sprites_;
    std::uint16_t atlasWidth_ = 0;
    std::uint16_t atlasHeight_ = 0;
};

[[nodiscard]] constexpr AssetId MakeAssetId(const char* text) noexcept {
    AssetId value = 2166136261U;
    while (*text != '\0') {
        value ^= static_cast<std::uint8_t>(*text++);
        value *= 16777619U;
    }
    return value;
}

} // namespace Homestead

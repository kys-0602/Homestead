#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;

[[nodiscard]] bool AddBitmapText(
    const char* text,
    float x,
    float y,
    std::uint32_t color,
    std::uint16_t depth,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

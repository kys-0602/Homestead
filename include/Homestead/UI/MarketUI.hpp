#pragma once

#include <cstdint>

namespace Homestead {
class AssetStore; class RenderQueue;
inline constexpr std::uint8_t MarketItemCount = 6;
[[nodiscard]] int MarketItemAt(std::uint32_t x, std::uint32_t y) noexcept;
[[nodiscard]] std::uint8_t UpdateMarketFocus(std::uint8_t focus, bool up, bool down,
    bool mouseValid, std::uint32_t mouseX, std::uint32_t mouseY) noexcept;
[[nodiscard]] bool AddMarketUI(std::uint16_t gold, std::uint8_t focus,
    const AssetStore& assets, RenderQueue& queue) noexcept;
} // namespace Homestead

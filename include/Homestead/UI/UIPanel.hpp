#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;

[[nodiscard]] bool AddUIPanel(float x, float y, float width, float height,
                             std::uint32_t color, std::uint16_t depth,
                             const AssetStore& assets, RenderQueue& queue) noexcept;

[[nodiscard]] bool AddUIFill(float x, float y, float width, float height,
                            std::uint32_t color, std::uint16_t depth,
                            const AssetStore& assets, RenderQueue& queue) noexcept;

} // namespace Homestead

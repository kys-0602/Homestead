#pragma once

#include <cstddef>
#include <cstdint>

namespace Homestead {

class AssetStore;
class Camera2D;
class RenderQueue;
class TileMap;

struct TileMapRenderStats {
    std::size_t visitedTiles = 0;
    std::size_t submittedSprites = 0;
};

struct TileBounds {
    std::int32_t firstX = 0;
    std::int32_t firstY = 0;
    std::int32_t lastX = -1;
    std::int32_t lastY = -1;
};

[[nodiscard]] std::uint8_t FarmlandConnectionMask(
    const TileMap& map,
    std::int32_t x,
    std::int32_t y) noexcept;

[[nodiscard]] TileBounds CalculateVisibleTileBounds(
    const Camera2D& camera,
    std::uint16_t mapWidth,
    std::uint16_t mapHeight) noexcept;

[[nodiscard]] TileBounds CalculateVisibleObjectBounds(
    const Camera2D& camera,
    std::uint16_t mapWidth,
    std::uint16_t mapHeight) noexcept;

class TileMapRenderer final {
public:
    [[nodiscard]] static bool Build(
        const TileMap& map,
        const Camera2D& camera,
        const AssetStore& assets,
        RenderQueue& queue,
        TileMapRenderStats* stats = nullptr) noexcept;
};

} // namespace Homestead

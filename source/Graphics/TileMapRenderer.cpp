#include "Homestead/Graphics/TileMapRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

AssetId GraphicAssetId(std::uint16_t graphic) noexcept {
    switch (static_cast<TileGraphic>(graphic)) {
    case TileGraphic::Grass: return MakeAssetId("terrain.grass");
    case TileGraphic::Path: return MakeAssetId("terrain.path");
    case TileGraphic::FlowerWhite: return MakeAssetId("decor.flower.white");
    case TileGraphic::OakTree: return MakeAssetId("tree.oak.big");
    case TileGraphic::FenceHorizontal: return MakeAssetId("decor.fence.horizontal");
    case TileGraphic::FenceVertical: return MakeAssetId("decor.fence.vertical");
    case TileGraphic::Sign: return MakeAssetId("decor.sign");
    case TileGraphic::FarmlandDry: return MakeAssetId("terrain.farmland.dry.15");
    case TileGraphic::FarmlandWet: return MakeAssetId("terrain.farmland.wet.15");
    case TileGraphic::Farmhouse: return MakeAssetId("building.farmhouse");
    case TileGraphic::WoodFloor: return MakeAssetId("interior.floor.wood");
    case TileGraphic::InteriorWall: return MakeAssetId("interior.wall.wood");
    case TileGraphic::Bed: return MakeAssetId("interior.bed");
    case TileGraphic::Door: return MakeAssetId("interior.door");
    case TileGraphic::None:
    case TileGraphic::Count:
        return 0;
    }
    return 0;
}

bool AddAsset(
    AssetId asset,
    std::int32_t tileX,
    std::int32_t tileY,
    SpriteLayer layer,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    if (asset == 0) {
        return true;
    }
    const SpriteAsset* sprite = assets.FindSprite(asset);
    if (sprite == nullptr) {
        return false;
    }

    const float worldX = static_cast<float>(tileX * TileSize);
    const float worldY = static_cast<float>(tileY * TileSize);
    Float2 position{};
    if (layer == SpriteLayer::Ground) {
        position = camera.WorldToScreen({
            worldX + static_cast<float>(sprite->trimX),
            worldY + static_cast<float>(sprite->trimY)});
    } else {
        position = camera.WorldToScreen({
            worldX + (static_cast<float>(TileSize) - sprite->sourceWidth) * 0.5F + sprite->trimX,
            worldY + TileSize - sprite->sourceHeight + sprite->trimY});
    }

    SpriteCommand command{};
    command.x = position.x;
    command.y = position.y;
    command.width = static_cast<float>(sprite->width) * camera.Zoom();
    command.height = static_cast<float>(sprite->height) * camera.Zoom();
    command.uvX = sprite->x;
    command.uvY = sprite->y;
    command.uvWidth = sprite->width;
    command.uvHeight = sprite->height;
    command.depth = static_cast<std::uint16_t>(tileY * MaximumMapTiles + tileX);
    const float visibleBottom = worldY + TileSize - sprite->sourceHeight +
        sprite->trimY + sprite->height;
    command.sortY = static_cast<std::uint16_t>(std::clamp(visibleBottom, 0.0F, 65535.0F));
    command.layer = layer;
    return queue.Add(command);
}

AssetId FarmlandAsset(std::uint8_t mask, bool wet) noexcept {
    constexpr AssetId dry[]{
        MakeAssetId("terrain.farmland.dry.0"), MakeAssetId("terrain.farmland.dry.1"),
        MakeAssetId("terrain.farmland.dry.2"), MakeAssetId("terrain.farmland.dry.3"),
        MakeAssetId("terrain.farmland.dry.4"), MakeAssetId("terrain.farmland.dry.5"),
        MakeAssetId("terrain.farmland.dry.6"), MakeAssetId("terrain.farmland.dry.7"),
        MakeAssetId("terrain.farmland.dry.8"), MakeAssetId("terrain.farmland.dry.9"),
        MakeAssetId("terrain.farmland.dry.10"), MakeAssetId("terrain.farmland.dry.11"),
        MakeAssetId("terrain.farmland.dry.12"), MakeAssetId("terrain.farmland.dry.13"),
        MakeAssetId("terrain.farmland.dry.14"), MakeAssetId("terrain.farmland.dry.15")};
    constexpr AssetId watered[]{
        MakeAssetId("terrain.farmland.wet.0"), MakeAssetId("terrain.farmland.wet.1"),
        MakeAssetId("terrain.farmland.wet.2"), MakeAssetId("terrain.farmland.wet.3"),
        MakeAssetId("terrain.farmland.wet.4"), MakeAssetId("terrain.farmland.wet.5"),
        MakeAssetId("terrain.farmland.wet.6"), MakeAssetId("terrain.farmland.wet.7"),
        MakeAssetId("terrain.farmland.wet.8"), MakeAssetId("terrain.farmland.wet.9"),
        MakeAssetId("terrain.farmland.wet.10"), MakeAssetId("terrain.farmland.wet.11"),
        MakeAssetId("terrain.farmland.wet.12"), MakeAssetId("terrain.farmland.wet.13"),
        MakeAssetId("terrain.farmland.wet.14"), MakeAssetId("terrain.farmland.wet.15")};
    return wet ? watered[mask] : dry[mask];
}

} // namespace

std::uint8_t FarmlandConnectionMask(
    const TileMap& map,
    std::int32_t x,
    std::int32_t y) noexcept {
    constexpr std::int32_t offsets[][2]{{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    std::uint8_t mask = 0;
    for (std::uint8_t index = 0; index < 4; ++index) {
        const Tile* neighbor = map.Get(x + offsets[index][0], y + offsets[index][1]);
        if (neighbor != nullptr && (neighbor->flags & TileFlagValue(TileFlag::Tilled)) != 0)
            mask |= static_cast<std::uint8_t>(1U << index);
    }
    return mask;
}

TileBounds CalculateVisibleTileBounds(
    const Camera2D& camera,
    std::uint16_t mapWidth,
    std::uint16_t mapHeight) noexcept {
    const FloatRect bounds = camera.VisibleBounds();
    TileBounds result{};
    result.firstX = std::max(
        0, static_cast<std::int32_t>(std::floor(bounds.left / TileSize)) - 1);
    result.firstY = std::max(
        0, static_cast<std::int32_t>(std::floor(bounds.top / TileSize)) - 1);
    result.lastX = std::min(
        static_cast<std::int32_t>(mapWidth) - 1,
        static_cast<std::int32_t>(std::ceil(bounds.right / TileSize)));
    result.lastY = std::min(
        static_cast<std::int32_t>(mapHeight) - 1,
        static_cast<std::int32_t>(std::ceil(bounds.bottom / TileSize)));
    return result;
}

TileBounds CalculateVisibleObjectBounds(
    const Camera2D& camera,
    std::uint16_t mapWidth,
    std::uint16_t mapHeight) noexcept {
    const FloatRect bounds = camera.VisibleBounds();
    TileBounds result{};
    // The selected farmhouse is 96x128 and is anchored at its bottom-center tile.
    result.firstX = std::max(0, static_cast<std::int32_t>(std::floor(bounds.left / TileSize)) - 4);
    result.firstY = std::max(0, static_cast<std::int32_t>(std::floor(bounds.top / TileSize)) - 1);
    result.lastX = std::min(static_cast<std::int32_t>(mapWidth) - 1,
        static_cast<std::int32_t>(std::ceil(bounds.right / TileSize)) + 3);
    result.lastY = std::min(static_cast<std::int32_t>(mapHeight) - 1,
        static_cast<std::int32_t>(std::ceil(bounds.bottom / TileSize)) + 7);
    return result;
}

bool TileMapRenderer::Build(
    const TileMap& map,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue,
    TileMapRenderStats* stats) noexcept {
    queue.Clear();
    TileMapRenderStats result{};
    const TileBounds bounds = CalculateVisibleTileBounds(camera, map.Width(), map.Height());

    if (bounds.firstX <= bounds.lastX && bounds.firstY <= bounds.lastY) {
        for (std::int32_t y = bounds.firstY; y <= bounds.lastY; ++y) {
            for (std::int32_t x = bounds.firstX; x <= bounds.lastX; ++x) {
                const Tile* tile = map.Get(x, y);
                if (tile == nullptr) {
                    return false;
                }
                ++result.visitedTiles;
                const bool tilled = (tile->flags & TileFlagValue(TileFlag::Tilled)) != 0;
                const std::uint8_t mask = tilled ? FarmlandConnectionMask(map, x, y) : 0;
                const AssetId ground = tilled ? FarmlandAsset(mask, false) : GraphicAssetId(tile->ground);
                if (!AddAsset(
                        ground, x, y, SpriteLayer::Ground,
                        camera, assets, queue)) {
                    return false;
                }
                ++result.submittedSprites;
                if (tilled && (tile->flags & TileFlagValue(TileFlag::Watered)) != 0) {
                    if (!AddAsset(FarmlandAsset(mask, true), x, y,
                                  SpriteLayer::GroundDecoration, camera, assets, queue)) return false;
                    ++result.submittedSprites;
                }
            }
        }
    }
    const TileBounds objectBounds = CalculateVisibleObjectBounds(camera, map.Width(), map.Height());
    if (objectBounds.firstX <= objectBounds.lastX && objectBounds.firstY <= objectBounds.lastY) {
        for (std::int32_t y = objectBounds.firstY; y <= objectBounds.lastY; ++y) {
            for (std::int32_t x = objectBounds.firstX; x <= objectBounds.lastX; ++x) {
                const Tile* tile = map.Get(x, y);
                if (tile != nullptr && tile->object != 0) {
                    if (!AddAsset(GraphicAssetId(tile->object), x, y,
                                  SpriteLayer::Actor, camera, assets, queue)) return false;
                    ++result.submittedSprites;
                }
            }
        }
    }
    if (stats != nullptr) {
        *stats = result;
    }
    return true;
}

} // namespace Homestead

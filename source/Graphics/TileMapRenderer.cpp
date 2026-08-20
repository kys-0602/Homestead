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
    case TileGraphic::None:
    case TileGraphic::Count:
        return 0;
    }
    return 0;
}

bool AddGraphic(
    std::uint16_t graphic,
    std::int32_t tileX,
    std::int32_t tileY,
    SpriteLayer layer,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    if (graphic == 0) {
        return true;
    }
    const SpriteAsset* sprite = assets.FindSprite(GraphicAssetId(graphic));
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

} // namespace

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
                if (!AddGraphic(
                        tile->ground, x, y, SpriteLayer::Ground,
                        camera, assets, queue)) {
                    return false;
                }
                ++result.submittedSprites;
                if (tile->object != 0) {
                    if (!AddGraphic(
                            tile->object, x, y, SpriteLayer::Actor,
                            camera, assets, queue)) {
                        return false;
                    }
                    ++result.submittedSprites;
                }
            }
        }
    }
    queue.Sort();
    if (stats != nullptr) {
        *stats = result;
    }
    return true;
}

} // namespace Homestead

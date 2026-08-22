#include "Homestead/Graphics/CropRenderer.hpp"

#include <algorithm>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/World/CropField.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {

bool AddCrops(const CropField& field, const Camera2D& camera,
              const AssetStore& assets, RenderQueue& queue) noexcept {
    const FloatRect visible = camera.VisibleBounds();
    for (const CropInstance& crop : field.Crops()) {
        if (!crop.active) continue;
        const float worldX = static_cast<float>(crop.tileX * TileSize);
        const float worldY = static_cast<float>(crop.tileY * TileSize);
        if (worldX + TileSize < visible.left || worldX > visible.right ||
            worldY + TileSize < visible.top || worldY > visible.bottom) continue;
        const CropDefinition* definition = FindCropDefinition(crop.crop);
        if (definition == nullptr || crop.stage >= definition->stageSprites.size()) return false;
        const SpriteAsset* sprite = assets.FindSprite(definition->stageSprites[crop.stage]);
        if (sprite == nullptr) return false;
        const Float2 position = camera.WorldToScreen({
            worldX + (TileSize - sprite->sourceWidth) * 0.5F + sprite->trimX,
            worldY + TileSize - sprite->sourceHeight + sprite->trimY});
        SpriteCommand command{};
        command.x = position.x;
        command.y = position.y;
        command.width = static_cast<float>(sprite->width) * camera.Zoom();
        command.height = static_cast<float>(sprite->height) * camera.Zoom();
        command.uvX = sprite->x;
        command.uvY = sprite->y;
        command.uvWidth = sprite->width;
        command.uvHeight = sprite->height;
        command.depth = static_cast<std::uint16_t>(crop.tileY * MaximumMapTiles + crop.tileX);
        command.layer = SpriteLayer::GroundDecoration;
        if (!queue.Add(command)) return false;
    }
    return true;
}

} // namespace Homestead

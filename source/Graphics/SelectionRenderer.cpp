#include "Homestead/Graphics/SelectionRenderer.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {

bool AddSelectionOverlay(
    const TileSelection& selection,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    const SpriteAsset* sprite = assets.FindSprite(MakeAssetId("ui.pointer.idle"));
    if (sprite == nullptr) {
        return false;
    }
    const Float2 screen = camera.WorldToScreen({
        static_cast<float>(selection.x * TileSize) + sprite->trimX,
        static_cast<float>(selection.y * TileSize) + sprite->trimY});
    SpriteCommand command{};
    command.x = screen.x;
    command.y = screen.y;
    command.width = static_cast<float>(sprite->width) * camera.Zoom();
    command.height = static_cast<float>(sprite->height) * camera.Zoom();
    command.uvX = sprite->x;
    command.uvY = sprite->y;
    command.uvWidth = sprite->width;
    command.uvHeight = sprite->height;
    command.color = selection.valid && selection.inRange ? 0xA060FF60U : 0xA06060FFU;
    command.layer = SpriteLayer::Debug;
    return queue.Add(command);
}

} // namespace Homestead

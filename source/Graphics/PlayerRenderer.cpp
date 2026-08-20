#include "Homestead/Graphics/PlayerRenderer.hpp"

#include <algorithm>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/World/EntityWorld.hpp"

namespace Homestead {

bool PlayerRenderer::Add(
    const EntityWorld& world,
    const PlayerState& player,
    float interpolationAlpha,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    const TransformComponent* transform = world.Transform(player.entity);
    const SpriteComponent* spriteComponent = world.Sprite(player.entity);
    if (transform == nullptr || spriteComponent == nullptr) {
        return false;
    }
    const SpriteAsset* sprite = assets.FindSprite(spriteComponent->asset);
    if (sprite == nullptr) {
        return false;
    }

    const float alpha = std::clamp(interpolationAlpha, 0.0F, 1.0F);
    const WorldPosition feet{
        transform->previous.x + (transform->current.x - transform->previous.x) * alpha,
        transform->previous.y + (transform->current.y - transform->previous.y) * alpha};
    const Float2 screen = camera.WorldToScreen({
        feet.x - static_cast<float>(sprite->sourceWidth) * 0.5F + sprite->trimX,
        feet.y - PlayerSpriteFootY + sprite->trimY});

    SpriteCommand command{};
    command.x = screen.x;
    command.y = screen.y;
    command.width = static_cast<float>(sprite->width) * camera.Zoom();
    command.height = static_cast<float>(sprite->height) * camera.Zoom();
    command.uvX = sprite->x;
    command.uvY = sprite->y;
    command.uvWidth = sprite->width;
    command.uvHeight = sprite->height;
    command.sortY = static_cast<std::uint16_t>(std::clamp(feet.y, 0.0F, 65535.0F));
    command.layer = SpriteLayer::Actor;
    if (player.facing == FacingDirection::Left) {
        command.flags = SpriteFlagValue(SpriteFlag::FlipHorizontal);
    }
    return queue.Add(command);
}

} // namespace Homestead

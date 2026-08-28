#include "Homestead/Graphics/FarmAnimalRenderer.hpp"

#include <algorithm>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/World/FarmAnimals.hpp"

namespace Homestead {
namespace {

bool AddAnimals(const FarmAnimal* animals, std::size_t count,
                AssetId idleId, AssetId walkId, std::uint32_t animationTicks,
                float interpolationAlpha, const Camera2D& camera,
                const AssetStore& assets, RenderQueue& queue) noexcept {
    const float alpha = std::clamp(interpolationAlpha, 0.0F, 1.0F);
    for (std::size_t index = 0; index < count; ++index) {
        const FarmAnimal& animal = animals[index];
        const AssetId assetId = animal.moving && (animationTicks / 10U) % 2U != 0U ? walkId : idleId;
        const SpriteAsset* sprite = assets.FindSprite(assetId);
        if (sprite == nullptr) return false;
        const WorldPosition feet{
            animal.previous.x + (animal.current.x - animal.previous.x) * alpha,
            animal.previous.y + (animal.current.y - animal.previous.y) * alpha};
        const Float2 position = camera.WorldToScreen({
            feet.x - static_cast<float>(sprite->sourceWidth) * 0.5F + sprite->trimX,
            feet.y - static_cast<float>(sprite->sourceHeight) + sprite->trimY});

        SpriteCommand command{};
        command.x = position.x;
        command.y = position.y;
        command.width = static_cast<float>(sprite->width) * camera.Zoom();
        command.height = static_cast<float>(sprite->height) * camera.Zoom();
        command.uvX = sprite->x;
        command.uvY = sprite->y;
        command.uvWidth = sprite->width;
        command.uvHeight = sprite->height;
        command.sortY = static_cast<std::uint16_t>(std::clamp(feet.y, 0.0F, 65535.0F));
        command.layer = SpriteLayer::Actor;
        if (animal.current.x > animal.previous.x) command.flags = SpriteFlagValue(SpriteFlag::FlipHorizontal);
        if (!queue.Add(command)) return false;
    }
    return true;
}

} // namespace

bool AddFarmAnimals(
    const FarmAnimals& animals,
    std::uint32_t animationTicks,
    float interpolationAlpha,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    return AddAnimals(animals.Chickens().data(), animals.Chickens().size(),
                      MakeAssetId("animal.chicken.idle"), MakeAssetId("animal.chicken.walk"),
                      animationTicks, interpolationAlpha, camera, assets, queue) &&
        AddAnimals(animals.Frogs().data(), animals.Frogs().size(),
                   MakeAssetId("animal.frog.idle"), MakeAssetId("animal.frog.walk"),
                   animationTicks, interpolationAlpha, camera, assets, queue);
}

} // namespace Homestead

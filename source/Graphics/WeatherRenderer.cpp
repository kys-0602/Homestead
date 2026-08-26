#include "Homestead/Graphics/WeatherRenderer.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

#include <array>
#include <cstddef>

namespace Homestead {

bool AddCloudShadows(
    std::uint32_t animationTicks,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    constexpr std::uint32_t TicksPerPixel = 12;
    constexpr std::uint32_t CycleWidth = 640;
    constexpr std::uint32_t FarmWorldHeight = 384;
    constexpr int MaximumCloudWidth = 44;
    constexpr std::uint32_t MaximumCloudHeight = 34;
    constexpr std::array<std::uint32_t, 4> BaseX{31, 183, 374, 552};
    constexpr std::array<AssetId, 4> CloudIds{
        MakeAssetId("effect.cloud.0"), MakeAssetId("effect.cloud.1"),
        MakeAssetId("effect.cloud.2"), MakeAssetId("effect.cloud.3")};
    const std::uint32_t travel = animationTicks / TicksPerPixel;

    for (std::size_t index = 0; index < BaseX.size(); ++index) {
        const std::uint32_t absoluteX = BaseX[index] + travel;
        const std::uint32_t lap = absoluteX / CycleWidth;
        std::uint32_t random = (static_cast<std::uint32_t>(index) + 1U) * 0x9E3779B9U ^
            (lap + 7U) * 0x85EBCA6BU;
        random ^= random >> 16U;
        const SpriteAsset* sprite = assets.FindSprite(CloudIds[random % CloudIds.size()]);
        if (sprite == nullptr) return false;

        const Float2 screen = camera.WorldToScreen({
            static_cast<float>(static_cast<int>(absoluteX % CycleWidth) - MaximumCloudWidth +
                               sprite->trimX),
            static_cast<float>(8U + (random >> 8U) %
                (FarmWorldHeight - MaximumCloudHeight - 16U) + sprite->trimY)});

        SpriteCommand command{};
        command.x = screen.x;
        command.y = screen.y;
        command.width = static_cast<float>(sprite->width) * camera.Zoom();
        command.height = static_cast<float>(sprite->height) * camera.Zoom();
        command.uvX = sprite->x;
        command.uvY = sprite->y;
        command.uvWidth = sprite->width;
        command.uvHeight = sprite->height;
        command.color = 0x60FFFFFFU;
        command.layer = SpriteLayer::Effect;
        if (!queue.Add(command)) return false;
    }
    return true;
}

} // namespace Homestead

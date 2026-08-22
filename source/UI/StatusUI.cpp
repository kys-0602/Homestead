#include "Homestead/UI/StatusUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/World/WorldClock.hpp"

namespace Homestead {
namespace {

bool AddFade(std::uint8_t alpha, std::uint16_t depth,
             const SpriteAsset& pixel, RenderQueue& queue) noexcept {
    if (alpha == 0) return true;
    SpriteCommand command{};
    command.x = 0.0F; command.y = 0.0F;
    command.width = 320.0F; command.height = 180.0F;
    command.uvX = pixel.x; command.uvY = pixel.y;
    command.uvWidth = pixel.width; command.uvHeight = pixel.height;
    command.color = static_cast<std::uint32_t>(alpha) << 24U;
    command.layer = SpriteLayer::UI;
    command.depth = depth;
    return queue.Add(command);
}

} // namespace

bool AddStatusUI(const WorldClock& clock, std::uint8_t harvested, std::uint8_t goal,
                 bool showInstructions, bool complete, const AssetStore& assets,
                 RenderQueue& queue) noexcept {
    const SpriteAsset* pixel = assets.FindSprite(MakeAssetId("terrain.grass"));
    if (pixel == nullptr ||
        !AddFade(clock.FadeAlpha(), 100, *pixel, queue)) return false;
    if (clock.IsTransitioning()) return true;

    if (complete && !AddFade(176, 10, *pixel, queue)) return false;

    char day[] = "DAY 00";
    day[4] = static_cast<char>('0' + (clock.Day() / 10U) % 10U);
    day[5] = static_cast<char>('0' + clock.Day() % 10U);
    char time[] = "TIME 0000";
    const std::uint16_t hour = static_cast<std::uint16_t>((clock.Minute() / 60U) % 24U);
    const std::uint16_t minute = static_cast<std::uint16_t>(clock.Minute() % 60U);
    time[5] = static_cast<char>('0' + hour / 10U);
    time[6] = static_cast<char>('0' + hour % 10U);
    time[7] = static_cast<char>('0' + minute / 10U);
    time[8] = static_cast<char>('0' + minute % 10U);
    char progress[] = "CARROTS 0 OF 0";
    progress[8] = static_cast<char>('0' + harvested);
    progress[13] = static_cast<char>('0' + goal);
    if (!AddBitmapText(day, 4.0F, 4.0F, 0xFFFFFFFFU, 20, assets, queue) ||
        !AddBitmapText(time, 262.0F, 4.0F, 0xFFFFFFFFU, 20, assets, queue) ||
        !AddBitmapText(progress, 4.0F, 14.0F, 0xFFFFFFFFU, 20, assets, queue)) return false;
    if (complete) {
        return AddBitmapText("GOAL COMPLETE", 121.0F, 82.0F,
                             0xFF80FFFFU, 20, assets, queue);
    }
    if (showInstructions) {
        return AddBitmapText("GROW 3 CARROTS", 116.0F, 72.0F,
                             0xFFFFFFFFU, 20, assets, queue) &&
            AddBitmapText("N ENDS DAY", 127.0F, 82.0F,
                          0xFFFFFFFFU, 20, assets, queue);
    }
    return true;
}

} // namespace Homestead

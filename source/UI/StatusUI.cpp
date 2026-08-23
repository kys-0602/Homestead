#include "Homestead/UI/StatusUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/Strings.hpp"
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

bool CompletionContinueAt(std::uint32_t x, std::uint32_t y) noexcept {
    return x >= 126 && x < 194 && y >= 92 && y < 104;
}

bool AddStatusUI(const WorldClock& clock, std::uint16_t gold, std::uint16_t goal,
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
    char progress[] = "GOLD 00000 OF 00000";
    for (int index=9;index>=5;--index) { progress[index]=static_cast<char>('0'+gold%10U); gold/=10U; }
    for (int index=18;index>=14;--index) { progress[index]=static_cast<char>('0'+goal%10U); goal/=10U; }
    if (!AddBitmapText(day, 4.0F, 4.0F, 0xFFFFFFFFU, 20, assets, queue) ||
        !AddBitmapText(time, 262.0F, 4.0F, 0xFFFFFFFFU, 20, assets, queue) ||
        !AddBitmapText(progress, 4.0F, 14.0F, 0xFFFFFFFFU, 20, assets, queue)) return false;
    if (complete) {
        return AddBitmapText(Text::GoalComplete, 121.0F, 82.0F,
                             0xFF80FFFFU, 20, assets, queue) &&
            AddBitmapText(Text::Continue, 133.0F, 96.0F,
                          0xFFFFFFFFU, 20, assets, queue);
    }
    if (showInstructions) {
        return AddBitmapText(Text::StartTools, 92.0F, 67.0F,
                             0xFFFFFFFFU, 20, assets, queue) &&
            AddBitmapText(Text::StartControls, 95.0F, 77.0F,
                          0xFFFFFFFFU, 20, assets, queue) &&
            AddBitmapText("M MARKET", 133.0F, 87.0F,
                          0xFFFFFFFFU, 20, assets, queue);
    }
    return true;
}

} // namespace Homestead

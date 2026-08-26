#include "Homestead/UI/StatusUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/Strings.hpp"
#include "Homestead/UI/UIPanel.hpp"
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
    return x >= 124 && x < 196 && y >= 90 && y < 108;
}

bool AddStatusUI(const WorldClock& clock, std::uint16_t gold, std::uint16_t goal,
                 bool showInstructions, bool showSaved, bool complete, const AssetStore& assets,
                 RenderQueue& queue) noexcept {
    const SpriteAsset* pixel = assets.FindSprite(MakeAssetId("terrain.grass"));
    if (pixel == nullptr ||
        !AddFade(clock.FadeAlpha(), 100, *pixel, queue)) return false;
    if (clock.IsTransitioning()) return true;

    if (complete && !AddFade(176, 10, *pixel, queue)) return false;

    char day[] = "DAY 00";
    day[4] = static_cast<char>('0' + (clock.Day() / 10U) % 10U);
    day[5] = static_cast<char>('0' + clock.Day() % 10U);
    char time[] = "TIME 00:00";
    const std::uint16_t hour = static_cast<std::uint16_t>((clock.Minute() / 60U) % 24U);
    const std::uint16_t minute = static_cast<std::uint16_t>(clock.Minute() % 60U);
    time[5] = static_cast<char>('0' + hour / 10U);
    time[6] = static_cast<char>('0' + hour % 10U);
    time[8] = static_cast<char>('0' + minute / 10U);
    time[9] = static_cast<char>('0' + minute % 10U);
    char progress[] = "GOLD 00000 OF 00000";
    for (int index=9;index>=5;--index) { progress[index]=static_cast<char>('0'+gold%10U); gold/=10U; }
    for (int index=18;index>=14;--index) { progress[index]=static_cast<char>('0'+goal%10U); goal/=10U; }
    constexpr std::uint32_t hudTextColor=0xFF603020U;
    if (!AddUIPanel(0.0F,0.0F,128.0F,26.0F,0xFFFFFFFFU,18,assets,queue) ||
        !AddUIPanel(250.0F,0.0F,70.0F,16.0F,0xFFFFFFFFU,18,assets,queue) ||
        !AddBitmapText(day, 6.0F, 5.0F, hudTextColor, 20, assets, queue) ||
        !AddBitmapText(time, 256.0F, 5.0F, hudTextColor, 20, assets, queue) ||
        !AddBitmapText(progress, 6.0F, 15.0F, hudTextColor, 20, assets, queue)) return false;
    if (complete) {
        return AddUIPanel(108.0F,68.0F,104.0F,50.0F,
                          0xFFFFFFFFU,22,assets,queue) &&
            AddUIPanel(124.0F,90.0F,72.0F,18.0F,
                       0xFFB87858U,24,assets,queue) &&
            AddBitmapText(Text::GoalComplete, 121.0F, 78.0F,
                             hudTextColor, 26, assets, queue) &&
            AddBitmapText(Text::Continue, 133.0F, 96.0F,
                          0xFFFFFFFFU, 26, assets, queue);
    }
    if (showSaved && (!AddUIPanel(126.0F,24.0F,68.0F,16.0F,
                                  0xFFFFFFFFU,22,assets,queue) ||
        !AddBitmapText("GAME SAVED", 133.0F, 29.0F,
                       hudTextColor, 24, assets, queue))) return false;
    if (showInstructions) {
        return AddUIPanel(82.0F,60.0F,156.0F,38.0F,
                          0xFFFFFFFFU,22,assets,queue) &&
            AddBitmapText(Text::StartTools, 88.0F, 67.0F,
                             hudTextColor, 24, assets, queue) &&
            AddBitmapText(Text::StartControls, 88.0F, 77.0F,
                          hudTextColor, 24, assets, queue) &&
            AddBitmapText("M MARKET", 139.0F, 87.0F,
                          hudTextColor, 24, assets, queue);
    }
    return true;
}

} // namespace Homestead

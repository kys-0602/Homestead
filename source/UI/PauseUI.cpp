#include "Homestead/UI/PauseUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Platform/Settings.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/Strings.hpp"
#include "Homestead/UI/UIPanel.hpp"

namespace Homestead {
namespace {

constexpr std::int32_t PanelX = 68;
constexpr std::int32_t PanelY = 24;
constexpr std::int32_t PanelWidth = 184;
constexpr std::int32_t ItemY = 48;
constexpr std::int32_t ItemHeight = 14;

bool AddNumber(std::uint8_t value, float x, float y, std::uint32_t color, const AssetStore& assets,
               RenderQueue& queue) noexcept {
    char text[3]{};
    if (value == 10) { text[0] = '1'; text[1] = '0'; }
    else text[0] = static_cast<char>('0' + value);
    return AddBitmapText(text, x, y, color, 43, assets, queue);
}

} // namespace

int PauseItemAt(std::uint32_t x, std::uint32_t y) noexcept {
    if (x < PanelX || x >= PanelX + PanelWidth || y < ItemY ||
        y >= ItemY + PauseItemCount * ItemHeight) return -1;
    return static_cast<int>((y - ItemY) / ItemHeight);
}

bool AddPauseUI(const Settings& settings, std::uint8_t focus,
                const AssetStore& assets, RenderQueue& queue) noexcept {
    const SpriteAsset* pointer = assets.FindSprite(MakeAssetId("ui.pointer.idle"));
    if (pointer == nullptr || !AddUIPanel(static_cast<float>(PanelX),static_cast<float>(PanelY),
        static_cast<float>(PanelWidth),140.0F,0xFFFFFFFFU,40,assets,queue) ||
        !AddBitmapText(Text::Paused, 142.0F, 34.0F, 0xFF603020U, 43, assets, queue)) return false;
    constexpr const char* labels[PauseItemCount] = {
        Text::Resume, Text::Inventory, Text::WindowSize, Text::Fullscreen,
        Text::Master, Text::Music, Text::Effects, Text::Quit};
    for (std::uint8_t index = 0; index < PauseItemCount; ++index) {
        const float y = static_cast<float>(ItemY + index * ItemHeight + 4);
        if (index == focus && !AddUIFill(78.0F,static_cast<float>(ItemY+index*ItemHeight),
                                        164.0F,static_cast<float>(ItemHeight),
                                        0xFFB87858U,41,assets,queue)) return false;
        if (!AddBitmapText(labels[index], 91.0F, y,
                           index==focus?0xFFFFFFFFU:0xFF603020U, 43, assets, queue)) return false;
    }
    const float valueX = 211.0F;
    char size[] = "0000";
    const unsigned width = 320U * settings.windowScale;
    size[0] = static_cast<char>('0' + width / 1000U);
    size[1] = static_cast<char>('0' + (width / 100U) % 10U);
    size[2] = static_cast<char>('0' + (width / 10U) % 10U);
    size[3] = static_cast<char>('0' + width % 10U);
    const char* visibleSize = width < 1000U ? size + 1 : size;
    constexpr std::uint32_t normalText=0xFF603020U;
    constexpr std::uint32_t selectedText=0xFFFFFFFFU;
    if (!AddBitmapText(visibleSize, valueX - (width < 1000U ? 0.0F : 6.0F), 80.0F,
                       focus==2?selectedText:normalText, 43, assets, queue) ||
        !AddBitmapText(settings.fullscreen ? Text::On : Text::Off, valueX, 94.0F,
                       focus==3?selectedText:normalText, 43, assets, queue) ||
        !AddNumber(settings.masterVolume, valueX, 108.0F,
                   focus==4?selectedText:normalText, assets, queue) ||
        !AddNumber(settings.musicVolume, valueX, 122.0F,
                   focus==5?selectedText:normalText, assets, queue) ||
        !AddNumber(settings.effectVolume, valueX, 136.0F,
                   focus==6?selectedText:normalText, assets, queue)) return false;
    SpriteCommand cursor{};
    cursor.x = 73.0F + pointer->trimX;
    cursor.y = static_cast<float>(ItemY + focus * ItemHeight) + pointer->trimY;
    cursor.width = static_cast<float>(pointer->width); cursor.height = static_cast<float>(pointer->height);
    cursor.uvX = pointer->x; cursor.uvY = pointer->y;
    cursor.uvWidth = pointer->width; cursor.uvHeight = pointer->height;
    cursor.color = 0xFFFFFFFFU; cursor.layer = SpriteLayer::UI; cursor.depth = 44;
    return queue.Add(cursor);
}

} // namespace Homestead

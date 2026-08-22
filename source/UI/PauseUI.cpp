#include "Homestead/UI/PauseUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Platform/Settings.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/Strings.hpp"

namespace Homestead {
namespace {

constexpr std::int32_t PanelX = 68;
constexpr std::int32_t PanelY = 24;
constexpr std::int32_t PanelWidth = 184;
constexpr std::int32_t ItemY = 48;
constexpr std::int32_t ItemHeight = 14;

bool AddPanel(const SpriteAsset& pixel, RenderQueue& queue) noexcept {
    SpriteCommand command{};
    command.x = static_cast<float>(PanelX); command.y = static_cast<float>(PanelY);
    command.width = static_cast<float>(PanelWidth); command.height = 126.0F;
    command.uvX = pixel.x; command.uvY = pixel.y;
    command.uvWidth = pixel.width; command.uvHeight = pixel.height;
    command.color = 0xE0203020U; command.layer = SpriteLayer::UI; command.depth = 40;
    return queue.Add(command);
}

bool AddNumber(std::uint8_t value, float x, float y, const AssetStore& assets,
               RenderQueue& queue) noexcept {
    char text[3]{};
    if (value == 10) { text[0] = '1'; text[1] = '0'; }
    else text[0] = static_cast<char>('0' + value);
    return AddBitmapText(text, x, y, 0xFFFFFFFFU, 43, assets, queue);
}

} // namespace

int PauseItemAt(std::uint32_t x, std::uint32_t y) noexcept {
    if (x < PanelX || x >= PanelX + PanelWidth || y < ItemY ||
        y >= ItemY + PauseItemCount * ItemHeight) return -1;
    return static_cast<int>((y - ItemY) / ItemHeight);
}

bool AddPauseUI(const Settings& settings, std::uint8_t focus,
                const AssetStore& assets, RenderQueue& queue) noexcept {
    const SpriteAsset* pixel = assets.FindSprite(MakeAssetId("terrain.grass"));
    const SpriteAsset* pointer = assets.FindSprite(MakeAssetId("ui.pointer.idle"));
    if (pixel == nullptr || pointer == nullptr || !AddPanel(*pixel, queue) ||
        !AddBitmapText(Text::Paused, 142.0F, 34.0F, 0xFF80FFFFU, 43, assets, queue)) return false;
    constexpr const char* labels[PauseItemCount] = {
        Text::Resume, Text::Inventory, Text::WindowSize, Text::Fullscreen,
        Text::Master, Text::Music, Text::Effects};
    for (std::uint8_t index = 0; index < PauseItemCount; ++index) {
        const float y = static_cast<float>(ItemY + index * ItemHeight + 4);
        if (!AddBitmapText(labels[index], 91.0F, y, 0xFFFFFFFFU, 42, assets, queue)) return false;
    }
    const float valueX = 211.0F;
    char size[] = "0000";
    const unsigned width = 320U * settings.windowScale;
    size[0] = static_cast<char>('0' + width / 1000U);
    size[1] = static_cast<char>('0' + (width / 100U) % 10U);
    size[2] = static_cast<char>('0' + (width / 10U) % 10U);
    size[3] = static_cast<char>('0' + width % 10U);
    const char* visibleSize = width < 1000U ? size + 1 : size;
    if (!AddBitmapText(visibleSize, valueX - (width < 1000U ? 0.0F : 6.0F), 80.0F,
                       0xFFFFFFFFU, 43, assets, queue) ||
        !AddBitmapText(settings.fullscreen ? Text::On : Text::Off, valueX, 94.0F,
                       0xFFFFFFFFU, 43, assets, queue) ||
        !AddNumber(settings.masterVolume, valueX, 108.0F, assets, queue) ||
        !AddNumber(settings.musicVolume, valueX, 122.0F, assets, queue) ||
        !AddNumber(settings.effectVolume, valueX, 136.0F, assets, queue)) return false;
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

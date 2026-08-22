#include "Homestead/UI/BitmapText.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

namespace Homestead {
namespace {

bool Glyph(char value, std::uint16_t& column, std::uint16_t& y) noexcept {
    if (value >= 'A' && value <= 'Z') {
        column = static_cast<std::uint16_t>(value - 'A'); y = 2; return true;
    }
    if (value >= '1' && value <= '9') {
        column = static_cast<std::uint16_t>(value - '1'); y = 23; return true;
    }
    if (value == '0') { column = 9; y = 23; return true; }
    return false;
}

} // namespace

bool AddBitmapText(const char* text, float x, float y, std::uint32_t color,
                   std::uint16_t depth, const AssetStore& assets,
                   RenderQueue& queue) noexcept {
    const SpriteAsset* font = assets.FindSprite(MakeAssetId("font.small"));
    if (font == nullptr) return false;
    float cursor = x;
    while (*text != '\0') {
        if (*text == ' ') {
            cursor += 6.0F;
            ++text;
            continue;
        }
        std::uint16_t column = 0;
        std::uint16_t glyphY = 0;
        if (!Glyph(*text, column, glyphY)) return false;
        SpriteCommand command{};
        command.x = cursor;
        command.y = y;
        command.width = 5.0F;
        command.height = 5.0F;
        command.uvX = static_cast<std::uint16_t>(font->x + column * 5U);
        const std::uint16_t atlasGlyphY = glyphY > font->trimY ?
            static_cast<std::uint16_t>(glyphY - font->trimY) : 0;
        command.uvY = static_cast<std::uint16_t>(font->y + atlasGlyphY);
        command.uvWidth = 5;
        command.uvHeight = 5;
        command.color = color;
        command.layer = SpriteLayer::UI;
        command.depth = depth;
        if (!queue.Add(command)) return false;
        cursor += 6.0F;
        ++text;
    }
    return true;
}

} // namespace Homestead

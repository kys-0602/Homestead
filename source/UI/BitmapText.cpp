#include "Homestead/UI/BitmapText.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

namespace Homestead {
namespace {

AssetId GlyphId(char value) noexcept {
    switch (value) {
    case 'A': return MakeAssetId("font.A"); case 'C': return MakeAssetId("font.C");
    case 'D': return MakeAssetId("font.D"); case 'E': return MakeAssetId("font.E");
    case 'F': return MakeAssetId("font.F"); case 'G': return MakeAssetId("font.G");
    case 'I': return MakeAssetId("font.I"); case 'L': return MakeAssetId("font.L");
    case 'M': return MakeAssetId("font.M"); case 'N': return MakeAssetId("font.N");
    case 'O': return MakeAssetId("font.O"); case 'P': return MakeAssetId("font.P");
    case 'R': return MakeAssetId("font.R"); case 'S': return MakeAssetId("font.S");
    case 'T': return MakeAssetId("font.T"); case 'U': return MakeAssetId("font.U");
    case 'V': return MakeAssetId("font.V"); case 'W': return MakeAssetId("font.W");
    case 'Y': return MakeAssetId("font.Y"); case 'Z': return MakeAssetId("font.Z");
    case '0': return MakeAssetId("font.0"); case '1': return MakeAssetId("font.1");
    case '2': return MakeAssetId("font.2"); case '3': return MakeAssetId("font.3");
    case '4': return MakeAssetId("font.4"); case '5': return MakeAssetId("font.5");
    case '6': return MakeAssetId("font.6"); case '7': return MakeAssetId("font.7");
    case '8': return MakeAssetId("font.8"); case '9': return MakeAssetId("font.9");
    default: return 0;
    }
}

} // namespace

bool AddBitmapText(const char* text, float x, float y, std::uint32_t color,
                   std::uint16_t depth, const AssetStore& assets,
                   RenderQueue& queue) noexcept {
    float cursor = x;
    while (*text != '\0') {
        if (*text == ' ') {
            cursor += 6.0F;
            ++text;
            continue;
        }
        const AssetId glyphId = GlyphId(*text);
        if (glyphId == 0) return false;
        const SpriteAsset* glyph = assets.FindSprite(glyphId);
        SpriteCommand command{};
        command.x = cursor + glyph->trimX;
        command.y = y + glyph->trimY;
        command.width = static_cast<float>(glyph->width);
        command.height = static_cast<float>(glyph->height);
        command.uvX = glyph->x;
        command.uvY = glyph->y;
        command.uvWidth = glyph->width;
        command.uvHeight = glyph->height;
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

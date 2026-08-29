#include "Homestead/UI/CropCatalogueUI.hpp"

#include <cstddef>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/Economy.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/UIPanel.hpp"
#include "Homestead/World/CropCatalogue.hpp"

namespace Homestead {
namespace {
void Append(char* text, std::size_t& length, const char* value) noexcept { while (*value != '\0') text[length++] = *value++; }
void AppendNumber(char* text, std::size_t& length, unsigned value) noexcept {
    char digits[5]{}; std::size_t count = 0;
    do { digits[count++] = static_cast<char>('0' + value % 10U); value /= 10U; } while (value != 0);
    while (count != 0) text[length++] = digits[--count];
}
} // namespace
bool AddCropCatalogueUI(const CropCatalogue& catalogue, const AssetStore& assets, RenderQueue& queue) noexcept {
    constexpr std::uint32_t text = 0xFF603020U, seen = 0xFF305840U;
    constexpr std::uint32_t silver = 0xFFC0C0C0U, gold = 0xFFFFC840U;
    if (!AddUIPanel(45, 18, 230, 146, 0xFFFFFFFFU, 40, assets, queue) ||
        !AddBitmapText("CROP CATALOGUE", 116, 28, text, 43, assets, queue) ||
        !AddBitmapText("SILVER 125% GOLD 150%", 96, 40, text, 43, assets, queue) ||
        !AddBitmapText("S", 190, 48, silver, 43, assets, queue) ||
        !AddBitmapText("G", 238, 48, gold, 43, assets, queue)) return false;
    for (std::uint8_t index = 0; index < MarketCropCount; ++index) {
        const CropId crop = static_cast<CropId>(index + 1U);
        const float y = 56.0F + static_cast<float>(index * 14U);
        if (!catalogue.IsDiscovered(crop)) {
            if (!AddBitmapText("UNSEEN", 80, y, text, 43, assets, queue)) return false;
            continue;
        }
        const CropDefinition* definition = FindCropDefinition(crop);
        if (definition == nullptr) return false;
        char line[24]{}; std::size_t length = 0;
        Append(line, length, definition->name); line[length++] = ' ';
        AppendNumber(line, length, definition->growthDays); line[length++] = 'D'; line[length++] = ' ';
        AppendNumber(line, length, MarketEntries()[index].sellPrice); line[length++] = 'G';
        if (!AddBitmapText(line, 80, y, seen, 43, assets, queue) ||
            !AddBitmapText(catalogue.HasQuality(crop, ItemQuality::Silver) ? "YES" : "NO",
                182, y, silver, 43, assets, queue) ||
            !AddBitmapText(catalogue.HasQuality(crop, ItemQuality::Gold) ? "YES" : "NO",
                228, y, gold, 43, assets, queue)) return false;
    }
    return AddBitmapText("E OR ESC CLOSE", 106, 148, text, 43, assets, queue);
}
} // namespace Homestead

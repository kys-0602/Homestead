#include "Homestead/Game/Economy.hpp"

#include <limits>
#include <iterator>

namespace Homestead {

bool BuySeed(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= MarketTable.size() || gold < MarketTable[crop].seedPrice) return false;
    if (inventory.Add(MarketTable[crop].seed, 1) != 0) return false;
    gold = static_cast<std::uint16_t>(gold - MarketTable[crop].seedPrice);
    return true;
}

bool SellHarvest(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= MarketTable.size()) return false;
    std::uint16_t qualityCounts[static_cast<std::size_t>(ItemQuality::Count)]{};
    std::uint32_t proceeds = 0;
    for (std::size_t index = 0; index < Inventory::SlotCount; ++index) {
        const ItemStack& stack = inventory.Slot(index);
        if (stack.item != MarketTable[crop].harvest) continue;
        const std::size_t quality = static_cast<std::size_t>(stack.quality);
        if (quality >= std::size(qualityCounts)) return false;
        qualityCounts[quality] = static_cast<std::uint16_t>(qualityCounts[quality] + stack.count);
        proceeds += static_cast<std::uint32_t>(stack.count) * MarketTable[crop].sellPrice *
            QualitySalePercent(stack.quality) / 100U;
    }
    if (proceeds == 0) return false;
    const std::uint32_t capacity = static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) - gold;
    if (proceeds > capacity) return false;
    for (std::size_t quality = 0; quality < std::size(qualityCounts); ++quality) {
        if (qualityCounts[quality] != 0 && !inventory.Remove(
                MarketTable[crop].harvest, qualityCounts[quality], static_cast<ItemQuality>(quality))) return false;
    }
    gold = static_cast<std::uint16_t>(gold + proceeds);
    return true;
}

} // namespace Homestead

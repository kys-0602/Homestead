#include "Homestead/Game/Economy.hpp"

#include <limits>

namespace Homestead {

bool BuySeed(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= MarketTable.size() || gold < MarketTable[crop].seedPrice) return false;
    if (inventory.Add(MarketTable[crop].seed, 1) != 0) return false;
    gold = static_cast<std::uint16_t>(gold - MarketTable[crop].seedPrice);
    return true;
}

bool SellHarvest(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= MarketTable.size()) return false;
    const std::uint16_t count = inventory.Count(MarketTable[crop].harvest);
    if (count == 0) return false;
    const std::uint32_t proceeds = static_cast<std::uint32_t>(count) * MarketTable[crop].sellPrice;
    const std::uint32_t capacity = static_cast<std::uint32_t>(std::numeric_limits<std::uint16_t>::max()) - gold;
    if (proceeds > capacity || !inventory.Remove(MarketTable[crop].harvest, count)) return false;
    gold = static_cast<std::uint16_t>(gold + proceeds);
    return true;
}

} // namespace Homestead

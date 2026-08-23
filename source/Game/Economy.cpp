#include "Homestead/Game/Economy.hpp"

#include <algorithm>
#include <limits>

namespace Homestead {
namespace {

constexpr std::array<MarketEntry, MarketCropCount> Entries{{
    {ItemId::WheatSeed, ItemId::Wheat, 4, 7},
    {ItemId::CarrotSeed, ItemId::Carrot, 6, 12},
    {ItemId::TomatoSeed, ItemId::Tomato, 9, 20}}};

} // namespace

const std::array<MarketEntry, MarketCropCount>& MarketEntries() noexcept { return Entries; }

bool BuySeed(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= Entries.size() || gold < Entries[crop].seedPrice) return false;
    if (inventory.Add(Entries[crop].seed, 1) != 0) return false;
    gold = static_cast<std::uint16_t>(gold - Entries[crop].seedPrice);
    return true;
}

bool SellHarvest(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept {
    if (crop >= Entries.size()) return false;
    const std::uint16_t count = inventory.Count(Entries[crop].harvest);
    if (count == 0) return false;
    const std::uint32_t proceeds = static_cast<std::uint32_t>(count) * Entries[crop].sellPrice;
    const std::uint16_t added = static_cast<std::uint16_t>(std::min<std::uint32_t>(
        proceeds, std::numeric_limits<std::uint16_t>::max() - gold));
    if (added == 0 || !inventory.Remove(Entries[crop].harvest, count)) return false;
    gold = static_cast<std::uint16_t>(gold + added);
    return true;
}

} // namespace Homestead

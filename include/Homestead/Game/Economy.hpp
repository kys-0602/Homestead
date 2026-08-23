#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Game/Inventory.hpp"

namespace Homestead {

struct MarketEntry {
    ItemId seed = ItemId::None;
    ItemId harvest = ItemId::None;
    std::uint8_t seedPrice = 0;
    std::uint8_t sellPrice = 0;
};

inline constexpr std::uint16_t StartingGold = 20;
inline constexpr std::uint16_t GoalGold = 100;
inline constexpr std::size_t MarketCropCount = 3;

[[nodiscard]] const std::array<MarketEntry, MarketCropCount>& MarketEntries() noexcept;
[[nodiscard]] bool BuySeed(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept;
[[nodiscard]] bool SellHarvest(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept;

} // namespace Homestead

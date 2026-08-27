#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Game/Inventory.hpp"

namespace Homestead {

struct MarketEntry {
    const char* name = "";
    ItemId seed = ItemId::None;
    ItemId harvest = ItemId::None;
    std::uint8_t seedPrice = 0;
    std::uint8_t sellPrice = 0;
};

inline constexpr std::uint16_t StartingGold = 20;
inline constexpr std::uint16_t GoalGold = 100;
inline constexpr std::size_t MarketCropCount = 6;
inline constexpr std::array<MarketEntry, MarketCropCount> MarketTable{{
    {"WHEAT", ItemId::WheatSeed, ItemId::Wheat, 4, 7},
    {"CARROT", ItemId::CarrotSeed, ItemId::Carrot, 6, 12},
    {"TOMATO", ItemId::TomatoSeed, ItemId::Tomato, 9, 20},
    {"POTATO", ItemId::PotatoSeed, ItemId::Potato, 5, 10},
    {"CORN", ItemId::CornSeed, ItemId::Corn, 8, 17},
    {"CABBAGE", ItemId::CabbageSeed, ItemId::Cabbage, 12, 30}}};

[[nodiscard]] inline constexpr const std::array<MarketEntry, MarketCropCount>& MarketEntries() noexcept {
    return MarketTable;
}
[[nodiscard]] bool BuySeed(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept;
[[nodiscard]] bool SellHarvest(Inventory& inventory, std::uint16_t& gold, std::size_t crop) noexcept;

} // namespace Homestead

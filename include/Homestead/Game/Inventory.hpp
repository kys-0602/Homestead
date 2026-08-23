#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"

namespace Homestead {

enum class ItemId : std::uint8_t {
    None,
    WheatSeed,
    Wheat,
    CarrotSeed,
    Carrot,
    TomatoSeed,
    Tomato,
    Hoe,
    WateringCan
};

enum class ItemCategory : std::uint8_t {
    Seed,
    Harvest,
    Tool
};

struct ItemDefinition {
    ItemId id = ItemId::None;
    ItemCategory category = ItemCategory::Harvest;
    std::uint8_t maximumStack = 0;
    AssetId sprite = 0;
};

struct ItemStack {
    ItemId item = ItemId::None;
    std::uint8_t count = 0;
};

[[nodiscard]] const ItemDefinition* FindItemDefinition(ItemId item) noexcept;

class Inventory final {
public:
    static constexpr std::size_t SlotCount = 16;
    static constexpr std::size_t HotbarSlotCount = 8;

    [[nodiscard]] std::uint16_t Add(ItemId item, std::uint16_t count) noexcept;
    [[nodiscard]] bool Remove(ItemId item, std::uint16_t count) noexcept;
    [[nodiscard]] bool Move(std::size_t from, std::size_t to) noexcept;
    [[nodiscard]] bool Exchange(std::size_t first, std::size_t second) noexcept;
    [[nodiscard]] std::uint16_t Count(ItemId item) const noexcept;
    void Clear() noexcept { slots_.fill({}); }
    [[nodiscard]] const ItemStack& Slot(std::size_t index) const noexcept;
    [[nodiscard]] ItemStack& Slot(std::size_t index) noexcept;

private:
    std::array<ItemStack, SlotCount> slots_{};
};

} // namespace Homestead

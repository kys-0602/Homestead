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
    WateringCan,
    PotatoSeed,
    Potato,
    CornSeed,
    Corn,
    CabbageSeed,
    Cabbage
};

enum class ItemCategory : std::uint8_t {
    Seed,
    Harvest,
    Tool
};

enum class ItemQuality : std::uint8_t {
    Normal,
    Silver,
    Gold,
    Count
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
    ItemQuality quality = ItemQuality::Normal;
};

[[nodiscard]] const ItemDefinition* FindItemDefinition(ItemId item) noexcept;
[[nodiscard]] bool IsValidItemQuality(ItemId item, ItemQuality quality) noexcept;

class Inventory final {
public:
    static constexpr std::size_t SlotCount = 16;
    static constexpr std::size_t HotbarSlotCount = 8;

    [[nodiscard]] std::uint16_t Add(
        ItemId item, std::uint16_t count, ItemQuality quality = ItemQuality::Normal) noexcept;
    [[nodiscard]] bool Remove(ItemId item, std::uint16_t count) noexcept;
    [[nodiscard]] bool Remove(ItemId item, std::uint16_t count, ItemQuality quality) noexcept;
    [[nodiscard]] bool Move(std::size_t from, std::size_t to) noexcept;
    [[nodiscard]] bool Exchange(std::size_t first, std::size_t second) noexcept;
    [[nodiscard]] std::uint16_t Count(ItemId item) const noexcept;
    [[nodiscard]] std::uint16_t Count(ItemId item, ItemQuality quality) const noexcept;
    void Clear() noexcept { slots_.fill({}); }
    [[nodiscard]] const ItemStack& Slot(std::size_t index) const noexcept;
    [[nodiscard]] ItemStack& Slot(std::size_t index) noexcept;

private:
    std::array<ItemStack, SlotCount> slots_{};
};

} // namespace Homestead

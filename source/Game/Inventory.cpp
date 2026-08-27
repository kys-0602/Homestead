#include "Homestead/Game/Inventory.hpp"

#include <algorithm>

namespace Homestead {
namespace {

constexpr ItemDefinition ItemDefinitions[] = {
    {ItemId::WheatSeed, ItemCategory::Seed, 99, MakeAssetId("crop.wheat.seed_bag")},
    {ItemId::Wheat, ItemCategory::Harvest, 99, MakeAssetId("crop.wheat.harvest")},
    {ItemId::CarrotSeed, ItemCategory::Seed, 99, MakeAssetId("crop.carrot.seed_bag")},
    {ItemId::Carrot, ItemCategory::Harvest, 99, MakeAssetId("crop.carrot.harvest")},
    {ItemId::TomatoSeed, ItemCategory::Seed, 99, MakeAssetId("crop.tomato.seed_bag")},
    {ItemId::Tomato, ItemCategory::Harvest, 99, MakeAssetId("crop.tomato.harvest")},
    {ItemId::PotatoSeed, ItemCategory::Seed, 99, MakeAssetId("crop.potato.seed_bag")},
    {ItemId::Potato, ItemCategory::Harvest, 99, MakeAssetId("crop.potato.harvest")},
    {ItemId::CornSeed, ItemCategory::Seed, 99, MakeAssetId("crop.corn.seed_bag")},
    {ItemId::Corn, ItemCategory::Harvest, 99, MakeAssetId("crop.corn.harvest")},
    {ItemId::CabbageSeed, ItemCategory::Seed, 99, MakeAssetId("crop.cabbage.seed_bag")},
    {ItemId::Cabbage, ItemCategory::Harvest, 99, MakeAssetId("crop.cabbage.harvest")},
    {ItemId::Hoe, ItemCategory::Tool, 1, MakeAssetId("icon.tool.hoe")},
    {ItemId::WateringCan, ItemCategory::Tool, 1, MakeAssetId("icon.tool.watering_can")}
};

constexpr ItemStack EmptyStack{};

} // namespace

const ItemDefinition* FindItemDefinition(ItemId item) noexcept {
    for (const ItemDefinition& definition : ItemDefinitions) {
        if (definition.id == item) {
            return &definition;
        }
    }
    return nullptr;
}

std::uint16_t Inventory::Add(ItemId item, std::uint16_t count) noexcept {
    const ItemDefinition* definition = FindItemDefinition(item);
    if (definition == nullptr || count == 0) {
        return count;
    }
    for (ItemStack& slot : slots_) {
        if (slot.item != item || slot.count >= definition->maximumStack) {
            continue;
        }
        const std::uint16_t available = definition->maximumStack - slot.count;
        const std::uint16_t added = std::min(count, available);
        slot.count = static_cast<std::uint8_t>(slot.count + added);
        count = static_cast<std::uint16_t>(count - added);
        if (count == 0) return 0;
    }
    for (ItemStack& slot : slots_) {
        if (slot.item != ItemId::None) continue;
        const std::uint16_t added = std::min<std::uint16_t>(count, definition->maximumStack);
        slot = {item, static_cast<std::uint8_t>(added)};
        count = static_cast<std::uint16_t>(count - added);
        if (count == 0) return 0;
    }
    return count;
}

bool Inventory::Remove(ItemId item, std::uint16_t count) noexcept {
    if (item == ItemId::None || count == 0 || Count(item) < count) return false;
    for (std::size_t index = slots_.size(); index > 0 && count > 0; --index) {
        ItemStack& slot = slots_[index - 1];
        if (slot.item != item) continue;
        const std::uint16_t removed = std::min<std::uint16_t>(count, slot.count);
        slot.count = static_cast<std::uint8_t>(slot.count - removed);
        count = static_cast<std::uint16_t>(count - removed);
        if (slot.count == 0) slot = {};
    }
    return true;
}

bool Inventory::Move(std::size_t from, std::size_t to) noexcept {
    if (from >= SlotCount || to >= SlotCount || from == to || slots_[from].item == ItemId::None) {
        return false;
    }
    if (slots_[to].item == ItemId::None) {
        slots_[to] = slots_[from];
        slots_[from] = {};
        return true;
    }
    if (slots_[to].item != slots_[from].item) return false;
    const ItemDefinition* definition = FindItemDefinition(slots_[from].item);
    if (definition == nullptr || slots_[to].count >= definition->maximumStack) return false;
    const std::uint8_t moved = static_cast<std::uint8_t>(std::min<unsigned>(
        slots_[from].count, definition->maximumStack - slots_[to].count));
    slots_[to].count = static_cast<std::uint8_t>(slots_[to].count + moved);
    slots_[from].count = static_cast<std::uint8_t>(slots_[from].count - moved);
    if (slots_[from].count == 0) slots_[from] = {};
    return true;
}

bool Inventory::Exchange(std::size_t first, std::size_t second) noexcept {
    if (first >= SlotCount || second >= SlotCount || first == second) return false;
    std::swap(slots_[first], slots_[second]);
    return true;
}

std::uint16_t Inventory::Count(ItemId item) const noexcept {
    std::uint16_t count = 0;
    for (const ItemStack& slot : slots_) if (slot.item == item) count += slot.count;
    return count;
}

const ItemStack& Inventory::Slot(std::size_t index) const noexcept {
    return index < SlotCount ? slots_[index] : EmptyStack;
}

ItemStack& Inventory::Slot(std::size_t index) noexcept {
    return slots_[index];
}

} // namespace Homestead

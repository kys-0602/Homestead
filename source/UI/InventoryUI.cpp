#include "Homestead/UI/InventoryUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/Inventory.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

namespace Homestead {
namespace {

bool AddSprite(const SpriteAsset& sprite, float x, float y, std::uint32_t color,
               std::uint16_t depth, RenderQueue& queue) noexcept {
    SpriteCommand command{};
    command.x = x + sprite.trimX;
    command.y = y + sprite.trimY;
    command.width = static_cast<float>(sprite.width);
    command.height = static_cast<float>(sprite.height);
    command.uvX = sprite.x;
    command.uvY = sprite.y;
    command.uvWidth = sprite.width;
    command.uvHeight = sprite.height;
    command.color = color;
    command.depth = depth;
    command.layer = SpriteLayer::UI;
    return queue.Add(command);
}

} // namespace

int InventorySlotAt(std::uint32_t x, std::uint32_t y, bool open) noexcept {
    const auto hitRow = [x, y](std::int32_t top, int first) noexcept {
        if (x < HotbarX || x >= HotbarX + InventorySlotSize * 8 ||
            y < static_cast<std::uint32_t>(top) || y >= static_cast<std::uint32_t>(top + InventorySlotSize)) return -1;
        return first + static_cast<int>((x - HotbarX) / InventorySlotSize);
    };
    const int hotbar = hitRow(HotbarY, 0);
    if (hotbar >= 0 || !open) return hotbar;
    return hitRow(InventoryOverlayY, 8);
}

bool AddInventoryUI(const Inventory& inventory, std::size_t selectedSlot,
                    std::size_t cursorSlot, bool open, const AssetStore& assets,
                    RenderQueue& queue) noexcept {
    const SpriteAsset* background = assets.FindSprite(MakeAssetId("terrain.grass"));
    const SpriteAsset* pointer = assets.FindSprite(MakeAssetId("ui.pointer.idle"));
    if (background == nullptr || pointer == nullptr) return false;
    const std::size_t end = open ? Inventory::SlotCount : Inventory::HotbarSlotCount;
    for (std::size_t index = 0; index < end; ++index) {
        const bool secondRow = index >= Inventory::HotbarSlotCount;
        const float x = static_cast<float>(HotbarX + static_cast<int>(index % 8) * InventorySlotSize + 1);
        const float y = static_cast<float>((secondRow ? InventoryOverlayY : HotbarY) + 1);
        std::uint32_t tint = index == selectedSlot ? 0xFFFFFFFFU : 0xFFB0B0B0U;
        if (open && index == cursorSlot) tint = 0xFF70FFFFU;
        if (!AddSprite(*background, x, y, tint, 0, queue)) return false;
        const ItemStack& stack = inventory.Slot(index);
        const ItemDefinition* definition = FindItemDefinition(stack.item);
        if (definition != nullptr) {
            const SpriteAsset* item = assets.FindSprite(definition->sprite);
            if (item == nullptr || !AddSprite(*item, x, y, 0xFFFFFFFFU, 1, queue)) return false;
        }
    }
    if (open) {
        const float x = static_cast<float>(HotbarX + static_cast<int>(cursorSlot % 8) * InventorySlotSize + 1);
        const float y = static_cast<float>((cursorSlot >= 8 ? InventoryOverlayY : HotbarY) + 1);
        if (!AddSprite(*pointer, x, y, 0xC0FFFFFFU, 2, queue)) return false;
    }
    return true;
}

} // namespace Homestead

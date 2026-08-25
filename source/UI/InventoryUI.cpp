#include "Homestead/UI/InventoryUI.hpp"

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/Inventory.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/UIPanel.hpp"

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

bool AddFrame(const SpriteAsset& sprite, float x, float y,
              std::uint16_t depth, RenderQueue& queue) noexcept {
    if (sprite.sourceWidth == 0 || sprite.sourceHeight == 0) return false;
    const float scaleX = static_cast<float>(InventoryFrameSize) /
        static_cast<float>(sprite.sourceWidth);
    const float scaleY = static_cast<float>(InventoryFrameSize) /
        static_cast<float>(sprite.sourceHeight);
    SpriteCommand command{};
    command.x=x+static_cast<float>(sprite.trimX)*scaleX;
    command.y=y+static_cast<float>(sprite.trimY)*scaleY;
    command.width=static_cast<float>(sprite.width)*scaleX;
    command.height=static_cast<float>(sprite.height)*scaleY;
    command.uvX=sprite.x; command.uvY=sprite.y;
    command.uvWidth=sprite.width; command.uvHeight=sprite.height;
    command.depth=depth; command.layer=SpriteLayer::UI;
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
    const SpriteAsset* frame = assets.FindSprite(MakeAssetId("ui.slot.frame"));
    const SpriteAsset* selected = assets.FindSprite(MakeAssetId("ui.slot.selected"));
    if (frame == nullptr || selected == nullptr) return false;
    if (open && !AddUIPanel(44.0F, 110.0F, 232.0F, 64.0F,
                            0xFFFFFFFFU, 0, assets, queue)) return false;
    const std::size_t end = open ? Inventory::SlotCount : Inventory::HotbarSlotCount;
    for (std::size_t index = 0; index < end; ++index) {
        const bool secondRow = index >= Inventory::HotbarSlotCount;
        const float slotX = static_cast<float>(HotbarX + static_cast<int>(index % 8) * InventorySlotSize);
        const float slotY = static_cast<float>(secondRow ? InventoryOverlayY : HotbarY);
        const bool highlighted = index == selectedSlot || (open && index == cursorSlot);
        if (!AddFrame(highlighted ? *selected : *frame, slotX+1.0F, slotY+1.0F, 0, queue)) return false;
        const float x = slotX + 6.0F;
        const float y = slotY + 6.0F;
        const ItemStack& stack = inventory.Slot(index);
        const ItemDefinition* definition = FindItemDefinition(stack.item);
        if (definition != nullptr) {
            const SpriteAsset* item = assets.FindSprite(definition->sprite);
            if (item == nullptr || !AddSprite(*item, x, y, 0xFFFFFFFFU, 1, queue)) return false;
            if (stack.count > 1) {
                char countText[3]{};
                const bool twoDigits = stack.count >= 10;
                countText[0] = static_cast<char>('0' + (twoDigits ? stack.count / 10 : stack.count));
                if (twoDigits) countText[1] = static_cast<char>('0' + stack.count % 10);
                const float textX = x + (twoDigits ? 5.0F : 11.0F);
                if (!AddBitmapText(countText, textX, y + 9.0F,
                                   0xFFFFFFFFU, 3, assets, queue)) return false;
            }
        }
    }
    return true;
}

} // namespace Homestead

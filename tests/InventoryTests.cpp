#include "Homestead/Game/Inventory.hpp"
#include "Homestead/UI/InventoryUI.hpp"

int main() {
    Homestead::Inventory inventory;
    if (inventory.Add(Homestead::ItemId::CarrotSeed, 120) != 0 ||
        inventory.Slot(0).count != 99 || inventory.Slot(1).count != 21) return 1;
    if (!inventory.Remove(Homestead::ItemId::CarrotSeed, 22) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 98) return 2;
    if (inventory.Remove(Homestead::ItemId::CarrotSeed, 99) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 98) return 3;
    if (inventory.Add(Homestead::ItemId::Hoe, 2) != 0 ||
        inventory.Count(Homestead::ItemId::Hoe) != 2) return 4;
    if (inventory.Add(Homestead::ItemId::Carrot, 3) != 0 ||
        !inventory.Move(3, 4) || inventory.Slot(4).count != 3 ||
        inventory.Slot(3).item != Homestead::ItemId::None) return 5;
    if (!inventory.Exchange(0, 4) || inventory.Slot(4).count != 98) return 6;

    Homestead::Inventory full;
    for (std::size_t index = 0; index < Homestead::Inventory::SlotCount; ++index) {
        full.Slot(index) = {Homestead::ItemId::Hoe, 1};
    }
    if (full.Add(Homestead::ItemId::Carrot, 3) != 3) return 7;

    Homestead::Inventory quality;
    if (quality.Add(Homestead::ItemId::Carrot, 2, Homestead::ItemQuality::Silver) != 0 ||
        quality.Add(Homestead::ItemId::Carrot, 1, Homestead::ItemQuality::Gold) != 0 ||
        quality.Count(Homestead::ItemId::Carrot) != 3 ||
        quality.Count(Homestead::ItemId::Carrot, Homestead::ItemQuality::Silver) != 2 ||
        !quality.Remove(Homestead::ItemId::Carrot, 1, Homestead::ItemQuality::Gold) ||
        quality.Count(Homestead::ItemId::Carrot) != 2) return 8;

    if (Homestead::InventorySlotAt(Homestead::HotbarX + 1,
            Homestead::HotbarY + 1, false) != 0 ||
        Homestead::InventorySlotAt(Homestead::HotbarX + 7 * Homestead::InventorySlotSize + 1,
            Homestead::HotbarY + 1, false) != 7 ||
        Homestead::InventorySlotAt(Homestead::HotbarX + 1,
            Homestead::InventoryOverlayY + 1, false) != -1 ||
        Homestead::InventorySlotAt(Homestead::HotbarX + 1,
            Homestead::InventoryOverlayY + 1, true) != 8) return 9;
    return 0;
}

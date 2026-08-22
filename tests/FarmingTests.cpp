#include "Homestead/Game/Inventory.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/CropField.hpp"
#include "Homestead/World/TileMap.hpp"

#include <cstdint>
#include <vector>

namespace {

void U16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}

void U32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    U16(bytes, static_cast<std::uint16_t>(value));
    U16(bytes, static_cast<std::uint16_t>(value >> 16U));
}

std::vector<std::uint8_t> MakeFarm() {
    std::vector<std::uint8_t> bytes{'H', 'S', 'T', 'M'};
    U16(bytes, 1); U16(bytes, 24); U16(bytes, 2); U16(bytes, 2);
    U16(bytes, 16); U16(bytes, 16); U16(bytes, 3); U16(bytes, 6); U32(bytes, 4);
    for (unsigned index = 0; index < 4; ++index) {
        U16(bytes, 1); U16(bytes, 0);
        bytes.push_back(Homestead::TileFlagValue(Homestead::TileFlag::Tilled));
        bytes.push_back(0);
    }
    return bytes;
}

} // namespace

int main() {
    const std::vector<std::uint8_t> bytes = MakeFarm();
    Homestead::TileMap map;
    Homestead::Inventory inventory;
    Homestead::CropField crops;
    if (!map.LoadMemory(bytes.data(), bytes.size()) ||
        inventory.Add(Homestead::ItemId::CarrotSeed, 4) != 0) return 1;

    const Homestead::TileSelection first{0, 0, true, true};
    const Homestead::TileSelection second{1, 0, true, true};
    if (!crops.Plant(map, inventory, first, Homestead::ItemId::CarrotSeed) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 3 || crops.Count() != 1) return 2;
    if (crops.Plant(map, inventory, first, Homestead::ItemId::CarrotSeed) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 3) return 3;
    if (crops.Plant(map, inventory, second, Homestead::ItemId::Carrot) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 3) return 4;

    crops.OnDayChanged(map);
    if (crops.Find(0, 0)->stage != 0) return 5;
    for (std::uint8_t stage = 1; stage <= 3; ++stage) {
        map.Get(0, 0)->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Watered);
        crops.OnDayChanged(map);
        if (crops.Find(0, 0)->stage != stage ||
            (map.Get(0, 0)->flags & Homestead::TileFlagValue(Homestead::TileFlag::Watered)) != 0) return 6;
    }
    map.Get(0, 0)->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Watered);
    crops.OnDayChanged(map);
    if (crops.Find(0, 0)->stage != 3) return 7;

    inventory.Clear();
    for (std::size_t index = 0; index < Homestead::Inventory::SlotCount; ++index) {
        inventory.Slot(index) = {Homestead::ItemId::Hoe, 1};
    }
    if (crops.Harvest(inventory, first) || crops.Count() != 1) return 8;
    inventory.Clear();
    if (!crops.Harvest(inventory, first) || crops.Count() != 0 ||
        inventory.Count(Homestead::ItemId::Carrot) != 1) return 9;
    if (crops.Harvest(inventory, first)) return 10;

    map.Get(1, 1)->flags = 0;
    const Homestead::TileSelection untilled{1, 1, true, true};
    if (inventory.Add(Homestead::ItemId::CarrotSeed, 1) != 0 ||
        crops.Plant(map, inventory, untilled, Homestead::ItemId::CarrotSeed) ||
        inventory.Count(Homestead::ItemId::CarrotSeed) != 1) return 11;
    return 0;
}

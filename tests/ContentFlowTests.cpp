#include "Homestead/Game/Economy.hpp"
#include "Homestead/Save/SaveCodec.hpp"
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
    U16(bytes, 1); U16(bytes, 24); U16(bytes, 6); U16(bytes, 1);
    U16(bytes, 16); U16(bytes, 16); U16(bytes, 3); U16(bytes, 6); U32(bytes, 6);
    for (unsigned index = 0; index < 6; ++index) {
        U16(bytes, 1); U16(bytes, 0);
        bytes.push_back(Homestead::TileFlagValue(Homestead::TileFlag::Tilled));
        bytes.push_back(0);
    }
    return bytes;
}

void Capture(const Homestead::Inventory& inventory, const Homestead::CropField& crops,
             std::uint16_t gold, Homestead::SaveSnapshot& snapshot) {
    snapshot = {};
    snapshot.day = 2;
    snapshot.gold = gold;
    for (std::size_t index = 0; index < Homestead::Inventory::SlotCount; ++index) {
        snapshot.inventory[index] = inventory.Slot(index);
    }
    for (std::uint8_t x = 0; x < 6; ++x) {
        snapshot.tileDeltas.push_back({x, 0, Homestead::TileFlagValue(Homestead::TileFlag::Tilled)});
    }
    for (const Homestead::CropInstance& crop : crops.Crops()) {
        if (crop.active) snapshot.crops.push_back(crop);
    }
}

} // namespace

int main() {
    const std::vector<std::uint8_t> mapBytes = MakeFarm();
    Homestead::TileMap map;
    Homestead::Inventory inventory;
    Homestead::CropField crops;
    if (!map.LoadMemory(mapBytes.data(), mapBytes.size()) ||
        inventory.Add(Homestead::ItemId::Hoe, 1) != 0 ||
        inventory.Add(Homestead::ItemId::WateringCan, 1) != 0 ||
        inventory.Count(Homestead::ItemId::Carrot) != 0) return 1;

    std::uint16_t gold=100;
    for(std::size_t crop=0;crop<Homestead::MarketCropCount;++crop)
        if(!Homestead::BuySeed(inventory,gold,crop))return 2;
    constexpr Homestead::ItemId seeds[]{Homestead::ItemId::WheatSeed, Homestead::ItemId::CarrotSeed,
        Homestead::ItemId::TomatoSeed, Homestead::ItemId::PotatoSeed, Homestead::ItemId::CornSeed,
        Homestead::ItemId::CabbageSeed};

    for (std::int32_t x = 0; x < 6; ++x) {
        if (!crops.Plant(map, inventory, {x, 0, true, true},
                         seeds[x])) return 2;
        map.Get(x, 0)->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Watered);
    }
    crops.OnDayChanged(map);

    Homestead::SaveSnapshot saved;
    Capture(inventory, crops, gold, saved);
    std::vector<std::uint8_t> encoded;
    Homestead::SaveSnapshot loaded;
    if (!Homestead::EncodeSave(saved, encoded) ||
        !Homestead::DecodeSave(encoded.data(), encoded.size(), loaded)) return 3;

    Homestead::TileMap restoredMap;
    Homestead::Inventory restoredInventory;
    Homestead::CropField restoredCrops;
    if (!restoredMap.LoadMemory(mapBytes.data(), mapBytes.size())) return 4;
    for (std::size_t index = 0; index < Homestead::Inventory::SlotCount; ++index) {
        restoredInventory.Slot(index) = loaded.inventory[index];
    }
    for (const Homestead::CropInstance& crop : loaded.crops) {
        if (!restoredCrops.Restore(crop, restoredMap)) return 5;
    }

    for (unsigned day = 0; day < 5; ++day) {
        for (std::int32_t x = 0; x < 6; ++x) {
            restoredMap.Get(x, 0)->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Watered);
        }
        restoredCrops.OnDayChanged(restoredMap);
    }
    for (std::int32_t x = 0; x < 6; ++x) {
        if (!restoredCrops.Harvest(restoredInventory, {x, 0, true, true})) return 6;
    }
    gold=loaded.gold;
    for(std::size_t crop=0;crop<Homestead::MarketCropCount;++crop)
        if(!Homestead::SellHarvest(restoredInventory,gold,crop))return 7;
    if (restoredCrops.Count() != 0 || gold!=152) return 8;
    return 0;
}

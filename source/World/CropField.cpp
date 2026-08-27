#include "Homestead/World/CropField.hpp"

#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

constexpr CropDefinition CropDefinitions[]{
    {CropId::Wheat, ItemId::WheatSeed, ItemId::Wheat, 3, 2,
     {MakeAssetId("crop.wheat.stage_0"), MakeAssetId("crop.wheat.stage_1"),
      MakeAssetId("crop.wheat.stage_2"), MakeAssetId("crop.wheat.stage_3")}},
    {CropId::Carrot, ItemId::CarrotSeed, ItemId::Carrot, 3, 3,
    {MakeAssetId("crop.carrot.stage_0"), MakeAssetId("crop.carrot.stage_1"),
     MakeAssetId("crop.carrot.stage_2"), MakeAssetId("crop.carrot.stage_3")}},
    {CropId::Tomato, ItemId::TomatoSeed, ItemId::Tomato, 3, 4,
     {MakeAssetId("crop.tomato.stage_0"), MakeAssetId("crop.tomato.stage_1"),
      MakeAssetId("crop.tomato.stage_2"), MakeAssetId("crop.tomato.stage_3")}},
    {CropId::Potato, ItemId::PotatoSeed, ItemId::Potato, 3, 2,
     {MakeAssetId("crop.potato.stage_0"), MakeAssetId("crop.potato.stage_1"),
      MakeAssetId("crop.potato.stage_2"), MakeAssetId("crop.potato.stage_3")}},
    {CropId::Corn, ItemId::CornSeed, ItemId::Corn, 3, 3,
     {MakeAssetId("crop.corn.stage_0"), MakeAssetId("crop.corn.stage_1"),
      MakeAssetId("crop.corn.stage_2"), MakeAssetId("crop.corn.stage_3")}},
    {CropId::Cabbage, ItemId::CabbageSeed, ItemId::Cabbage, 3, 5,
     {MakeAssetId("crop.cabbage.stage_0"), MakeAssetId("crop.cabbage.stage_1"),
      MakeAssetId("crop.cabbage.stage_2"), MakeAssetId("crop.cabbage.stage_3")}}};

} // namespace

const CropDefinition* FindCropDefinition(CropId crop) noexcept {
    for (const CropDefinition& definition : CropDefinitions)
        if (definition.id == crop) return &definition;
    return nullptr;
}

bool CropField::Plant(TileMap& map, Inventory& inventory,
                      const TileSelection& selection, ItemId seedItem) noexcept {
    if (!selection.valid || !selection.inRange || count_ >= Capacity ||
        Find(selection.x, selection.y) != nullptr) {
        return false;
    }
    const CropDefinition* selected = nullptr;
    for (const CropDefinition& definition : CropDefinitions)
        if (definition.seedItem == seedItem) selected = &definition;
    if (selected == nullptr) return false;
    Tile* tile = map.Get(selection.x, selection.y);
    if (tile == nullptr || (tile->flags & TileFlagValue(TileFlag::Tilled)) == 0 ||
        tile->object != 0 || inventory.Count(seedItem) == 0) {
        return false;
    }
    CropInstance* empty = nullptr;
    for (CropInstance& crop : crops_) {
        if (!crop.active) {
            empty = &crop;
            break;
        }
    }
    if (empty == nullptr || !inventory.Remove(seedItem, 1)) return false;
    *empty = {static_cast<std::int16_t>(selection.x), static_cast<std::int16_t>(selection.y),
              selected->id, 0, 0, true};
    ++count_;
    return true;
}

bool CropField::Harvest(Inventory& inventory, const TileSelection& selection) noexcept {
    if (!selection.valid || !selection.inRange) return false;
    CropInstance* crop = FindMutable(selection.x, selection.y);
    if (crop == nullptr) return false;
    const CropDefinition* definition = FindCropDefinition(crop->crop);
    if (definition == nullptr || crop->stage < definition->finalStage) return false;
    if (inventory.Add(definition->harvestItem, 1) != 0) return false;
    *crop = {};
    --count_;
    return true;
}

void CropField::OnDayChanged(TileMap& map) noexcept {
    for (CropInstance& crop : crops_) {
        if (!crop.active) continue;
        Tile* tile = map.Get(crop.tileX, crop.tileY);
        const CropDefinition* definition = FindCropDefinition(crop.crop);
        if (tile != nullptr && definition != nullptr &&
            (tile->flags & TileFlagValue(TileFlag::Watered)) != 0 &&
            crop.stage < definition->finalStage) {
            ++crop.wateredDays;
            crop.stage = static_cast<std::uint8_t>(
                (static_cast<unsigned>(crop.wateredDays) * definition->finalStage) /
                definition->growthDays);
            if (crop.stage > definition->finalStage) crop.stage = definition->finalStage;
        }
    }
    for (std::int32_t y = 0; y < map.Height(); ++y) {
        for (std::int32_t x = 0; x < map.Width(); ++x) {
            Tile* tile = map.Get(x, y);
            if (tile != nullptr) {
                tile->flags &= static_cast<std::uint8_t>(~TileFlagValue(TileFlag::Watered));
            }
        }
    }
}

void CropField::Clear() noexcept {
    crops_.fill({});
    count_ = 0;
}

bool CropField::Restore(const CropInstance& crop, const TileMap& map) noexcept {
    const CropDefinition* definition = FindCropDefinition(crop.crop);
    const Tile* tile = map.Get(crop.tileX, crop.tileY);
    if (!crop.active || definition == nullptr || crop.stage > definition->finalStage ||
        crop.wateredDays > definition->growthDays ||
        tile == nullptr || (tile->flags & TileFlagValue(TileFlag::Tilled)) == 0 ||
        tile->object != 0 || count_ >= Capacity || Find(crop.tileX, crop.tileY) != nullptr) return false;
    for (CropInstance& slot : crops_) {
        if (!slot.active) { slot = crop; ++count_; return true; }
    }
    return false;
}

const CropInstance* CropField::Find(std::int32_t x, std::int32_t y) const noexcept {
    for (const CropInstance& crop : crops_) {
        if (crop.active && crop.tileX == x && crop.tileY == y) return &crop;
    }
    return nullptr;
}

CropInstance* CropField::FindMutable(std::int32_t x, std::int32_t y) noexcept {
    for (CropInstance& crop : crops_) {
        if (crop.active && crop.tileX == x && crop.tileY == y) return &crop;
    }
    return nullptr;
}

} // namespace Homestead

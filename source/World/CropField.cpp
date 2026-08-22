#include "Homestead/World/CropField.hpp"

#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

constexpr CropDefinition CarrotDefinition{
    CropId::Carrot,
    ItemId::CarrotSeed,
    ItemId::Carrot,
    3,
    {MakeAssetId("crop.carrot.stage_0"), MakeAssetId("crop.carrot.stage_1"),
     MakeAssetId("crop.carrot.stage_2"), MakeAssetId("crop.carrot.stage_3")}};

} // namespace

const CropDefinition* FindCropDefinition(CropId crop) noexcept {
    return crop == CropId::Carrot ? &CarrotDefinition : nullptr;
}

bool CropField::Plant(TileMap& map, Inventory& inventory,
                      const TileSelection& selection, ItemId seedItem) noexcept {
    if (!selection.valid || !selection.inRange || count_ >= Capacity ||
        seedItem != CarrotDefinition.seedItem || Find(selection.x, selection.y) != nullptr) {
        return false;
    }
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
              CropId::Carrot, 0, true};
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
            ++crop.stage;
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

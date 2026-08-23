#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/Inventory.hpp"

namespace Homestead {

class Inventory;
class TileMap;
struct TileSelection;

enum class CropId : std::uint8_t {
    None,
    Wheat,
    Carrot,
    Tomato
};

struct CropDefinition {
    CropId id = CropId::None;
    ItemId seedItem = ItemId::None;
    ItemId harvestItem = ItemId::None;
    std::uint8_t finalStage = 0;
    std::uint8_t growthDays = 0;
    std::array<AssetId, 4> stageSprites{};
};

struct CropInstance {
    std::int16_t tileX = 0;
    std::int16_t tileY = 0;
    CropId crop = CropId::None;
    std::uint8_t stage = 0;
    std::uint8_t wateredDays = 0;
    bool active = false;
};

[[nodiscard]] const CropDefinition* FindCropDefinition(CropId crop) noexcept;

class CropField final {
public:
    static constexpr std::size_t Capacity = 256;

    [[nodiscard]] bool Plant(
        TileMap& map,
        Inventory& inventory,
        const TileSelection& selection,
        ItemId seedItem) noexcept;
    [[nodiscard]] bool Harvest(
        Inventory& inventory,
        const TileSelection& selection) noexcept;
    void OnDayChanged(TileMap& map) noexcept;
    void Clear() noexcept;
    [[nodiscard]] bool Restore(const CropInstance& crop, const TileMap& map) noexcept;

    [[nodiscard]] const CropInstance* Find(std::int32_t x, std::int32_t y) const noexcept;
    [[nodiscard]] const std::array<CropInstance, Capacity>& Crops() const noexcept { return crops_; }
    [[nodiscard]] std::size_t Count() const noexcept { return count_; }

private:
    [[nodiscard]] CropInstance* FindMutable(std::int32_t x, std::int32_t y) noexcept;

    std::array<CropInstance, Capacity> crops_{};
    std::size_t count_ = 0;
};

} // namespace Homestead

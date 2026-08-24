#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "Homestead/Game/Inventory.hpp"
#include "Homestead/World/CropField.hpp"
#include "Homestead/World/MapId.hpp"

namespace Homestead {

inline constexpr std::size_t MaximumSaveBytes = 32U * 1024U;
inline constexpr std::size_t MaximumTileDeltas = 4096;

struct SavedTileDelta {
    std::uint8_t x = 0;
    std::uint8_t y = 0;
    std::uint8_t flags = 0;
};

struct SaveSnapshot {
    std::int32_t playerX256 = 0;
    std::int32_t playerY256 = 0;
    std::uint16_t day = 1;
    std::uint16_t minute = 360;
    std::uint8_t selectedSlot = 0;
    MapId mapId = MapId::Farm;
    std::uint16_t gold = 20;
    std::array<ItemStack, Inventory::SlotCount> inventory{};
    std::vector<SavedTileDelta> tileDeltas;
    std::vector<CropInstance> crops;
};

[[nodiscard]] bool EncodeSave(
    const SaveSnapshot& snapshot,
    std::vector<std::uint8_t>& bytes) noexcept;
[[nodiscard]] bool DecodeSave(
    const std::uint8_t* bytes,
    std::size_t size,
    SaveSnapshot& snapshot) noexcept;

} // namespace Homestead

#pragma once

#include <cstdint>

#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/World/EntityWorld.hpp"

namespace Homestead {

class TileMap;

inline constexpr float MaximumToolRange = 32.0F;

struct TileSelection {
    std::int32_t x = 0;
    std::int32_t y = 0;
    bool valid = false;
    bool inRange = false;
};

[[nodiscard]] TileSelection SelectFrontTile(
    WorldPosition playerFeet,
    FacingDirection facing,
    const TileMap& map) noexcept;

[[nodiscard]] TileSelection SelectMouseTile(
    WorldPosition playerFeet,
    WorldPosition mouseWorld,
    const TileMap& map) noexcept;

[[nodiscard]] bool TryInteract(
    PlayerState& player,
    const TileMap& map,
    const TileSelection& selection) noexcept;

void FaceSelection(
    PlayerState& player,
    WorldPosition playerFeet,
    const TileSelection& selection) noexcept;

} // namespace Homestead

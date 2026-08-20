#pragma once

namespace Homestead {

class EntityWorld;
class TileMap;
struct PlayerState;

struct MovementInput {
    float x = 0.0F;
    float y = 0.0F;
};

[[nodiscard]] bool UpdatePlayerMovement(
    EntityWorld& world,
    PlayerState& player,
    const TileMap& map,
    MovementInput input,
    float stepSeconds) noexcept;

} // namespace Homestead

#pragma once

namespace Homestead {

class EntityWorld;
class TileMap;
struct PlayerState;
struct TileSelection;

inline constexpr unsigned ToolActionTicks = 36;
inline constexpr unsigned ToolImpactTick = 18;

[[nodiscard]] bool TryStartToolUse(
    PlayerState& player,
    const TileMap& map,
    const TileSelection& selection) noexcept;

[[nodiscard]] bool UpdateToolUse(
    EntityWorld& world,
    PlayerState& player,
    TileMap& map) noexcept;

} // namespace Homestead

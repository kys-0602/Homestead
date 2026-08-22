#pragma once

#include <cstdint>

namespace Homestead {

class EntityWorld;
class TileMap;
struct PlayerState;
struct TileSelection;
enum class ToolAction : std::uint8_t;

inline constexpr unsigned ToolActionTicks = 36;
inline constexpr unsigned ToolImpactTick = 18;

[[nodiscard]] bool TryStartToolUse(
    PlayerState& player,
    const TileMap& map,
    const TileSelection& selection,
    ToolAction action) noexcept;

[[nodiscard]] bool UpdateToolUse(
    EntityWorld& world,
    PlayerState& player,
    TileMap& map) noexcept;

} // namespace Homestead

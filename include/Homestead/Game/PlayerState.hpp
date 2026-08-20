#pragma once

#include <cstdint>

#include "Homestead/World/EntityWorld.hpp"

namespace Homestead {

// The selected 64x64 player frames place the movement-frame feet on source row 41.
inline constexpr float PlayerSpriteFootY = 41.0F;

enum class FacingDirection : std::uint8_t {
    Down,
    Right,
    Up,
    Left
};

enum class ToolAction : std::uint8_t {
    None,
    Hoe,
    Watering
};

struct ToolUseState {
    ToolAction action = ToolAction::None;
    std::int16_t tileX = 0;
    std::int16_t tileY = 0;
    std::uint8_t elapsedTicks = 0;
    bool applied = false;
};

struct PlayerState {
    EntityId entity{};
    float movementSpeed = 60.0F;
    FacingDirection facing = FacingDirection::Down;
    std::uint16_t animationTicks = 0;
    ToolUseState toolUse{};
    std::int16_t lastInteractionX = -1;
    std::int16_t lastInteractionY = -1;
    std::uint16_t interactionCount = 0;
    bool moving = false;
};

} // namespace Homestead

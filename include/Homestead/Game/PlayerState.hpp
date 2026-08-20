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

struct PlayerState {
    EntityId entity{};
    float movementSpeed = 60.0F;
    FacingDirection facing = FacingDirection::Down;
    std::uint16_t animationTicks = 0;
    bool moving = false;
};

} // namespace Homestead

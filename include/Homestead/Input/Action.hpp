#pragma once

#include <cstdint>

namespace Homestead {

enum class Action : std::uint8_t {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Interact,
    UseTool,
    Menu,
    Count
};

enum class PhysicalKey : std::uint8_t {
    W,
    A,
    S,
    D,
    Up,
    Down,
    Left,
    Right,
    E,
    Space,
    F,
    Escape,
    MouseLeft,
    MouseRight,
    Count
};

} // namespace Homestead

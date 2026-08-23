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
    Inventory,
    Market,
    EndDay,
    Hotbar1,
    Hotbar2,
    Hotbar3,
    Hotbar4,
    Hotbar5,
    Hotbar6,
    Hotbar7,
    Hotbar8,
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
    I,
    M,
    Escape,
    N,
    Digit1,
    Digit2,
    Digit3,
    Digit4,
    Digit5,
    Digit6,
    Digit7,
    Digit8,
    MouseLeft,
    MouseRight,
    Count
};

} // namespace Homestead

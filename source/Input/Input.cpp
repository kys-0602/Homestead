#include "Homestead/Input/Input.hpp"

#include <algorithm>
#include <cstddef>

namespace Homestead {

void Input::BeginFrame() noexcept {
    pressed_.fill(false);
    released_.fill(false);
}

void Input::SetPhysicalKey(PhysicalKey key, bool held) noexcept {
    if (key == PhysicalKey::Count) {
        return;
    }

    const std::size_t keyIndex = ToIndex(key);
    if (keyHeld_[keyIndex] == held) {
        return;
    }

    const Action action = MapAction(key);
    const bool wasHeld = Held(action);
    keyHeld_[keyIndex] = held;
    const bool isHeld = Held(action);
    const std::size_t actionIndex = ToIndex(action);

    if (!wasHeld && isHeld) {
        pressed_[actionIndex] = true;
        pendingPressed_[actionIndex] = true;
    } else if (wasHeld && !isHeld) {
        released_[actionIndex] = true;
    }
}

void Input::OnFocusLost() noexcept {
    std::array<bool, ActionCount> heldActions{};
    for (std::size_t index = 0; index < ActionCount; ++index) {
        heldActions[index] = Held(static_cast<Action>(index));
    }

    keyHeld_.fill(false);
    pressed_.fill(false);
    pendingPressed_.fill(false);
    clientMouseX_ = -1;
    clientMouseY_ = -1;
    logicalMouseValid_ = false;
    for (std::size_t index = 0; index < ActionCount; ++index) {
        if (heldActions[index]) {
            released_[index] = true;
        }
    }
}

bool Input::Held(Action action) const noexcept {
    if (action == Action::Count) {
        return false;
    }

    for (std::size_t index = 0; index < KeyCount; ++index) {
        if (keyHeld_[index] && MapAction(static_cast<PhysicalKey>(index)) == action) {
            return true;
        }
    }
    return false;
}

bool Input::Pressed(Action action) const noexcept {
    return action != Action::Count && pressed_[ToIndex(action)];
}

bool Input::Released(Action action) const noexcept {
    return action != Action::Count && released_[ToIndex(action)];
}

bool Input::ConsumePressed(Action action) noexcept {
    if (action == Action::Count) {
        return false;
    }

    const std::size_t index = ToIndex(action);
    const bool pressed = pendingPressed_[index];
    pendingPressed_[index] = false;
    return pressed;
}

void Input::SetClientMouse(std::int32_t x, std::int32_t y) noexcept {
    clientMouseX_ = x;
    clientMouseY_ = y;
}

void Input::SetLogicalMouse(std::uint32_t x, std::uint32_t y, bool valid) noexcept {
    logicalMouseX_ = x;
    logicalMouseY_ = y;
    logicalMouseValid_ = valid;
}

Action Input::MapAction(PhysicalKey key) noexcept {
    switch (key) {
    case PhysicalKey::W:
    case PhysicalKey::Up:
        return Action::MoveUp;
    case PhysicalKey::S:
    case PhysicalKey::Down:
        return Action::MoveDown;
    case PhysicalKey::A:
    case PhysicalKey::Left:
        return Action::MoveLeft;
    case PhysicalKey::D:
    case PhysicalKey::Right:
        return Action::MoveRight;
    case PhysicalKey::E:
    case PhysicalKey::Space:
    case PhysicalKey::MouseRight:
        return Action::Interact;
    case PhysicalKey::F:
    case PhysicalKey::MouseLeft:
        return Action::UseTool;
    case PhysicalKey::Escape:
        return Action::Menu;
    case PhysicalKey::Count:
        return Action::Count;
    }
    return Action::Count;
}

std::size_t Input::ToIndex(Action action) noexcept {
    return static_cast<std::size_t>(action);
}

std::size_t Input::ToIndex(PhysicalKey key) noexcept {
    return static_cast<std::size_t>(key);
}

} // namespace Homestead

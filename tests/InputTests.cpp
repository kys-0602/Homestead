#include "Homestead/Input/Input.hpp"

int main() {
    Homestead::Input input;
    input.BeginFrame();
    input.SetPhysicalKey(Homestead::PhysicalKey::W, true);
    if (!input.Held(Homestead::Action::MoveUp) ||
        !input.Pressed(Homestead::Action::MoveUp) ||
        !input.ConsumePressed(Homestead::Action::MoveUp) ||
        input.ConsumePressed(Homestead::Action::MoveUp)) {
        return 1;
    }

    input.BeginFrame();
    input.SetPhysicalKey(Homestead::PhysicalKey::W, true);
    if (!input.Held(Homestead::Action::MoveUp) ||
        input.Pressed(Homestead::Action::MoveUp)) {
        return 2;
    }

    input.SetPhysicalKey(Homestead::PhysicalKey::Up, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::W, false);
    if (!input.Held(Homestead::Action::MoveUp) ||
        input.Released(Homestead::Action::MoveUp)) {
        return 3;
    }

    input.SetPhysicalKey(Homestead::PhysicalKey::Up, false);
    if (input.Held(Homestead::Action::MoveUp) ||
        !input.Released(Homestead::Action::MoveUp)) {
        return 4;
    }

    input.BeginFrame();
    input.SetPhysicalKey(Homestead::PhysicalKey::E, true);
    if (!input.Pressed(Homestead::Action::Interact)) {
        return 5;
    }
    input.BeginFrame();
    if (input.Pressed(Homestead::Action::Interact) ||
        !input.ConsumePressed(Homestead::Action::Interact)) {
        return 6;
    }

    input.SetPhysicalKey(Homestead::PhysicalKey::MouseLeft, true);
    if (!input.Held(Homestead::Action::UseTool)) {
        return 7;
    }
    input.SetPhysicalKey(Homestead::PhysicalKey::MouseLeft, false);
    Homestead::PhysicalKey pressSource = Homestead::PhysicalKey::Count;
    if (!input.ConsumePressed(Homestead::Action::UseTool, pressSource) ||
        pressSource != Homestead::PhysicalKey::MouseLeft) {
        return 8;
    }

    input.SetPhysicalKey(Homestead::PhysicalKey::Digit8, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::Digit8, false);
    if (!input.ConsumePressed(Homestead::Action::Hotbar8)) {
        return 9;
    }

    input.SetPhysicalKey(Homestead::PhysicalKey::Escape, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::Escape, false);
    input.SetPhysicalKey(Homestead::PhysicalKey::I, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::I, false);
    if (!input.ConsumePressed(Homestead::Action::Menu) ||
        !input.ConsumePressed(Homestead::Action::Inventory)) return 12;
    input.SetPhysicalKey(Homestead::PhysicalKey::M, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::M, false);
    if (!input.ConsumePressed(Homestead::Action::Market)) return 14;

    input.SetClientMouse(120, 80);
    input.SetLogicalMouse(30, 20, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::D, true);
    input.OnFocusLost();
    if (input.Held(Homestead::Action::MoveRight) ||
        input.Held(Homestead::Action::UseTool) ||
        input.ConsumePressed(Homestead::Action::MoveRight) ||
        input.IsLogicalMouseValid() ||
        !input.Released(Homestead::Action::MoveRight)) {
        return 13;
    }

    return 0;
}

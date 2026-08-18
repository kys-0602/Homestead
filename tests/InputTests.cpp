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

    input.SetClientMouse(120, 80);
    input.SetLogicalMouse(30, 20, true);
    input.SetPhysicalKey(Homestead::PhysicalKey::D, true);
    input.OnFocusLost();
    if (input.Held(Homestead::Action::MoveRight) ||
        input.Held(Homestead::Action::UseTool) ||
        input.ConsumePressed(Homestead::Action::MoveRight) ||
        input.IsLogicalMouseValid() ||
        !input.Released(Homestead::Action::MoveRight)) {
        return 8;
    }

    return 0;
}

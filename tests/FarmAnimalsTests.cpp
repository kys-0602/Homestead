#include "Homestead/World/FarmAnimals.hpp"

#include <cmath>

int main() {
    Homestead::FarmAnimals animals;
    animals.Reset();
    const auto initial = animals.Chickens();
    if (initial.size() != Homestead::FarmAnimals::ChickenCount || initial[0].routeIndex != 1 ||
        initial[0].current.x != initial[0].previous.x || initial[0].current.y != initial[0].previous.y ||
        animals.Frogs().size() != Homestead::FarmAnimals::FrogCount) return 1;
    for (int tick = 0; tick < 90; ++tick) animals.FixedUpdate();
    const auto& updated = animals.Chickens();
    if (updated[0].current.x <= initial[0].current.x || !updated[0].moving) return 2;
    if (updated[1].current.x == initial[1].current.x && updated[1].current.y == initial[1].current.y) return 3;
    animals.Reset();
    const auto& reset = animals.Chickens();
    if (std::fabs(reset[2].current.x - initial[2].current.x) > 0.001F ||
        std::fabs(reset[2].current.y - initial[2].current.y) > 0.001F) return 4;
    const float blockedStartX = reset[0].current.x;
    animals.FixedUpdate({302.0F, 200.0F});
    const Homestead::FarmAnimal& blocked = animals.Chickens()[0];
    if (blocked.moving || blocked.current.x != blockedStartX || blocked.pauseTicks == 0) return 5;
    Homestead::FarmAnimals facingAnimals;
    facingAnimals.Reset();
    for (int tick = 0; tick < 60; ++tick) facingAnimals.FixedUpdate();
    if (!facingAnimals.Frogs()[0].facingRight || facingAnimals.Frogs()[0].flipWhenFacingRight) return 6;
    if (facingAnimals.Frogs()[0].collisionTop != -22.0F ||
        facingAnimals.Frogs()[0].collisionBottom != -14.0F) return 7;
    return 0;
}

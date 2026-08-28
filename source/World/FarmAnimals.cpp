#include "Homestead/World/FarmAnimals.hpp"

#include <array>
#include <cmath>

namespace Homestead {
namespace {

constexpr float WalkSpeed = 18.0F / 60.0F;
constexpr std::uint16_t PauseDuration = 45;
using Route = std::array<WorldPosition, 4>;

constexpr std::array<Route, FarmAnimals::ChickenCount> ChickenRoutes{{
    {{{18.5F * 16.0F, 12.5F * 16.0F}, {24.5F * 16.0F, 12.5F * 16.0F},
      {25.5F * 16.0F, 14.5F * 16.0F}, {17.5F * 16.0F, 14.5F * 16.0F}}},
    {{{24.5F * 16.0F, 21.5F * 16.0F}, {28.5F * 16.0F, 21.5F * 16.0F},
      {28.5F * 16.0F, 16.5F * 16.0F}, {24.5F * 16.0F, 16.5F * 16.0F}}},
    {{{12.5F * 16.0F, 13.5F * 16.0F}, {15.5F * 16.0F, 13.5F * 16.0F},
      {15.5F * 16.0F, 21.5F * 16.0F}, {12.5F * 16.0F, 21.5F * 16.0F}}}
}};
constexpr std::array<Route, FarmAnimals::FrogCount> FrogRoutes{{
    {{{25.5F * 16.0F, 21.5F * 16.0F}, {28.0F * 16.0F, 21.5F * 16.0F},
      {28.0F * 16.0F, 22.5F * 16.0F}, {25.5F * 16.0F, 22.5F * 16.0F}}}
}};
template <std::size_t Count>
void ResetAnimals(std::array<FarmAnimal, Count>& animals,
                  const std::array<Route, Count>& routes,
                  std::uint16_t pauseOffset, float halfWidth, float top,
                  float bottom, bool flipWhenFacingRight) noexcept {
    for (std::size_t index = 0; index < animals.size(); ++index) {
        FarmAnimal& animal = animals[index];
        animal = {};
        animal.current = routes[index][0];
        animal.previous = animal.current;
        animal.routeIndex = 1;
        animal.pauseTicks = static_cast<std::uint16_t>(pauseOffset + index * 15U);
        animal.collisionHalfWidth = halfWidth;
        animal.collisionTop = top;
        animal.collisionBottom = bottom;
        animal.flipWhenFacingRight = flipWhenFacingRight;
    }
}

template <std::size_t Count>
void UpdateAnimals(std::array<FarmAnimal, Count>& animals,
                   const std::array<Route, Count>& routes,
                   float speed, std::uint16_t pauseDuration, WorldPosition playerFeet) noexcept {
    for (std::size_t index = 0; index < animals.size(); ++index) {
        FarmAnimal& animal = animals[index];
        animal.previous = animal.current;
        animal.moving = false;
        if (animal.pauseTicks != 0) {
            --animal.pauseTicks;
            continue;
        }

        const WorldPosition target = routes[index][animal.routeIndex];
        const float dx = target.x - animal.current.x;
        const float dy = target.y - animal.current.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= speed) {
            animal.current = target;
            animal.routeIndex = static_cast<std::uint8_t>(
                (animal.routeIndex + 1U) % routes[index].size());
            animal.pauseTicks = pauseDuration;
            continue;
        }
        const WorldPosition next{
            animal.current.x + dx * speed / distance, animal.current.y + dy * speed / distance};
        const bool overlapsPlayer = next.x + animal.collisionHalfWidth > playerFeet.x - 5.0F &&
            next.x - animal.collisionHalfWidth < playerFeet.x + 5.0F &&
            next.y + animal.collisionBottom > playerFeet.y - 6.0F &&
            next.y + animal.collisionTop < playerFeet.y;
        if (overlapsPlayer) {
            animal.pauseTicks = 15;
            continue;
        }
        animal.current = next;
        if (std::fabs(dx) > 0.001F) animal.facingRight = dx > 0.0F;
        animal.moving = true;
    }
}

} // namespace

void FarmAnimals::Reset() noexcept {
    ResetAnimals(chickens_, ChickenRoutes, 0, 6.0F, -8.0F, 0.0F, true);
    ResetAnimals(frogs_, FrogRoutes, 20, 4.0F, -22.0F, -14.0F, false);
}

void FarmAnimals::FixedUpdate(WorldPosition playerFeet) noexcept {
    UpdateAnimals(chickens_, ChickenRoutes, WalkSpeed, PauseDuration, playerFeet);
    UpdateAnimals(frogs_, FrogRoutes, WalkSpeed * 0.5F,
                  static_cast<std::uint16_t>(PauseDuration * 2U), playerFeet);
}

} // namespace Homestead

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/World/EntityWorld.hpp"

namespace Homestead {

struct FarmAnimal {
    WorldPosition previous{};
    WorldPosition current{};
    std::uint8_t routeIndex = 0;
    std::uint16_t pauseTicks = 0;
    float collisionHalfWidth = 0.0F;
    float collisionTop = 0.0F;
    float collisionBottom = 0.0F;
    bool flipWhenFacingRight = true;
    bool facingRight = false;
    bool moving = false;
};

class FarmAnimals final {
public:
    static constexpr std::size_t ChickenCount = 3;
    static constexpr std::size_t FrogCount = 1;

    void Reset() noexcept;
    void FixedUpdate(WorldPosition playerFeet = {}) noexcept;

    [[nodiscard]] const std::array<FarmAnimal, ChickenCount>& Chickens() const noexcept {
        return chickens_;
    }
    [[nodiscard]] const std::array<FarmAnimal, FrogCount>& Frogs() const noexcept {
        return frogs_;
    }
private:
    std::array<FarmAnimal, ChickenCount> chickens_{};
    std::array<FarmAnimal, FrogCount> frogs_{};
};

} // namespace Homestead

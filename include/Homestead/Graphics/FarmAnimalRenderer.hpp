#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class Camera2D;
class FarmAnimals;
class RenderQueue;

[[nodiscard]] bool AddFarmAnimals(
    const FarmAnimals& animals,
    std::uint32_t animationTicks,
    float interpolationAlpha,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

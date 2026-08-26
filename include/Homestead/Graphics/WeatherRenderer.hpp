#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class Camera2D;
class RenderQueue;

[[nodiscard]] bool AddCloudShadows(
    std::uint32_t animationTicks,
    const Camera2D& camera,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

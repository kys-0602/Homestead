#pragma once

namespace Homestead {

class AssetStore;
class Camera2D;
class EntityWorld;
class RenderQueue;
struct PlayerState;

class PlayerRenderer final {
public:
    [[nodiscard]] static bool Add(
        const EntityWorld& world,
        const PlayerState& player,
        float interpolationAlpha,
        const Camera2D& camera,
        const AssetStore& assets,
        RenderQueue& queue) noexcept;
};

} // namespace Homestead

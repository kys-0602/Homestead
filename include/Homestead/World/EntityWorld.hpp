#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"

namespace Homestead {

struct WorldPosition {
    float x = 0.0F;
    float y = 0.0F;
};

struct EntityId {
    std::uint16_t index = UINT16_MAX;
    std::uint16_t generation = 0;
};

struct TransformComponent {
    WorldPosition previous{};
    WorldPosition current{};
};

struct SpriteComponent {
    AssetId asset = 0;
};

struct ColliderComponent {
    float left = -5.0F;
    float top = -6.0F;
    float right = 5.0F;
    float bottom = 0.0F;
};

class EntityWorld final {
public:
    static constexpr std::size_t Capacity = 16;

    [[nodiscard]] EntityId Create(
        WorldPosition position,
        AssetId sprite,
        ColliderComponent collider = {}) noexcept;
    [[nodiscard]] bool IsAlive(EntityId id) const noexcept;
    [[nodiscard]] TransformComponent* Transform(EntityId id) noexcept;
    [[nodiscard]] const TransformComponent* Transform(EntityId id) const noexcept;
    [[nodiscard]] SpriteComponent* Sprite(EntityId id) noexcept;
    [[nodiscard]] const SpriteComponent* Sprite(EntityId id) const noexcept;
    [[nodiscard]] const ColliderComponent* Collider(EntityId id) const noexcept;
    void Clear() noexcept;

private:
    std::array<TransformComponent, Capacity> transforms_{};
    std::array<SpriteComponent, Capacity> sprites_{};
    std::array<ColliderComponent, Capacity> colliders_{};
    std::array<std::uint16_t, Capacity> generations_{};
    std::array<bool, Capacity> active_{};
};

} // namespace Homestead

#include "Homestead/World/EntityWorld.hpp"

namespace Homestead {

EntityId EntityWorld::Create(
    WorldPosition position,
    AssetId sprite,
    ColliderComponent collider) noexcept {
    for (std::size_t index = 0; index < Capacity; ++index) {
        if (!active_[index]) {
            active_[index] = true;
            ++generations_[index];
            if (generations_[index] == 0) {
                ++generations_[index];
            }
            transforms_[index] = {position, position};
            sprites_[index] = {sprite};
            colliders_[index] = collider;
            return {static_cast<std::uint16_t>(index), generations_[index]};
        }
    }
    return {};
}

bool EntityWorld::IsAlive(EntityId id) const noexcept {
    return id.index < Capacity && active_[id.index] &&
        generations_[id.index] == id.generation;
}

TransformComponent* EntityWorld::Transform(EntityId id) noexcept {
    return IsAlive(id) ? &transforms_[id.index] : nullptr;
}

const TransformComponent* EntityWorld::Transform(EntityId id) const noexcept {
    return IsAlive(id) ? &transforms_[id.index] : nullptr;
}

SpriteComponent* EntityWorld::Sprite(EntityId id) noexcept {
    return IsAlive(id) ? &sprites_[id.index] : nullptr;
}

const SpriteComponent* EntityWorld::Sprite(EntityId id) const noexcept {
    return IsAlive(id) ? &sprites_[id.index] : nullptr;
}

const ColliderComponent* EntityWorld::Collider(EntityId id) const noexcept {
    return IsAlive(id) ? &colliders_[id.index] : nullptr;
}

void EntityWorld::Clear() noexcept {
    active_.fill(false);
}

} // namespace Homestead

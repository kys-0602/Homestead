#include "Homestead/Systems/PlayerMovement.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

constexpr float CollisionEpsilon = 0.001F;

struct CollisionRect {
    float left;
    float top;
    float right;
    float bottom;
};

bool GetCollisionRect(
    const TileMap& map,
    std::int32_t x,
    std::int32_t y,
    CollisionRect& result) noexcept {
    const Tile* tile = map.Get(x, y);
    const float tileLeft = static_cast<float>(x * TileSize);
    const float tileTop = static_cast<float>(y * TileSize);
    if (tile == nullptr) {
        result = {tileLeft, tileTop, tileLeft + TileSize, tileTop + TileSize};
        return true;
    }
    if ((tile->flags & TileFlagValue(TileFlag::Blocked)) == 0) {
        return false;
    }

    switch (static_cast<TileGraphic>(tile->object)) {
    case TileGraphic::OakTree:
        result = {tileLeft + 2.0F, tileTop, tileLeft + 14.0F, tileTop + 8.0F};
        break;
    case TileGraphic::FenceHorizontal:
        result = {tileLeft, tileTop + 8.0F, tileLeft + 16.0F, tileTop + 14.0F};
        break;
    case TileGraphic::FenceVertical:
        result = {tileLeft + 5.0F, tileTop, tileLeft + 11.0F, tileTop + 16.0F};
        break;
    case TileGraphic::Sign:
        result = {tileLeft + 4.0F, tileTop + 8.0F, tileLeft + 12.0F, tileTop + 16.0F};
        break;
    default:
        result = {tileLeft, tileTop, tileLeft + TileSize, tileTop + TileSize};
        break;
    }
    return true;
}

bool Overlaps(float firstMin, float firstMax, float secondMin, float secondMax) noexcept {
    return firstMax > secondMin + CollisionEpsilon &&
        firstMin < secondMax - CollisionEpsilon;
}

void MoveX(
    TransformComponent& transform,
    const ColliderComponent& collider,
    const TileMap& map,
    float amount) noexcept {
    const float previousX = transform.current.x;
    transform.current.x += amount;
    const std::int32_t firstY = static_cast<std::int32_t>(
        std::floor((transform.current.y + collider.top) / TileSize));
    const std::int32_t lastY = static_cast<std::int32_t>(
        std::floor((transform.current.y + collider.bottom - CollisionEpsilon) / TileSize));
    const std::int32_t firstX = static_cast<std::int32_t>(
        std::floor((transform.current.x + collider.left) / TileSize));
    const std::int32_t lastX = static_cast<std::int32_t>(
        std::floor((transform.current.x + collider.right - CollisionEpsilon) / TileSize));
    for (std::int32_t y = firstY; y <= lastY; ++y) {
        for (std::int32_t x = firstX; x <= lastX; ++x) {
            CollisionRect obstacle{};
            if (!GetCollisionRect(map, x, y, obstacle) ||
                !Overlaps(
                    transform.current.y + collider.top,
                    transform.current.y + collider.bottom,
                    obstacle.top,
                    obstacle.bottom)) {
                continue;
            }
            if (amount > 0.0F &&
                previousX + collider.right <= obstacle.left + CollisionEpsilon &&
                transform.current.x + collider.right > obstacle.left) {
                transform.current.x = std::min(
                    transform.current.x, obstacle.left - collider.right);
            } else if (amount < 0.0F &&
                       previousX + collider.left >= obstacle.right - CollisionEpsilon &&
                       transform.current.x + collider.left < obstacle.right) {
                transform.current.x = std::max(
                    transform.current.x, obstacle.right - collider.left);
            }
        }
    }
}

void MoveY(
    TransformComponent& transform,
    const ColliderComponent& collider,
    const TileMap& map,
    float amount) noexcept {
    const float previousY = transform.current.y;
    transform.current.y += amount;
    const std::int32_t firstX = static_cast<std::int32_t>(
        std::floor((transform.current.x + collider.left) / TileSize));
    const std::int32_t lastX = static_cast<std::int32_t>(
        std::floor((transform.current.x + collider.right - CollisionEpsilon) / TileSize));
    const std::int32_t firstY = static_cast<std::int32_t>(
        std::floor((transform.current.y + collider.top) / TileSize));
    const std::int32_t lastY = static_cast<std::int32_t>(
        std::floor((transform.current.y + collider.bottom - CollisionEpsilon) / TileSize));
    for (std::int32_t y = firstY; y <= lastY; ++y) {
        for (std::int32_t x = firstX; x <= lastX; ++x) {
            CollisionRect obstacle{};
            if (!GetCollisionRect(map, x, y, obstacle) ||
                !Overlaps(
                    transform.current.x + collider.left,
                    transform.current.x + collider.right,
                    obstacle.left,
                    obstacle.right)) {
                continue;
            }
            if (amount > 0.0F &&
                previousY + collider.bottom <= obstacle.top + CollisionEpsilon &&
                transform.current.y + collider.bottom > obstacle.top) {
                transform.current.y = std::min(
                    transform.current.y, obstacle.top - collider.bottom);
            } else if (amount < 0.0F &&
                       previousY + collider.top >= obstacle.bottom - CollisionEpsilon &&
                       transform.current.y + collider.top < obstacle.bottom) {
                transform.current.y = std::max(
                    transform.current.y, obstacle.bottom - collider.top);
            }
        }
    }
}

AssetId PlayerSprite(const PlayerState& player) noexcept {
    if (!player.moving) {
        switch (player.facing) {
        case FacingDirection::Up: return MakeAssetId("player.idle.up.0");
        case FacingDirection::Left:
        case FacingDirection::Right: return MakeAssetId("player.idle.right.0");
        case FacingDirection::Down: return MakeAssetId("player.idle.down.0");
        }
    }

    const std::uint16_t frame = static_cast<std::uint16_t>((player.animationTicks / 10U) % 6U);
    if (player.facing == FacingDirection::Up) {
        constexpr AssetId frames[] = {
            MakeAssetId("player.walk.up.0"), MakeAssetId("player.walk.up.1"),
            MakeAssetId("player.walk.up.2"), MakeAssetId("player.walk.up.3"),
            MakeAssetId("player.walk.up.4"), MakeAssetId("player.walk.up.5")};
        return frames[frame];
    }
    if (player.facing == FacingDirection::Left || player.facing == FacingDirection::Right) {
        constexpr AssetId frames[] = {
            MakeAssetId("player.walk.right.0"), MakeAssetId("player.walk.right.1"),
            MakeAssetId("player.walk.right.2"), MakeAssetId("player.walk.right.3"),
            MakeAssetId("player.walk.right.4"), MakeAssetId("player.walk.right.5")};
        return frames[frame];
    }
    constexpr AssetId frames[] = {
        MakeAssetId("player.walk.down.0"), MakeAssetId("player.walk.down.1"),
        MakeAssetId("player.walk.down.2"), MakeAssetId("player.walk.down.3"),
        MakeAssetId("player.walk.down.4"), MakeAssetId("player.walk.down.5")};
    return frames[frame];
}

} // namespace

bool UpdatePlayerMovement(
    EntityWorld& world,
    PlayerState& player,
    const TileMap& map,
    MovementInput input,
    float stepSeconds) noexcept {
    TransformComponent* transform = world.Transform(player.entity);
    SpriteComponent* sprite = world.Sprite(player.entity);
    const ColliderComponent* collider = world.Collider(player.entity);
    if (transform == nullptr || sprite == nullptr || collider == nullptr || stepSeconds < 0.0F) {
        return false;
    }

    transform->previous = transform->current;
    const float lengthSquared = input.x * input.x + input.y * input.y;
    player.moving = lengthSquared > 0.0F;
    if (player.moving) {
        const float inverseLength = 1.0F / std::sqrt(lengthSquared);
        input.x *= inverseLength;
        input.y *= inverseLength;
        if (std::fabs(input.x) > std::fabs(input.y)) {
            player.facing = input.x < 0.0F ? FacingDirection::Left : FacingDirection::Right;
        } else {
            player.facing = input.y < 0.0F ? FacingDirection::Up : FacingDirection::Down;
        }
        ++player.animationTicks;
        const float distance = player.movementSpeed * stepSeconds;
        MoveX(*transform, *collider, map, input.x * distance);
        MoveY(*transform, *collider, map, input.y * distance);
    } else {
        player.animationTicks = 0;
    }
    sprite->asset = PlayerSprite(player);
    return true;
}

} // namespace Homestead

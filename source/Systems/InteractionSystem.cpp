#include "Homestead/Systems/InteractionSystem.hpp"

#include <cmath>

#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

TileSelection MakeSelection(
    WorldPosition playerFeet,
    std::int32_t x,
    std::int32_t y,
    const TileMap& map) noexcept {
    const float centerX = (static_cast<float>(x) + 0.5F) * TileSize;
    const float centerY = (static_cast<float>(y) + 0.5F) * TileSize;
    const float deltaX = centerX - playerFeet.x;
    const float deltaY = centerY - playerFeet.y;
    TileSelection result{};
    result.x = x;
    result.y = y;
    result.valid = map.Get(x, y) != nullptr;
    result.inRange = deltaX * deltaX + deltaY * deltaY <=
        MaximumToolRange * MaximumToolRange;
    return result;
}

} // namespace

TileSelection SelectFrontTile(
    WorldPosition playerFeet,
    FacingDirection facing,
    const TileMap& map) noexcept {
    std::int32_t x = static_cast<std::int32_t>(std::floor(playerFeet.x / TileSize));
    std::int32_t y = static_cast<std::int32_t>(std::floor(playerFeet.y / TileSize));
    switch (facing) {
    case FacingDirection::Up: --y; break;
    case FacingDirection::Right: ++x; break;
    case FacingDirection::Down: ++y; break;
    case FacingDirection::Left: --x; break;
    }
    return MakeSelection(playerFeet, x, y, map);
}

TileSelection SelectMouseTile(
    WorldPosition playerFeet,
    WorldPosition mouseWorld,
    const TileMap& map) noexcept {
    const std::int32_t x = static_cast<std::int32_t>(std::floor(mouseWorld.x / TileSize));
    const std::int32_t y = static_cast<std::int32_t>(std::floor(mouseWorld.y / TileSize));
    return MakeSelection(playerFeet, x, y, map);
}

TileSelection SelectNearbySpecialObject(
    WorldPosition playerFeet,
    const TileMap& map) noexcept {
    constexpr float maximumRange = 40.0F;
    constexpr float maximumRangeSquared = maximumRange * maximumRange;
    TileSelection nearest{};
    float nearestDistance = maximumRangeSquared;
    for (std::int32_t y = 0; y < map.Height(); ++y) {
        for (std::int32_t x = 0; x < map.Width(); ++x) {
            const Tile* tile = map.Get(x, y);
            if (tile == nullptr) continue;
            const TileGraphic graphic = static_cast<TileGraphic>(tile->object);
            if (graphic != TileGraphic::Farmhouse && graphic != TileGraphic::Bed &&
                graphic != TileGraphic::Door) continue;
            const float deltaX = (static_cast<float>(x) + 0.5F) * TileSize - playerFeet.x;
            const float deltaY = (static_cast<float>(y) + 0.5F) * TileSize - playerFeet.y;
            const float distance = deltaX * deltaX + deltaY * deltaY;
            if (distance <= nearestDistance) {
                nearest = {x, y, true, true};
                nearestDistance = distance;
            }
        }
    }
    return nearest;
}

bool TryInteract(
    PlayerState& player,
    const TileMap& map,
    const TileSelection& selection) noexcept {
    if (player.toolUse.action != ToolAction::None) {
        return false;
    }
    const Tile* tile = selection.valid && selection.inRange ?
        map.Get(selection.x, selection.y) : nullptr;
    if (tile == nullptr || tile->object == 0) {
        return false;
    }
    player.lastInteractionX = static_cast<std::int16_t>(selection.x);
    player.lastInteractionY = static_cast<std::int16_t>(selection.y);
    ++player.interactionCount;
    return true;
}

void FaceSelection(
    PlayerState& player,
    WorldPosition playerFeet,
    const TileSelection& selection) noexcept {
    const float deltaX = (static_cast<float>(selection.x) + 0.5F) * TileSize - playerFeet.x;
    const float deltaY = (static_cast<float>(selection.y) + 0.5F) * TileSize - playerFeet.y;
    if (std::fabs(deltaX) > std::fabs(deltaY)) {
        player.facing = deltaX < 0.0F ? FacingDirection::Left : FacingDirection::Right;
    } else {
        player.facing = deltaY < 0.0F ? FacingDirection::Up : FacingDirection::Down;
    }
}

} // namespace Homestead

#include "Homestead/Systems/ToolSystem.hpp"

#include <cstdint>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {
namespace {

bool CanTill(const Tile& tile) noexcept {
    constexpr std::uint8_t forbidden = TileFlagValue(TileFlag::Blocked) |
        TileFlagValue(TileFlag::Water);
    return tile.object == 0 && (tile.flags & forbidden) == 0 &&
        (tile.flags & TileFlagValue(TileFlag::Farmable)) != 0 &&
        (tile.flags & TileFlagValue(TileFlag::Tilled)) == 0;
}

bool CanWater(const Tile& tile) noexcept {
    constexpr std::uint8_t forbidden = TileFlagValue(TileFlag::Blocked) |
        TileFlagValue(TileFlag::Water);
    return tile.object == 0 && (tile.flags & forbidden) == 0 &&
        (tile.flags & TileFlagValue(TileFlag::Tilled)) != 0 &&
        (tile.flags & TileFlagValue(TileFlag::Watered)) == 0;
}

AssetId ToolSprite(ToolAction action, FacingDirection facing, std::uint8_t frame) noexcept {
    const std::uint8_t index = static_cast<std::uint8_t>(frame % 6U);
    if (action == ToolAction::Hoe) {
        if (facing == FacingDirection::Up) {
            constexpr AssetId sprites[] = {
                MakeAssetId("player.hoe.up.0"), MakeAssetId("player.hoe.up.1"),
                MakeAssetId("player.hoe.up.2"), MakeAssetId("player.hoe.up.3"),
                MakeAssetId("player.hoe.up.4"), MakeAssetId("player.hoe.up.5")};
            return sprites[index];
        }
        if (facing == FacingDirection::Left || facing == FacingDirection::Right) {
            constexpr AssetId sprites[] = {
                MakeAssetId("player.hoe.right.0"), MakeAssetId("player.hoe.right.1"),
                MakeAssetId("player.hoe.right.2"), MakeAssetId("player.hoe.right.3"),
                MakeAssetId("player.hoe.right.4"), MakeAssetId("player.hoe.right.5")};
            return sprites[index];
        }
        constexpr AssetId sprites[] = {
            MakeAssetId("player.hoe.down.0"), MakeAssetId("player.hoe.down.1"),
            MakeAssetId("player.hoe.down.2"), MakeAssetId("player.hoe.down.3"),
            MakeAssetId("player.hoe.down.4"), MakeAssetId("player.hoe.down.5")};
        return sprites[index];
    }

    if (facing == FacingDirection::Up) {
        constexpr AssetId sprites[] = {
            MakeAssetId("player.water.up.0"), MakeAssetId("player.water.up.1"),
            MakeAssetId("player.water.up.2"), MakeAssetId("player.water.up.3"),
            MakeAssetId("player.water.up.4"), MakeAssetId("player.water.up.5")};
        return sprites[index];
    }
    if (facing == FacingDirection::Left || facing == FacingDirection::Right) {
        constexpr AssetId sprites[] = {
            MakeAssetId("player.water.right.0"), MakeAssetId("player.water.right.1"),
            MakeAssetId("player.water.right.2"), MakeAssetId("player.water.right.3"),
            MakeAssetId("player.water.right.4"), MakeAssetId("player.water.right.5")};
        return sprites[index];
    }
    constexpr AssetId sprites[] = {
        MakeAssetId("player.water.down.0"), MakeAssetId("player.water.down.1"),
        MakeAssetId("player.water.down.2"), MakeAssetId("player.water.down.3"),
        MakeAssetId("player.water.down.4"), MakeAssetId("player.water.down.5")};
    return sprites[index];
}

} // namespace

bool TryStartToolUse(
    PlayerState& player,
    const TileMap& map,
    const TileSelection& selection,
    ToolAction action) noexcept {
    if (player.toolUse.action != ToolAction::None || action == ToolAction::None ||
        !selection.valid || !selection.inRange) {
        return false;
    }
    const Tile* tile = map.Get(selection.x, selection.y);
    if (tile == nullptr) {
        return false;
    }

    if ((action == ToolAction::Hoe && !CanTill(*tile)) ||
        (action == ToolAction::Watering && !CanWater(*tile))) {
        return false;
    }

    player.toolUse.action = action;
    player.toolUse.tileX = static_cast<std::int16_t>(selection.x);
    player.toolUse.tileY = static_cast<std::int16_t>(selection.y);
    player.toolUse.elapsedTicks = 0;
    player.toolUse.applied = false;
    return true;
}

bool UpdateToolUse(EntityWorld& world, PlayerState& player, TileMap& map) noexcept {
    if (player.toolUse.action == ToolAction::None) {
        return true;
    }
    SpriteComponent* sprite = world.Sprite(player.entity);
    if (sprite == nullptr) {
        return false;
    }

    ++player.toolUse.elapsedTicks;
    const std::uint8_t frame = static_cast<std::uint8_t>(
        (static_cast<unsigned>(player.toolUse.elapsedTicks) * 6U) / ToolActionTicks);
    sprite->asset = ToolSprite(
        player.toolUse.action,
        player.facing,
        frame < 6U ? frame : 5U);

    if (!player.toolUse.applied && player.toolUse.elapsedTicks >= ToolImpactTick) {
        Tile* tile = map.Get(player.toolUse.tileX, player.toolUse.tileY);
        if (tile == nullptr) {
            return false;
        }
        if (player.toolUse.action == ToolAction::Hoe) {
            tile->flags |= TileFlagValue(TileFlag::Tilled);
        } else {
            tile->flags |= TileFlagValue(TileFlag::Watered);
        }
        player.toolUse.applied = true;
    }

    if (player.toolUse.elapsedTicks >= ToolActionTicks) {
        player.toolUse = {};
    }
    return true;
}

} // namespace Homestead

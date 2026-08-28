#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/Systems/PlayerMovement.hpp"
#include "Homestead/Systems/ToolSystem.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/TileMap.hpp"

#include <cstdint>
#include <vector>

namespace {

void U16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}

void U32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 16U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 24U));
}

std::vector<std::uint8_t> MakeMap() {
    constexpr std::uint16_t width = 8;
    constexpr std::uint16_t height = 8;
    std::vector<std::uint8_t> bytes{'H', 'S', 'T', 'M'};
    U16(bytes, 1); U16(bytes, 24); U16(bytes, width); U16(bytes, height);
    U16(bytes, 16); U16(bytes, 16); U16(bytes, 3); U16(bytes, 6);
    U32(bytes, static_cast<std::uint32_t>(width) * height);
    for (std::uint16_t y = 0; y < height; ++y) {
        for (std::uint16_t x = 0; x < width; ++x) {
            const bool path = x == 3 && y == 2;
            const bool sign = x == 2 && y == 1;
            const bool fence = x == 4 && y == 2;
            U16(bytes, static_cast<std::uint16_t>(
                path ? Homestead::TileGraphic::Path : Homestead::TileGraphic::Grass));
            U16(bytes, static_cast<std::uint16_t>(
                sign ? Homestead::TileGraphic::Sign :
                (fence ? Homestead::TileGraphic::FenceVertical : Homestead::TileGraphic::None)));
            const std::uint8_t flags = sign || fence ?
                Homestead::TileFlagValue(Homestead::TileFlag::Blocked) :
                (x == 2 && y == 3 ? Homestead::TileFlagValue(Homestead::TileFlag::Farmable) : 0);
            bytes.push_back(flags);
            bytes.push_back(0);
        }
    }
    return bytes;
}

} // namespace

int main() {
    const std::vector<std::uint8_t> bytes = MakeMap();
    Homestead::TileMap map;
    if (!map.LoadMemory(bytes.data(), bytes.size())) return 1;

    const Homestead::WorldPosition feet{40.0F, 40.0F};
    const Homestead::TileSelection front = Homestead::SelectFrontTile(
        feet, Homestead::FacingDirection::Up, map);
    if (!front.valid || !front.inRange || front.x != 2 || front.y != 1) return 2;
    const Homestead::TileSelection nearby = Homestead::SelectNearbySpecialObject(feet, map);
    if (!nearby.valid || !nearby.inRange || nearby.x != 2 || nearby.y != 1) return 2;

    const Homestead::TileSelection mouse = Homestead::SelectMouseTile(
        feet, {72.0F, 40.0F}, map);
    if (!mouse.valid || !mouse.inRange || mouse.x != 4 || mouse.y != 2) return 3;
    const Homestead::TileSelection distant = Homestead::SelectMouseTile(
        feet, {88.0F, 40.0F}, map);
    if (!distant.valid || distant.inRange) return 4;

    Homestead::PlayerState player{};
    if (!Homestead::TryInteract(player, map, front) || player.interactionCount != 1 ||
        player.lastInteractionX != 2 || player.lastInteractionY != 1) return 5;
    if (Homestead::TryInteract(player, map, distant)) return 6;
    Homestead::FaceSelection(player, feet, mouse);
    if (player.facing != Homestead::FacingDirection::Right) return 7;

    Homestead::EntityWorld world;
    player.entity = world.Create(feet, Homestead::MakeAssetId("player.idle.down.0"));
    player.facing = Homestead::FacingDirection::Down;
    const Homestead::TileSelection farm = Homestead::SelectFrontTile(
        feet, Homestead::FacingDirection::Down, map);
    if (!Homestead::TryStartToolUse(player, map, farm, Homestead::ToolAction::Hoe) ||
        player.toolUse.action != Homestead::ToolAction::Hoe ||
        Homestead::TryStartToolUse(player, map, farm, Homestead::ToolAction::Hoe)) return 8;

    const Homestead::TransformComponent* before = world.Transform(player.entity);
    if (before == nullptr) return 9;
    const Homestead::WorldPosition position = before->current;
    if (!Homestead::UpdatePlayerMovement(world, player, map, {1.0F, 0.0F}, 1.0F / 60.0F))
        return 10;
    const Homestead::TransformComponent* locked = world.Transform(player.entity);
    if (locked == nullptr || locked->current.x != position.x || locked->current.y != position.y)
        return 11;

    for (unsigned tick = 0; tick + 1U < Homestead::ToolImpactTick; ++tick) {
        if (!Homestead::UpdateToolUse(world, player, map)) return 12;
    }
    const Homestead::Tile* farmTile = map.Get(farm.x, farm.y);
    const Homestead::SpriteComponent* toolSprite = world.Sprite(player.entity);
    if (farmTile == nullptr ||
        (farmTile->flags & Homestead::TileFlagValue(Homestead::TileFlag::Tilled)) != 0 ||
        toolSprite == nullptr ||
        toolSprite->asset != Homestead::MakeAssetId("player.hoe.down.2"))
        return 13;
    if (!Homestead::UpdateToolUse(world, player, map)) return 14;
    farmTile = map.Get(farm.x, farm.y);
    toolSprite = world.Sprite(player.entity);
    if (farmTile == nullptr ||
        (farmTile->flags & Homestead::TileFlagValue(Homestead::TileFlag::Tilled)) == 0 ||
        toolSprite == nullptr ||
        toolSprite->asset != Homestead::MakeAssetId("player.hoe.down.3"))
        return 15;

    while (player.toolUse.action != Homestead::ToolAction::None) {
        if (!Homestead::UpdateToolUse(world, player, map)) return 16;
    }
    if (!Homestead::TryStartToolUse(player, map, farm, Homestead::ToolAction::Watering) ||
        player.toolUse.action != Homestead::ToolAction::Watering) return 17;
    for (unsigned tick = 0; tick < Homestead::ToolImpactTick; ++tick) {
        if (!Homestead::UpdateToolUse(world, player, map)) return 18;
    }
    farmTile = map.Get(farm.x, farm.y);
    if (farmTile == nullptr ||
        (farmTile->flags & Homestead::TileFlagValue(Homestead::TileFlag::Watered)) == 0)
        return 19;

    player.toolUse = {};
    const Homestead::TileSelection bareGrass = Homestead::SelectMouseTile(
        feet, {24.0F, 56.0F}, map);
    const Homestead::TileSelection path = Homestead::SelectFrontTile(
        feet, Homestead::FacingDirection::Right, map);
    if (!bareGrass.valid || !bareGrass.inRange ||
        Homestead::TryStartToolUse(player, map, bareGrass, Homestead::ToolAction::Hoe) ||
        Homestead::TryStartToolUse(player, map, path, Homestead::ToolAction::Hoe) ||
        Homestead::TryStartToolUse(player, map, mouse, Homestead::ToolAction::Hoe) ||
        Homestead::TryStartToolUse(player, map, distant, Homestead::ToolAction::Hoe)) return 20;
    return 0;
}

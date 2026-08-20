#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Systems/PlayerMovement.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/TileMap.hpp"

#include <cmath>
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
    constexpr std::uint16_t width = 32;
    constexpr std::uint16_t height = 24;
    std::vector<std::uint8_t> bytes{'H', 'S', 'T', 'M'};
    U16(bytes, 1); U16(bytes, 24); U16(bytes, width); U16(bytes, height);
    U16(bytes, 16); U16(bytes, 16); U16(bytes, 3); U16(bytes, 6);
    U32(bytes, static_cast<std::uint32_t>(width) * height);
    for (std::uint16_t y = 0; y < height; ++y) {
        for (std::uint16_t x = 0; x < width; ++x) {
            U16(bytes, static_cast<std::uint16_t>(Homestead::TileGraphic::Grass));
            const bool fence = x == 10 && y >= 2 && y <= 12;
            const bool tree = x == 12 && y == 8;
            const bool blocked = fence || tree;
            U16(bytes, fence ?
                static_cast<std::uint16_t>(Homestead::TileGraphic::FenceVertical) :
                (tree ? static_cast<std::uint16_t>(Homestead::TileGraphic::OakTree) : 0));
            bytes.push_back(blocked ? Homestead::TileFlagValue(Homestead::TileFlag::Blocked) : 0);
            bytes.push_back(0);
        }
    }
    return bytes;
}

bool Near(float left, float right) {
    return std::fabs(left - right) < 0.01F;
}

} // namespace

int main() {
    const std::vector<std::uint8_t> bytes = MakeMap();
    Homestead::TileMap map;
    if (!map.LoadMemory(bytes.data(), bytes.size())) return 1;

    Homestead::EntityWorld axisWorld;
    Homestead::PlayerState axisPlayer{};
    axisPlayer.entity = axisWorld.Create({64.0F, 64.0F}, 1);
    for (int step = 0; step < 60; ++step) {
        if (!Homestead::UpdatePlayerMovement(
                axisWorld, axisPlayer, map, {0.0F, 1.0F}, 1.0F / 60.0F)) return 2;
    }
    const Homestead::TransformComponent* axis = axisWorld.Transform(axisPlayer.entity);
    if (axis == nullptr || !Near(axis->current.y - 64.0F, 60.0F)) return 3;

    Homestead::EntityWorld diagonalWorld;
    Homestead::PlayerState diagonalPlayer{};
    diagonalPlayer.entity = diagonalWorld.Create({64.0F, 64.0F}, 1);
    for (int step = 0; step < 60; ++step) {
        if (!Homestead::UpdatePlayerMovement(
                diagonalWorld, diagonalPlayer, map, {1.0F, 1.0F}, 1.0F / 60.0F)) return 4;
    }
    const Homestead::TransformComponent* diagonal =
        diagonalWorld.Transform(diagonalPlayer.entity);
    if (diagonal == nullptr) return 5;
    const float diagonalDistance = std::sqrt(
        (diagonal->current.x - 64.0F) * (diagonal->current.x - 64.0F) +
        (diagonal->current.y - 64.0F) * (diagonal->current.y - 64.0F));
    if (!Near(diagonalDistance, 60.0F)) return 6;

    Homestead::EntityWorld collisionWorld;
    Homestead::PlayerState collisionPlayer{};
    collisionPlayer.entity = collisionWorld.Create({155.0F, 80.0F}, 1);
    for (int step = 0; step < 30; ++step) {
        if (!Homestead::UpdatePlayerMovement(
                collisionWorld, collisionPlayer, map, {1.0F, 1.0F}, 1.0F / 60.0F)) return 7;
    }
    const Homestead::TransformComponent* collision =
        collisionWorld.Transform(collisionPlayer.entity);
    if (collision == nullptr || !Near(collision->current.x, 160.0F) ||
        collision->current.y <= 80.0F) return 8;

    const Homestead::TransformComponent before = *collision;
    if (!Homestead::UpdatePlayerMovement(
            collisionWorld, collisionPlayer, map, {}, 1.0F / 60.0F)) return 9;
    collision = collisionWorld.Transform(collisionPlayer.entity);
    if (collision == nullptr || !Near(collision->previous.x, before.current.x) ||
        !Near(collision->previous.y, before.current.y)) return 10;

    Homestead::EntityWorld treeWorld;
    Homestead::PlayerState treePlayer{};
    treePlayer.entity = treeWorld.Create({200.0F, 120.0F}, 1);
    for (int step = 0; step < 30; ++step) {
        if (!Homestead::UpdatePlayerMovement(
                treeWorld, treePlayer, map, {0.0F, 1.0F}, 1.0F / 60.0F)) return 11;
    }
    const Homestead::TransformComponent* tree = treeWorld.Transform(treePlayer.entity);
    if (tree == nullptr || !Near(tree->current.y, 128.0F)) return 12;

    Homestead::EntityWorld passWorld;
    Homestead::PlayerState passPlayer{};
    passPlayer.entity = passWorld.Create({206.0F, 132.0F}, 1);
    if (!Homestead::UpdatePlayerMovement(
            passWorld, passPlayer, map, {1.0F, 0.0F}, 1.0F / 60.0F)) return 13;
    const Homestead::TransformComponent* passed = passWorld.Transform(passPlayer.entity);
    if (passed == nullptr || !Near(passed->current.x, 207.0F)) return 14;
    return 0;
}

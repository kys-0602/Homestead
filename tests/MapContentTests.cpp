#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/TileMapRenderer.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/World/TileMap.hpp"

#include <Windows.h>

#include <iterator>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;
    wchar_t path[1024]{};
    const int converted = MultiByteToWideChar(
        CP_UTF8, 0, arguments[1], -1, path, static_cast<int>(std::size(path)));
    if (converted == 0) return 2;

    Homestead::AssetStore assets;
    if (!assets.LoadFile(path)) return 3;
    const Homestead::MapAsset* farmAsset = assets.FindMap(Homestead::MakeAssetId("map/farm"));
    const Homestead::MapAsset* houseAsset = assets.FindMap(Homestead::MakeAssetId("map/house"));
    Homestead::TileMap farm;
    Homestead::TileMap house;
    if (farmAsset == nullptr || houseAsset == nullptr ||
        !farm.LoadMemory(farmAsset->bytes.data(), farmAsset->bytes.size()) ||
        !house.LoadMemory(houseAsset->bytes.data(), houseAsset->bytes.size())) return 4;
    const Homestead::Tile* farmhouse = farm.Get(8, 7);
    const Homestead::Tile* farmSpawn = farm.Get(8, 10);
    const Homestead::Tile* requestSign = farm.Get(10, 11);
    const Homestead::Tile* marketSign = farm.Get(15, 11);
    const Homestead::Tile* bed = house.Get(3, 2);
    const Homestead::Tile* bookshelf = house.Get(12, 1);
    const Homestead::Tile* door = house.Get(8, 8);
    const Homestead::Tile* houseSpawn = house.Get(8, 7);
    unsigned signCount = 0;
    for (std::int32_t y = 0; y < farm.Height(); ++y) {
        for (std::int32_t x = 0; x < farm.Width(); ++x) {
            const Homestead::Tile* tile = farm.Get(x, y);
            if (tile != nullptr && tile->object ==
                static_cast<std::uint16_t>(Homestead::TileGraphic::Sign)) ++signCount;
        }
    }
    if (farm.Width() != 32 || farm.Height() != 24 || house.Width() != 16 || house.Height() != 10 ||
        farmhouse == nullptr || farmhouse->object != static_cast<std::uint16_t>(Homestead::TileGraphic::Farmhouse) ||
        farmSpawn == nullptr || farmSpawn->flags != 0 ||
        signCount != 1 ||
        requestSign == nullptr ||
        requestSign->object != static_cast<std::uint16_t>(Homestead::TileGraphic::Sign) ||
        (requestSign->flags & Homestead::TileFlagValue(Homestead::TileFlag::Blocked)) == 0 ||
        marketSign == nullptr ||
        marketSign->object != static_cast<std::uint16_t>(Homestead::TileGraphic::MarketSign) ||
        (marketSign->flags & Homestead::TileFlagValue(Homestead::TileFlag::Blocked)) == 0 ||
        bed == nullptr || bed->object != static_cast<std::uint16_t>(Homestead::TileGraphic::Bed) ||
        bookshelf == nullptr || bookshelf->object != static_cast<std::uint16_t>(Homestead::TileGraphic::Bookshelf) ||
        door == nullptr || door->object != static_cast<std::uint16_t>(Homestead::TileGraphic::Door) ||
        houseSpawn == nullptr || houseSpawn->flags != 0) return 5;
    const Homestead::TileSelection farmhouseSelection = Homestead::SelectNearbySpecialObject(
        {8.5F * Homestead::TileSize, 9.5F * Homestead::TileSize}, farm);
    const Homestead::TileSelection bedSelection = Homestead::SelectNearbySpecialObject(
        {5.0F * Homestead::TileSize, 2.5F * Homestead::TileSize}, house);
    const Homestead::TileSelection bookshelfSelection = Homestead::SelectNearbySpecialObject(
        {12.5F * Homestead::TileSize, 3.5F * Homestead::TileSize}, house);
    const Homestead::TileSelection doorSelection = Homestead::SelectNearbySpecialObject(
        {8.5F * Homestead::TileSize, 7.5F * Homestead::TileSize}, house);
    const Homestead::TileSelection signSelection = Homestead::SelectNearbySpecialObject(
        {8.5F * Homestead::TileSize, 10.5F * Homestead::TileSize}, farm);
    const Homestead::TileSelection marketSelection = Homestead::SelectNearbySpecialObject(
        {13.5F * Homestead::TileSize, 10.5F * Homestead::TileSize}, farm);
    if (!farmhouseSelection.valid || farmhouseSelection.x != 8 || farmhouseSelection.y != 7 ||
        !bedSelection.valid || bedSelection.x != 3 || bedSelection.y != 2 ||
        !bookshelfSelection.valid || bookshelfSelection.x != 12 || bookshelfSelection.y != 1 ||
        !doorSelection.valid || doorSelection.x != 8 || doorSelection.y != 8 ||
        !signSelection.valid || signSelection.x != 10 || signSelection.y != 11 ||
        !marketSelection.valid || marketSelection.x != 15 || marketSelection.y != 11) return 6;
    Homestead::Tile* center = farm.Get(20, 18);
    Homestead::Tile* up = farm.Get(20, 17);
    Homestead::Tile* right = farm.Get(21, 18);
    Homestead::Tile* down = farm.Get(20, 19);
    Homestead::Tile* left = farm.Get(19, 18);
    const Homestead::Tile* grass = farm.Get(12, 18);
    if (center == nullptr || up == nullptr || right == nullptr || down == nullptr || left == nullptr ||
        grass == nullptr ||
        (center->flags & Homestead::TileFlagValue(Homestead::TileFlag::Farmable)) == 0 ||
        (grass->flags & Homestead::TileFlagValue(Homestead::TileFlag::Farmable)) != 0) return 7;
    center->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Tilled);
    if (Homestead::FarmlandConnectionMask(farm, 20, 18) != 0) return 8;
    up->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Tilled);
    right->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Tilled);
    down->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Tilled);
    left->flags |= Homestead::TileFlagValue(Homestead::TileFlag::Tilled);
    if (Homestead::FarmlandConnectionMask(farm, 20, 18) != 15 ||
        Homestead::FarmlandConnectionMask(farm, 20, 17) != 4 ||
        Homestead::FarmlandConnectionMask(farm, 21, 18) != 8 ||
        Homestead::FarmlandConnectionMask(farm, 20, 19) != 1 ||
        Homestead::FarmlandConnectionMask(farm, 19, 18) != 2) return 9;
    for (unsigned mask = 0; mask < 16; ++mask) {
        char dry[] = "terrain.farmland.dry.00";
        char wet[] = "terrain.farmland.wet.00";
        const std::size_t dryDigit = sizeof(dry) - 3;
        const std::size_t wetDigit = sizeof(wet) - 3;
        if (mask < 10) {
            dry[dryDigit] = static_cast<char>('0' + mask); dry[dryDigit + 1] = '\0';
            wet[wetDigit] = static_cast<char>('0' + mask); wet[wetDigit + 1] = '\0';
        } else {
            dry[dryDigit] = '1'; dry[dryDigit + 1] = static_cast<char>('0' + mask - 10);
            wet[wetDigit] = '1'; wet[wetDigit + 1] = static_cast<char>('0' + mask - 10);
        }
        if (assets.FindSprite(Homestead::MakeAssetId(dry)) == nullptr ||
            assets.FindSprite(Homestead::MakeAssetId(wet)) == nullptr) return 10;
    }
    constexpr const char* uiAssets[] = {
        "font.colon", "font.percent", "ui.slot.frame", "ui.slot.selected", "ui.panel.tl",
        "ui.panel.top", "ui.panel.tr", "ui.panel.left", "ui.panel.center",
        "ui.panel.right", "ui.panel.bl", "ui.panel.bottom", "ui.panel.br"};
    for (const char* name : uiAssets) {
        if (assets.FindSprite(Homestead::MakeAssetId(name)) == nullptr) return 11;
    }
    if (assets.FindSprite(Homestead::MakeAssetId("terrain.farmable.soil")) == nullptr ||
        assets.FindSprite(Homestead::MakeAssetId("animal.chicken.idle")) == nullptr ||
        assets.FindSprite(Homestead::MakeAssetId("animal.chicken.walk")) == nullptr ||
        assets.FindSprite(Homestead::MakeAssetId("animal.frog.idle")) == nullptr ||
        assets.FindSprite(Homestead::MakeAssetId("animal.frog.walk")) == nullptr) return 12;
    return 0;
}

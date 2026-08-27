#include "Homestead/Save/SaveCodec.hpp"
#include "Homestead/World/TileMap.hpp"

#include <fstream>
#include <vector>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;

    Homestead::SaveSnapshot snapshot;
    snapshot.playerX256 = 15 * 16 * 256;
    snapshot.playerY256 = 10 * 16 * 256;
    snapshot.day = 12;
    snapshot.minute = 14 * 60 + 30;
    snapshot.gold = 76;
    snapshot.dailyRequestCompleted = true;
    snapshot.selectedSlot = 2;
    snapshot.inventory[0] = {Homestead::ItemId::Hoe, 1};
    snapshot.inventory[1] = {Homestead::ItemId::WateringCan, 1};
    snapshot.inventory[2] = {Homestead::ItemId::WheatSeed, 8};
    snapshot.inventory[3] = {Homestead::ItemId::CarrotSeed, 5};
    snapshot.inventory[4] = {Homestead::ItemId::TomatoSeed, 3};
    snapshot.inventory[5] = {Homestead::ItemId::Wheat, 4};
    snapshot.inventory[6] = {Homestead::ItemId::Carrot, 2};
    snapshot.inventory[7] = {Homestead::ItemId::Tomato, 1};
    snapshot.inventory[8] = {Homestead::ItemId::PotatoSeed, 5};
    snapshot.inventory[9] = {Homestead::ItemId::Corn, 2};
    snapshot.inventory[10] = {Homestead::ItemId::CabbageSeed, 2};

    constexpr std::uint8_t farmFlags = Homestead::TileFlagValue(Homestead::TileFlag::Tilled) |
        Homestead::TileFlagValue(Homestead::TileFlag::Watered);
    for (std::uint8_t y = 13; y < 17; ++y) {
        for (std::uint8_t x = 13; x < 19; ++x) snapshot.tileDeltas.push_back({x, y, farmFlags});
    }
    snapshot.crops.push_back({13, 13, Homestead::CropId::Wheat, 3, 2, true});
    snapshot.crops.push_back({14, 13, Homestead::CropId::Carrot, 2, 2, true});
    snapshot.crops.push_back({15, 13, Homestead::CropId::Tomato, 1, 2, true});

    std::vector<std::uint8_t> bytes;
    if (!Homestead::EncodeSave(snapshot, bytes)) return 2;
    std::ofstream output(arguments[1], std::ios::binary | std::ios::trunc);
    if (!output) return 3;
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return output ? 0 : 4;
}

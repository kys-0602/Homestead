#include "Homestead/Save/SaveCodec.hpp"
#include "Homestead/World/TileMap.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>

namespace {

std::uint32_t Checksum(const std::uint8_t* data, std::size_t size) {
    std::uint32_t value = 2166136261U;
    for (std::size_t index = 0; index < size; ++index) { value ^= data[index]; value *= 16777619U; }
    return value;
}

void SetU16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}

void SetU32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    SetU16(bytes, offset, static_cast<std::uint16_t>(value));
    SetU16(bytes, offset + 2, static_cast<std::uint16_t>(value >> 16U));
}

void RepairChecksum(std::vector<std::uint8_t>& bytes) {
    SetU32(bytes, 12, Checksum(bytes.data() + 16, bytes.size() - 16));
}

bool Rejects(const std::vector<std::uint8_t>& bytes) {
    Homestead::SaveSnapshot snapshot;
    return !Homestead::DecodeSave(bytes.data(), bytes.size(), snapshot);
}

std::vector<std::uint8_t> Version5Save(const std::vector<std::uint8_t>& version6) {
    std::vector<std::uint8_t> bytes = version6;
    bytes.erase(bytes.begin() + 33); // version 6 crop catalogue bits
    SetU16(bytes, 4, 5);
    SetU32(bytes, 8, static_cast<std::uint32_t>(bytes.size() - 16));
    RepairChecksum(bytes);
    return bytes;
}

std::vector<std::uint8_t> Version4Save(const std::vector<std::uint8_t>& version5) {
    std::vector<std::uint8_t> bytes = version5;
    for (std::size_t index = Homestead::Inventory::SlotCount; index > 0; --index)
        bytes.erase(bytes.begin() + 37 + (index - 1) * 3 + 2);
    SetU16(bytes, 4, 4);
    SetU32(bytes, 8, static_cast<std::uint32_t>(bytes.size() - 16));
    RepairChecksum(bytes);
    return bytes;
}

std::vector<std::uint8_t> Version3Save(const std::vector<std::uint8_t>& version4) {
    std::vector<std::uint8_t> bytes = version4;
    bytes.erase(bytes.begin() + 32); // version 4 daily request completion
    SetU16(bytes, 4, 3);
    SetU32(bytes, 8, static_cast<std::uint32_t>(bytes.size() - 16));
    RepairChecksum(bytes);
    return bytes;
}

std::vector<std::uint8_t> Version2Save(const std::vector<std::uint8_t>& version3) {
    std::vector<std::uint8_t> bytes = version3;
    bytes.erase(bytes.begin() + 29); // version 3 map ID
    SetU16(bytes, 4, 2);
    SetU32(bytes, 8, static_cast<std::uint32_t>(bytes.size() - 16));
    RepairChecksum(bytes);
    return bytes;
}

std::vector<std::uint8_t> LegacySave() {
    std::vector<std::uint8_t> payload(54,0);
    payload[8]=1; payload[10]=104; payload[11]=1; // day 1, minute 360
    payload[12]=0; payload[13]=0; // selected slot, legacy harvested carrots
    payload[18]=3; payload[19]=1; // legacy hoe
    payload[20]=4; payload[21]=1; // legacy watering can
    payload[22]=1; payload[23]=5; // legacy carrot seed
    payload[16]=1; // crop count
    payload[50]=3; payload[51]=4; payload[52]=1; payload[53]=2; // legacy carrot crop
    std::vector<std::uint8_t> bytes{'H','S','S','V',1,0,16,0};
    bytes.resize(16); SetU32(bytes,8,static_cast<std::uint32_t>(payload.size()));
    SetU32(bytes,12,Checksum(payload.data(),payload.size()));
    bytes.insert(bytes.end(),payload.begin(),payload.end()); return bytes;
}

} // namespace

int main(int argumentCount, char** arguments) {
    Homestead::SaveSnapshot source;
    source.playerX256 = 12345; source.playerY256 = 23456;
    source.day = 4; source.minute = 777; source.selectedSlot = 2;
    source.mapId = Homestead::MapId::House; source.gold = 87;
    source.dailyRequestCompleted = true;
    source.discoveredCrops = 0x03;
    source.inventory[0] = {Homestead::ItemId::Hoe, 1};
    source.inventory[1] = {Homestead::ItemId::WateringCan, 1};
    source.inventory[2] = {Homestead::ItemId::CarrotSeed, 9};
    source.inventory[3] = {Homestead::ItemId::Carrot, 2, Homestead::ItemQuality::Silver};
    source.tileDeltas.push_back({3, 4, Homestead::TileFlagValue(Homestead::TileFlag::Tilled)});
    source.crops.push_back({3, 4, Homestead::CropId::Carrot, 2, 2, true});
    std::vector<std::uint8_t> bytes;
    if (!Homestead::EncodeSave(source, bytes) || bytes.size() >= Homestead::MaximumSaveBytes) return 1;
    Homestead::SaveSnapshot decoded;
    if (!Homestead::DecodeSave(bytes.data(), bytes.size(), decoded) || decoded.day != 4 ||
        decoded.minute != 777 || decoded.mapId != Homestead::MapId::House ||
        decoded.gold != 87 || !decoded.dailyRequestCompleted || decoded.inventory[2].count != 9 ||
        decoded.inventory[3].quality != Homestead::ItemQuality::Silver ||
        decoded.discoveredCrops != 0x03 ||
        decoded.tileDeltas.size() != 1 || decoded.crops.size() != 1 || decoded.crops[0].stage != 2) return 2;
    const auto version5 = Version5Save(bytes); Homestead::SaveSnapshot migratedV5;
    if (!Homestead::DecodeSave(version5.data(), version5.size(), migratedV5) ||
        migratedV5.discoveredCrops != 0) return 3;
    const auto version4 = Version4Save(version5); Homestead::SaveSnapshot migratedV4;
    if (!Homestead::DecodeSave(version4.data(), version4.size(), migratedV4) ||
        migratedV4.inventory[3].quality != Homestead::ItemQuality::Normal) return 3;
    const auto version3 = Version3Save(version4); Homestead::SaveSnapshot migratedV3;
    if (!Homestead::DecodeSave(version3.data(), version3.size(), migratedV3) ||
        migratedV3.dailyRequestCompleted) return 3;
    const auto version2 = Version2Save(version3); Homestead::SaveSnapshot migratedV2;
    if (!Homestead::DecodeSave(version2.data(), version2.size(), migratedV2) ||
        migratedV2.mapId != Homestead::MapId::Farm || migratedV2.gold != 87 ||
        migratedV2.dailyRequestCompleted) return 3;

    auto damaged = bytes; damaged[0] = 'X'; if (!Rejects(damaged)) return 4;
    damaged = bytes; SetU16(damaged, 4, 7); if (!Rejects(damaged)) return 4;
    damaged = bytes; damaged.pop_back(); if (!Rejects(damaged)) return 5;
    damaged = bytes; damaged.back() ^= 1; if (!Rejects(damaged)) return 6;
    damaged = bytes; damaged[29] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 7;
    damaged = bytes; damaged[32] = 2; RepairChecksum(damaged); if (!Rejects(damaged)) return 8;
    damaged = bytes; damaged[33] = 0x80; RepairChecksum(damaged); if (!Rejects(damaged)) return 8;
    damaged = bytes; damaged[37] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 8;
    damaged = bytes; SetU16(damaged, 34, 5000); RepairChecksum(damaged); if (!Rejects(damaged)) return 9;
    damaged = bytes; damaged[39] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 10;
    damaged = bytes; damaged[92] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 11;
    damaged = bytes; damaged.resize(Homestead::MaximumSaveBytes + 1); if (!Rejects(damaged)) return 12;

    source.inventory[2].count = 100;
    if (Homestead::EncodeSave(source, bytes)) return 13;
    source.inventory[2].count = 9; source.crops[0].wateredDays = 255;
    if (Homestead::EncodeSave(source, bytes)) return 14;
    source.crops[0].wateredDays = 2; source.crops[0].stage = 1;
    if (Homestead::EncodeSave(source, bytes)) return 15;
    const auto legacy=LegacySave(); Homestead::SaveSnapshot migrated;
    if(!Homestead::DecodeSave(legacy.data(),legacy.size(),migrated)||migrated.gold!=20||
       migrated.inventory[0].item!=Homestead::ItemId::Hoe||
       migrated.inventory[2].item!=Homestead::ItemId::CarrotSeed||migrated.crops.size()!=1||
       migrated.crops[0].crop!=Homestead::CropId::Carrot||migrated.crops[0].stage!=2||
       migrated.crops[0].wateredDays!=2||migrated.mapId!=Homestead::MapId::Farm||
       migrated.dailyRequestCompleted||migrated.discoveredCrops!=0)return 16;
    auto invalidLegacy=legacy;invalidLegacy[16+13]=4;RepairChecksum(invalidLegacy);
    if(!Rejects(invalidLegacy))return 17;
    if (argumentCount != 2) return 18;
    std::ifstream input(arguments[1], std::ios::binary);
    if (!input) return 19;
    const std::vector<std::uint8_t> representative{
        std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    Homestead::SaveSnapshot submitted;
    if (representative.empty() || representative.size() > Homestead::MaximumSaveBytes ||
        !Homestead::DecodeSave(representative.data(), representative.size(), submitted) ||
        submitted.day != 12 || submitted.crops.size() != 3 || submitted.tileDeltas.size() != 24)
        return 20;
    return 0;
}

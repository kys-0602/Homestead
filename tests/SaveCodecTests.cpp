#include "Homestead/Save/SaveCodec.hpp"
#include "Homestead/World/TileMap.hpp"

#include <cstddef>
#include <cstdint>
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

int main() {
    Homestead::SaveSnapshot source;
    source.playerX256 = 12345; source.playerY256 = 23456;
    source.day = 4; source.minute = 777; source.selectedSlot = 2; source.gold = 87;
    source.inventory[0] = {Homestead::ItemId::Hoe, 1};
    source.inventory[1] = {Homestead::ItemId::WateringCan, 1};
    source.inventory[2] = {Homestead::ItemId::CarrotSeed, 9};
    source.tileDeltas.push_back({3, 4, Homestead::TileFlagValue(Homestead::TileFlag::Tilled)});
    source.crops.push_back({3, 4, Homestead::CropId::Carrot, 2, 2, true});
    std::vector<std::uint8_t> bytes;
    if (!Homestead::EncodeSave(source, bytes) || bytes.size() >= Homestead::MaximumSaveBytes) return 1;
    Homestead::SaveSnapshot decoded;
    if (!Homestead::DecodeSave(bytes.data(), bytes.size(), decoded) || decoded.day != 4 ||
        decoded.minute != 777 || decoded.gold != 87 || decoded.inventory[2].count != 9 ||
        decoded.tileDeltas.size() != 1 || decoded.crops.size() != 1 || decoded.crops[0].stage != 2) return 2;

    auto damaged = bytes; damaged[0] = 'X'; if (!Rejects(damaged)) return 3;
    damaged = bytes; SetU16(damaged, 4, 3); if (!Rejects(damaged)) return 4;
    damaged = bytes; damaged.pop_back(); if (!Rejects(damaged)) return 5;
    damaged = bytes; damaged.back() ^= 1; if (!Rejects(damaged)) return 6;
    damaged = bytes; damaged[35] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 7;
    damaged = bytes; SetU16(damaged, 31, 5000); RepairChecksum(damaged); if (!Rejects(damaged)) return 8;
    damaged = bytes; damaged[69] = 0x80; RepairChecksum(damaged); if (!Rejects(damaged)) return 9;
    damaged = bytes; damaged[72] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 10;
    damaged = bytes; damaged.resize(Homestead::MaximumSaveBytes + 1); if (!Rejects(damaged)) return 11;

    source.inventory[2].count = 100;
    if (Homestead::EncodeSave(source, bytes)) return 12;
    source.inventory[2].count = 9; source.crops[0].wateredDays = 255;
    if (Homestead::EncodeSave(source, bytes)) return 13;
    source.crops[0].wateredDays = 2; source.crops[0].stage = 1;
    if (Homestead::EncodeSave(source, bytes)) return 14;
    const auto legacy=LegacySave(); Homestead::SaveSnapshot migrated;
    if(!Homestead::DecodeSave(legacy.data(),legacy.size(),migrated)||migrated.gold!=20||
       migrated.inventory[0].item!=Homestead::ItemId::Hoe||
       migrated.inventory[2].item!=Homestead::ItemId::CarrotSeed||migrated.crops.size()!=1||
       migrated.crops[0].crop!=Homestead::CropId::Carrot||migrated.crops[0].stage!=2||
       migrated.crops[0].wateredDays!=2)return 15;
    auto invalidLegacy=legacy;invalidLegacy[16+13]=4;RepairChecksum(invalidLegacy);
    if(!Rejects(invalidLegacy))return 16;
    return 0;
}

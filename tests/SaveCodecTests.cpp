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

} // namespace

int main() {
    Homestead::SaveSnapshot source;
    source.playerX256 = 12345; source.playerY256 = 23456;
    source.day = 4; source.minute = 777; source.selectedSlot = 2; source.harvestedCarrots = 1;
    source.inventory[0] = {Homestead::ItemId::Hoe, 1};
    source.inventory[1] = {Homestead::ItemId::WateringCan, 1};
    source.inventory[2] = {Homestead::ItemId::CarrotSeed, 9};
    source.tileDeltas.push_back({3, 4, Homestead::TileFlagValue(Homestead::TileFlag::Tilled)});
    source.crops.push_back({3, 4, Homestead::CropId::Carrot, 2, true});
    std::vector<std::uint8_t> bytes;
    if (!Homestead::EncodeSave(source, bytes) || bytes.size() >= Homestead::MaximumSaveBytes) return 1;
    Homestead::SaveSnapshot decoded;
    if (!Homestead::DecodeSave(bytes.data(), bytes.size(), decoded) || decoded.day != 4 ||
        decoded.minute != 777 || decoded.inventory[2].count != 9 ||
        decoded.tileDeltas.size() != 1 || decoded.crops.size() != 1 || decoded.crops[0].stage != 2) return 2;

    auto damaged = bytes; damaged[0] = 'X'; if (!Rejects(damaged)) return 3;
    damaged = bytes; SetU16(damaged, 4, 2); if (!Rejects(damaged)) return 4;
    damaged = bytes; damaged.pop_back(); if (!Rejects(damaged)) return 5;
    damaged = bytes; damaged.back() ^= 1; if (!Rejects(damaged)) return 6;
    damaged = bytes; damaged[34] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 7;
    damaged = bytes; SetU16(damaged, 30, 5000); RepairChecksum(damaged); if (!Rejects(damaged)) return 8;
    damaged = bytes; damaged[68] = 0x80; RepairChecksum(damaged); if (!Rejects(damaged)) return 9;
    damaged = bytes; damaged[71] = 0xFF; RepairChecksum(damaged); if (!Rejects(damaged)) return 10;
    damaged = bytes; damaged.resize(Homestead::MaximumSaveBytes + 1); if (!Rejects(damaged)) return 11;

    source.inventory[2].count = 100;
    if (Homestead::EncodeSave(source, bytes)) return 12;
    return 0;
}

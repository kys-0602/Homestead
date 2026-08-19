#include "Homestead/World/TileMap.hpp"

#include <cstddef>
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

void SetU16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value);
    bytes[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}

std::vector<std::uint8_t> MakeMap(std::uint16_t width, std::uint16_t height) {
    std::vector<std::uint8_t> bytes{'H', 'S', 'T', 'M'};
    U16(bytes, 1);
    U16(bytes, 24);
    U16(bytes, width);
    U16(bytes, height);
    U16(bytes, 16);
    U16(bytes, 16);
    U16(bytes, 3);
    U16(bytes, 6);
    U32(bytes, static_cast<std::uint32_t>(width) * height);
    for (std::uint32_t index = 0; index < static_cast<std::uint32_t>(width) * height; ++index) {
        U16(bytes, 1);
        U16(bytes, index == 16 ? 4 : 0);
        bytes.push_back(index == 16 ? Homestead::TileFlagValue(Homestead::TileFlag::Blocked) : 0);
        bytes.push_back(0);
    }
    return bytes;
}

bool Loads(const std::vector<std::uint8_t>& bytes) {
    Homestead::TileMap map;
    return map.LoadMemory(bytes.data(), bytes.size());
}

} // namespace

int main() {
    const std::vector<std::uint8_t> valid = MakeMap(17, 17);
    Homestead::TileMap map;
    if (!map.LoadMemory(valid.data(), valid.size()) || map.Width() != 17 || map.Height() != 17 ||
        map.ChunkColumns() != 2 || map.ChunkRows() != 2 || map.Get(-1, 0) != nullptr ||
        map.Get(17, 0) != nullptr || map.Get(0, 17) != nullptr || map.Get(16, 0) == nullptr ||
        map.Get(16, 0)->object != 4) {
        return 1;
    }

    auto damaged = valid;
    damaged[0] = 'X';
    if (Loads(damaged)) return 2;
    damaged = valid;
    SetU16(damaged, 4, 2);
    if (Loads(damaged)) return 3;
    damaged = valid;
    damaged.pop_back();
    if (Loads(damaged)) return 4;
    damaged = valid;
    SetU16(damaged, 8, 0);
    if (Loads(damaged)) return 5;
    damaged = valid;
    SetU16(damaged, 8, Homestead::MaximumMapTiles + 1U);
    if (Loads(damaged)) return 6;
    damaged = valid;
    SetU16(damaged, 12, 8);
    if (Loads(damaged)) return 7;
    damaged = valid;
    SetU16(damaged, 24, static_cast<std::uint16_t>(Homestead::TileGraphic::Count));
    if (Loads(damaged)) return 8;
    damaged = valid;
    damaged[28] = 0x80;
    if (Loads(damaged)) return 9;
    return 0;
}

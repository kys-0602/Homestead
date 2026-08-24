#include "Homestead/Assets/AssetStore.hpp"

#include <algorithm>
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
    bytes[offset + 1U] = static_cast<std::uint8_t>(value >> 8U);
}

void SetU32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value >> 8U);
    bytes[offset + 2U] = static_cast<std::uint8_t>(value >> 16U);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value >> 24U);
}

std::uint32_t Checksum(const std::uint8_t* data, std::size_t size) {
    std::uint32_t value = 2166136261U;
    for (std::size_t index = 0; index < size; ++index) {
        value ^= data[index];
        value *= 16777619U;
    }
    return value;
}

void RefreshGlobalChecksum(std::vector<std::uint8_t>& pak) {
    SetU32(pak, 24, Checksum(pak.data() + 32, pak.size() - 32));
}

std::size_t FindEntry(const std::vector<std::uint8_t>& pak, std::uint16_t type) {
    const std::size_t entryCount = static_cast<std::uint32_t>(pak[8]) |
        (static_cast<std::uint32_t>(pak[9]) << 8U) |
        (static_cast<std::uint32_t>(pak[10]) << 16U) |
        (static_cast<std::uint32_t>(pak[11]) << 24U);
    for (std::size_t offset = 32; offset < 32 + entryCount * 24; offset += 24) {
        const std::uint16_t entryType = static_cast<std::uint16_t>(pak[offset + 4U]) |
            static_cast<std::uint16_t>(pak[offset + 5U] << 8U);
        if (entryType == type) {
            return offset;
        }
    }
    return 0;
}

std::uint32_t ReadU32(const std::vector<std::uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1U]) << 8U) |
        (static_cast<std::uint32_t>(bytes[offset + 2U]) << 16U) |
        (static_cast<std::uint32_t>(bytes[offset + 3U]) << 24U);
}

void RefreshEntryChecksum(std::vector<std::uint8_t>& pak, std::size_t entry) {
    const std::uint32_t offset = ReadU32(pak, entry + 8U);
    const std::uint32_t size = ReadU32(pak, entry + 12U);
    SetU32(pak, entry + 20U, Checksum(pak.data() + offset, size));
    RefreshGlobalChecksum(pak);
}

std::vector<std::uint8_t> MakeValidPak() {
    std::vector<std::uint8_t> atlas{'H', 'S', 'P', 'A'};
    U16(atlas, 1);
    U16(atlas, 1);
    U16(atlas, 1);
    U16(atlas, 1);
    atlas.insert(atlas.end(), {255, 255, 255, 255, 0});

    std::vector<std::uint8_t> sprites;
    U32(sprites, 1);
    U32(sprites, Homestead::MakeAssetId("terrain.grass"));
    for (int index = 0; index < 4; ++index) {
        U16(sprites, index < 2 ? 0 : 1);
    }
    U16(sprites, 0);
    U16(sprites, 0);
    U16(sprites, 1);

    std::vector<std::uint8_t> map{'H', 'S', 'T', 'M'};
    U16(map, 1);
    U16(map, 24);
    U16(map, 1);
    U16(map, 1);
    U16(map, 16);
    U16(map, 16);
    U16(map, 3);
    U16(map, 6);
    U32(map, 1);
    U16(map, 1);
    U16(map, 0);
    map.push_back(0);
    map.push_back(0);
    U16(sprites, 1);

    struct Entry {
        std::uint32_t id;
        std::uint16_t type;
        const std::vector<std::uint8_t>* payload;
        std::uint32_t offset;
    };
    std::vector<Entry> entries{
        {Homestead::MakeAssetId("atlas/main"), 1, &atlas, 0},
        {Homestead::MakeAssetId("sprites/main"), 2, &sprites, 0},
        {Homestead::MakeAssetId("map/farm"), 3, &map, 0},
        {Homestead::MakeAssetId("map/house"), 3, &map, 0}};
    std::sort(entries.begin(), entries.end(), [](const Entry& left, const Entry& right) {
        return left.id < right.id;
    });
    std::uint32_t offset = 128;
    for (Entry& entry : entries) {
        entry.offset = offset;
        offset += static_cast<std::uint32_t>(entry.payload->size());
        offset = (offset + 3U) & ~3U;
    }
    const std::uint32_t fileSize = entries.back().offset +
        static_cast<std::uint32_t>(entries.back().payload->size());
    std::vector<std::uint8_t> pak{'H', 'S', 'P', 'K'};
    U16(pak, 1);
    U16(pak, 32);
    U32(pak, 4);
    U32(pak, 32);
    U32(pak, 128);
    U32(pak, fileSize);
    U32(pak, 0);
    U32(pak, 0);
    for (const Entry& entry : entries) {
        U32(pak, entry.id);
        U16(pak, entry.type);
        U16(pak, 0);
        U32(pak, entry.offset);
        U32(pak, static_cast<std::uint32_t>(entry.payload->size()));
        U32(pak, static_cast<std::uint32_t>(entry.payload->size()));
        U32(pak, Checksum(entry.payload->data(), entry.payload->size()));
    }
    for (const Entry& entry : entries) {
        pak.resize(entry.offset, 0);
        pak.insert(pak.end(), entry.payload->begin(), entry.payload->end());
    }
    RefreshGlobalChecksum(pak);
    return pak;
}

bool Loads(const std::vector<std::uint8_t>& pak) {
    Homestead::AssetStore store;
    return store.LoadMemory(pak.data(), pak.size());
}

} // namespace

int main() {
    const std::vector<std::uint8_t> valid = MakeValidPak();
    Homestead::AssetStore store;
    if (!store.LoadMemory(valid.data(), valid.size()) || store.AtlasWidth() != 1 ||
        store.AtlasHeight() != 1 || store.SpriteCount() != 1 ||
        store.FindMap(Homestead::MakeAssetId("map/farm")) == nullptr ||
        store.FindMap(Homestead::MakeAssetId("map/house")) == nullptr ||
        store.FindSprite(Homestead::MakeAssetId("terrain.grass")) == nullptr) {
        return 1;
    }

    auto damaged = valid;
    damaged[0] = 'X';
    if (Loads(damaged)) {
        return 2;
    }
    damaged = valid;
    SetU16(damaged, 4, 2);
    if (Loads(damaged)) {
        return 3;
    }
    damaged = valid;
    damaged.pop_back();
    if (Loads(damaged)) {
        return 4;
    }
    damaged = valid;
    damaged.back() ^= 1U;
    if (Loads(damaged)) {
        return 5;
    }
    damaged = valid;
    SetU32(damaged, 40, 0xFFFFFFF0U);
    RefreshGlobalChecksum(damaged);
    if (Loads(damaged)) {
        return 6;
    }
    damaged = valid;
    SetU32(damaged, 8, 33);
    if (Loads(damaged)) {
        return 7;
    }
    damaged = valid;
    const std::size_t atlasEntry = FindEntry(damaged, 1);
    SetU32(damaged, atlasEntry, 0);
    RefreshGlobalChecksum(damaged);
    if (Loads(damaged)) {
        return 8;
    }
    damaged = valid;
    const std::uint32_t atlasOffset = ReadU32(damaged, atlasEntry + 8U);
    damaged[atlasOffset + 16U] = 1;
    RefreshEntryChecksum(damaged, atlasEntry);
    if (Loads(damaged)) {
        return 9;
    }
    damaged = valid;
    const std::size_t spriteEntry = FindEntry(damaged, 2);
    SetU32(damaged, spriteEntry + 8U, ReadU32(damaged, atlasEntry + 8U));
    SetU32(damaged, spriteEntry + 12U, ReadU32(damaged, atlasEntry + 12U));
    SetU32(damaged, spriteEntry + 16U, ReadU32(damaged, atlasEntry + 12U));
    SetU32(damaged, spriteEntry + 20U, ReadU32(damaged, atlasEntry + 20U));
    RefreshGlobalChecksum(damaged);
    if (Loads(damaged)) {
        return 10;
    }
    damaged = valid;
    SetU32(damaged, 16, 81);
    if (Loads(damaged)) {
        return 11;
    }
    damaged = valid;
    SetU32(damaged, atlasEntry + 8U, ReadU32(damaged, atlasEntry + 8U) + 1U);
    RefreshGlobalChecksum(damaged);
    if (Loads(damaged)) {
        return 12;
    }
    return 0;
}

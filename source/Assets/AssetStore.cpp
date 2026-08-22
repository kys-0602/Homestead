#include "Homestead/Assets/AssetStore.hpp"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <limits>

namespace Homestead {
namespace {

constexpr std::size_t PakHeaderSize = 32;
constexpr std::size_t PakEntrySize = 24;
constexpr std::uint32_t MaxPakEntries = 32;
constexpr std::uint32_t MaxSprites = 2048;

std::uint16_t ReadU16(const std::uint8_t* data) noexcept {
    return static_cast<std::uint16_t>(data[0]) |
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(data[1]) << 8U);
}

std::uint32_t ReadU32(const std::uint8_t* data) noexcept {
    return static_cast<std::uint32_t>(data[0]) |
        (static_cast<std::uint32_t>(data[1]) << 8U) |
        (static_cast<std::uint32_t>(data[2]) << 16U) |
        (static_cast<std::uint32_t>(data[3]) << 24U);
}

std::uint32_t Checksum(const std::uint8_t* data, std::size_t size) noexcept {
    std::uint32_t value = 2166136261U;
    for (std::size_t index = 0; index < size; ++index) {
        value ^= data[index];
        value *= 16777619U;
    }
    return value;
}

bool HasRange(std::size_t size, std::uint32_t offset, std::uint32_t length) noexcept {
    return offset <= size && length <= size - offset;
}

struct EntryView {
    AssetId id = 0;
    std::uint16_t type = 0;
    std::uint32_t offset = 0;
    std::uint32_t size = 0;
};

} // namespace

bool AssetStore::LoadFile(const wchar_t* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    HANDLE file = CreateFileW(
        path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    LARGE_INTEGER length{};
    if (!GetFileSizeEx(file, &length) || length.QuadPart <= 0 ||
        static_cast<std::uint64_t>(length.QuadPart) > std::numeric_limits<std::uint32_t>::max()) {
        CloseHandle(file);
        return false;
    }
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(length.QuadPart));
    DWORD read = 0;
    const bool readOk = ReadFile(
        file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) != FALSE;
    CloseHandle(file);
    return readOk && static_cast<std::size_t>(read) == bytes.size() &&
        LoadMemory(bytes.data(), bytes.size());
}

bool AssetStore::LoadMemory(const std::uint8_t* data, std::size_t size) noexcept {
    Clear();
    if (data == nullptr || size < PakHeaderSize ||
        data[0] != 'H' || data[1] != 'S' || data[2] != 'P' || data[3] != 'K' ||
        ReadU16(data + 4) != 1 || ReadU16(data + 6) != PakHeaderSize ||
        ReadU32(data + 20) != size || ReadU32(data + 28) != 0) {
        return false;
    }
    const std::uint32_t entryCount = ReadU32(data + 8);
    const std::uint32_t indexOffset = ReadU32(data + 12);
    const std::uint32_t payloadOffset = ReadU32(data + 16);
    if (entryCount == 0 || entryCount > MaxPakEntries || indexOffset != PakHeaderSize ||
        entryCount > (std::numeric_limits<std::uint32_t>::max() - indexOffset) / PakEntrySize) {
        return false;
    }
    const std::uint32_t indexEnd = indexOffset + entryCount * PakEntrySize;
    if ((payloadOffset & 3U) != 0U || payloadOffset < indexEnd || payloadOffset > size ||
        ReadU32(data + 24) != Checksum(data + PakHeaderSize, size - PakHeaderSize)) {
        return false;
    }

    EntryView atlasEntry{};
    EntryView spriteEntry{};
    EntryView mapEntry{};
    std::array<EntryView, MaxPakEntries> entries{};
    AssetId previousId = 0;
    for (std::uint32_t index = 0; index < entryCount; ++index) {
        const std::uint8_t* entry = data + indexOffset + index * PakEntrySize;
        EntryView view{};
        view.id = ReadU32(entry);
        view.type = ReadU16(entry + 4);
        const std::uint16_t flags = ReadU16(entry + 6);
        view.offset = ReadU32(entry + 8);
        view.size = ReadU32(entry + 12);
        const std::uint32_t rawSize = ReadU32(entry + 16);
        const std::uint32_t checksum = ReadU32(entry + 20);
        if ((index != 0 && view.id <= previousId) || flags != 0 || view.size == 0 ||
            rawSize != view.size || (view.offset & 3U) != 0U ||
            view.offset < payloadOffset ||
            !HasRange(size, view.offset, view.size) ||
            checksum != Checksum(data + view.offset, view.size)) {
            return false;
        }
        for (std::uint32_t prior = 0; prior < index; ++prior) {
            const std::uint64_t priorEnd =
                static_cast<std::uint64_t>(entries[prior].offset) + entries[prior].size;
            const std::uint64_t viewEnd = static_cast<std::uint64_t>(view.offset) + view.size;
            if (view.offset < priorEnd && entries[prior].offset < viewEnd) {
                return false;
            }
        }
        entries[index] = view;
        previousId = view.id;
        if (view.type == 1 && view.id == MakeAssetId("atlas/main") && atlasEntry.size == 0) {
            atlasEntry = view;
        } else if (
            view.type == 2 && view.id == MakeAssetId("sprites/main") &&
            spriteEntry.size == 0) {
            spriteEntry = view;
        } else if (
            view.type == 3 && view.id == MakeAssetId("map/farm") &&
            mapEntry.size == 0) {
            mapEntry = view;
        } else {
            return false;
        }
    }
    if (atlasEntry.size < 12 || spriteEntry.size < 4 || mapEntry.size < 24) {
        return false;
    }

    const std::uint8_t* atlas = data + atlasEntry.offset;
    if (atlas[0] != 'H' || atlas[1] != 'S' || atlas[2] != 'P' || atlas[3] != 'A' ||
        ReadU16(atlas + 4) != 1) {
        return false;
    }
    const std::uint16_t width = ReadU16(atlas + 6);
    const std::uint16_t height = ReadU16(atlas + 8);
    const std::uint16_t colorCount = ReadU16(atlas + 10);
    const std::uint64_t pixelCount = static_cast<std::uint64_t>(width) * height;
    const std::uint64_t expectedAtlasSize = 12ULL + colorCount * 4ULL + pixelCount;
    if (width == 0 || height == 0 || width > 4096 || height > 4096 ||
        colorCount == 0 || colorCount > 256 || expectedAtlasSize != atlasEntry.size) {
        return false;
    }
    atlasPixels_.resize(static_cast<std::size_t>(pixelCount) * 4U);
    const std::uint8_t* palette = atlas + 12;
    const std::uint8_t* indices = palette + colorCount * 4U;
    for (std::size_t pixel = 0; pixel < pixelCount; ++pixel) {
        const std::uint8_t paletteIndex = indices[pixel];
        if (paletteIndex >= colorCount) {
            Clear();
            return false;
        }
        std::copy_n(palette + paletteIndex * 4U, 4U, atlasPixels_.data() + pixel * 4U);
    }

    const std::uint8_t* spriteData = data + spriteEntry.offset;
    const std::uint32_t spriteCount = ReadU32(spriteData);
    if (spriteCount == 0 || spriteCount > MaxSprites ||
        4ULL + static_cast<std::uint64_t>(spriteCount) * 20ULL != spriteEntry.size) {
        Clear();
        return false;
    }
    sprites_.resize(spriteCount);
    previousId = 0;
    for (std::uint32_t index = 0; index < spriteCount; ++index) {
        const std::uint8_t* record = spriteData + 4U + index * 20U;
        SpriteAsset& sprite = sprites_[index];
        sprite.id = ReadU32(record);
        sprite.x = ReadU16(record + 4);
        sprite.y = ReadU16(record + 6);
        sprite.width = ReadU16(record + 8);
        sprite.height = ReadU16(record + 10);
        sprite.trimX = ReadU16(record + 12);
        sprite.trimY = ReadU16(record + 14);
        sprite.sourceWidth = ReadU16(record + 16);
        sprite.sourceHeight = ReadU16(record + 18);
        if ((index != 0 && sprite.id <= previousId) || sprite.width == 0 ||
            sprite.height == 0 || sprite.x > width || sprite.y > height ||
            sprite.width > width - sprite.x || sprite.height > height - sprite.y ||
            sprite.trimX > sprite.sourceWidth || sprite.trimY > sprite.sourceHeight ||
            sprite.width > sprite.sourceWidth - sprite.trimX ||
            sprite.height > sprite.sourceHeight - sprite.trimY) {
            Clear();
            return false;
        }
        previousId = sprite.id;
    }
    atlasWidth_ = width;
    atlasHeight_ = height;
    mapBytes_.assign(data + mapEntry.offset, data + mapEntry.offset + mapEntry.size);
    return true;
}

const SpriteAsset* AssetStore::FindSprite(AssetId id) const noexcept {
    const auto found = std::lower_bound(
        sprites_.begin(), sprites_.end(), id,
        [](const SpriteAsset& sprite, AssetId value) { return sprite.id < value; });
    return found != sprites_.end() && found->id == id ? &*found : nullptr;
}

void AssetStore::Clear() noexcept {
    atlasPixels_.clear();
    sprites_.clear();
    mapBytes_.clear();
    atlasWidth_ = 0;
    atlasHeight_ = 0;
}

} // namespace Homestead

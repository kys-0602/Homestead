#include "Homestead/World/TileMap.hpp"

#include <limits>

namespace Homestead {
namespace {

constexpr std::size_t MapHeaderSize = 24;
constexpr std::size_t TileRecordSize = 6;

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

bool IsValidGraphic(std::uint16_t value) noexcept {
    return value < static_cast<std::uint16_t>(TileGraphic::Count);
}

} // namespace

bool TileMap::LoadMemory(const std::uint8_t* data, std::size_t size) noexcept {
    Clear();
    if (data == nullptr || size < MapHeaderSize ||
        data[0] != 'H' || data[1] != 'S' || data[2] != 'T' || data[3] != 'M' ||
        ReadU16(data + 4) != 1 || ReadU16(data + 6) != MapHeaderSize) {
        return false;
    }

    const std::uint16_t width = ReadU16(data + 8);
    const std::uint16_t height = ReadU16(data + 10);
    const std::uint16_t tileSize = ReadU16(data + 12);
    const std::uint16_t chunkSize = ReadU16(data + 14);
    const std::uint16_t layerCount = ReadU16(data + 16);
    const std::uint16_t recordSize = ReadU16(data + 18);
    const std::uint32_t tileCount = ReadU32(data + 20);
    if (width == 0 || height == 0 || width > MaximumMapTiles || height > MaximumMapTiles ||
        tileSize != TileSize || chunkSize != TileChunkSize || layerCount != 3 ||
        recordSize != TileRecordSize || tileCount != static_cast<std::uint32_t>(width) * height ||
        tileCount > (std::numeric_limits<std::size_t>::max() - MapHeaderSize) / TileRecordSize ||
        MapHeaderSize + static_cast<std::size_t>(tileCount) * TileRecordSize != size) {
        return false;
    }

    const std::uint16_t chunkColumns = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(width) + TileChunkSize - 1U) / TileChunkSize);
    const std::uint16_t chunkRows = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(height) + TileChunkSize - 1U) / TileChunkSize);
    chunks_.resize(static_cast<std::size_t>(chunkColumns) * chunkRows);

    constexpr std::uint8_t validFlags = TileFlagValue(TileFlag::Blocked) |
        TileFlagValue(TileFlag::Water) | TileFlagValue(TileFlag::Tilled) |
        TileFlagValue(TileFlag::Watered);
    for (std::uint32_t index = 0; index < tileCount; ++index) {
        const std::uint8_t* record = data + MapHeaderSize + index * TileRecordSize;
        Tile tile{};
        tile.ground = ReadU16(record);
        tile.object = ReadU16(record + 2);
        tile.flags = record[4];
        tile.variant = record[5];
        const bool tilled = (tile.flags & TileFlagValue(TileFlag::Tilled)) != 0;
        const bool watered = (tile.flags & TileFlagValue(TileFlag::Watered)) != 0;
        if (!IsValidGraphic(tile.ground) || tile.ground == 0 ||
            !IsValidGraphic(tile.object) || (tile.flags & ~validFlags) != 0 ||
            (watered && !tilled) || tile.variant != 0) {
            Clear();
            return false;
        }

        const std::uint16_t x = static_cast<std::uint16_t>(index % width);
        const std::uint16_t y = static_cast<std::uint16_t>(index / width);
        const std::size_t chunkIndex =
            static_cast<std::size_t>(y / TileChunkSize) * chunkColumns + x / TileChunkSize;
        const std::size_t tileIndex =
            static_cast<std::size_t>(y % TileChunkSize) * TileChunkSize + x % TileChunkSize;
        chunks_[chunkIndex].tiles[tileIndex] = tile;
    }

    width_ = width;
    height_ = height;
    chunkColumns_ = chunkColumns;
    chunkRows_ = chunkRows;
    return true;
}

void TileMap::Clear() noexcept {
    chunks_.clear();
    width_ = 0;
    height_ = 0;
    chunkColumns_ = 0;
    chunkRows_ = 0;
}

const Tile* TileMap::Get(std::int32_t x, std::int32_t y) const noexcept {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return nullptr;
    }
    const std::size_t chunkIndex =
        static_cast<std::size_t>(y / TileChunkSize) * chunkColumns_ +
        static_cast<std::size_t>(x / TileChunkSize);
    const std::size_t tileIndex =
        static_cast<std::size_t>(y % TileChunkSize) * TileChunkSize +
        static_cast<std::size_t>(x % TileChunkSize);
    return &chunks_[chunkIndex].tiles[tileIndex];
}

Tile* TileMap::Get(std::int32_t x, std::int32_t y) noexcept {
    return const_cast<Tile*>(static_cast<const TileMap&>(*this).Get(x, y));
}

} // namespace Homestead

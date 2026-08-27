#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Homestead {

inline constexpr std::uint16_t TileSize = 16;
inline constexpr std::uint16_t TileChunkSize = 16;
inline constexpr std::uint16_t MaximumMapTiles = 128;

enum class TileGraphic : std::uint16_t {
    None,
    Grass,
    Path,
    FlowerWhite,
    OakTree,
    FenceHorizontal,
    FenceVertical,
    Sign,
    MarketSign,
    FarmlandDry,
    FarmlandWet,
    Farmhouse,
    WoodFloor,
    InteriorWall,
    Bed,
    Door,
    CampDecor,
    Scarecrow,
    Bookshelf,
    Count
};

enum class TileFlag : std::uint8_t {
    Blocked = 1U << 0U,
    Water = 1U << 1U,
    Tilled = 1U << 2U,
    Watered = 1U << 3U
};

struct Tile {
    std::uint16_t ground = 0;
    std::uint16_t object = 0;
    std::uint8_t flags = 0;
    std::uint8_t variant = 0;
};

class TileMap final {
public:
    [[nodiscard]] bool LoadMemory(const std::uint8_t* data, std::size_t size) noexcept;
    void Clear() noexcept;

    [[nodiscard]] const Tile* Get(std::int32_t x, std::int32_t y) const noexcept;
    [[nodiscard]] Tile* Get(std::int32_t x, std::int32_t y) noexcept;
    [[nodiscard]] std::uint16_t Width() const noexcept { return width_; }
    [[nodiscard]] std::uint16_t Height() const noexcept { return height_; }
    [[nodiscard]] std::uint16_t ChunkColumns() const noexcept { return chunkColumns_; }
    [[nodiscard]] std::uint16_t ChunkRows() const noexcept { return chunkRows_; }

private:
    struct Chunk {
        std::array<Tile, TileChunkSize * TileChunkSize> tiles{};
    };

    std::vector<Chunk> chunks_;
    std::uint16_t width_ = 0;
    std::uint16_t height_ = 0;
    std::uint16_t chunkColumns_ = 0;
    std::uint16_t chunkRows_ = 0;
};

[[nodiscard]] constexpr std::uint8_t TileFlagValue(TileFlag flag) noexcept {
    return static_cast<std::uint8_t>(flag);
}

} // namespace Homestead

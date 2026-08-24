#include "Map.hpp"

#include <charconv>
#include <fstream>
#include <string>
#include <vector>

#include "Homestead/World/TileMap.hpp"

namespace Homestead::AssetPacker {
namespace {

void WriteU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
}

void WriteU32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 16U));
    bytes.push_back(static_cast<std::uint8_t>(value >> 24U));
}

bool ParseDimension(const std::string& text, std::uint16_t& value) {
    std::uint32_t parsed = 0;
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (text.empty() || result.ec != std::errc{} || result.ptr != end ||
        parsed == 0 || parsed > MaximumMapTiles) {
        return false;
    }
    value = static_cast<std::uint16_t>(parsed);
    return true;
}

bool DecodeTile(char symbol, Tile& tile) {
    tile.ground = static_cast<std::uint16_t>(TileGraphic::Grass);
    switch (symbol) {
    case '.': return true;
    case '=':
        tile.ground = static_cast<std::uint16_t>(TileGraphic::Path);
        return true;
    case 'f':
        tile.object = static_cast<std::uint16_t>(TileGraphic::FlowerWhite);
        return true;
    case 'T':
        tile.object = static_cast<std::uint16_t>(TileGraphic::OakTree);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case '-':
        tile.object = static_cast<std::uint16_t>(TileGraphic::FenceHorizontal);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case '|':
        tile.object = static_cast<std::uint16_t>(TileGraphic::FenceVertical);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case 's':
        tile.object = static_cast<std::uint16_t>(TileGraphic::Sign);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case '#':
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case 'H':
        tile.object = static_cast<std::uint16_t>(TileGraphic::Farmhouse);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case ',':
        tile.ground = static_cast<std::uint16_t>(TileGraphic::WoodFloor);
        return true;
    case 'w':
        tile.ground = static_cast<std::uint16_t>(TileGraphic::WoodFloor);
        tile.object = static_cast<std::uint16_t>(TileGraphic::InteriorWall);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case 'b':
        tile.ground = static_cast<std::uint16_t>(TileGraphic::WoodFloor);
        tile.object = static_cast<std::uint16_t>(TileGraphic::Bed);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    case 'd':
        tile.ground = static_cast<std::uint16_t>(TileGraphic::WoodFloor);
        tile.object = static_cast<std::uint16_t>(TileGraphic::Door);
        tile.flags = TileFlagValue(TileFlag::Blocked);
        return true;
    default:
        return false;
    }
}

} // namespace

bool BuildMapPayload(
    const std::filesystem::path& path,
    std::vector<std::uint8_t>& bytes,
    MapStats& stats,
    std::string& error) {
    bytes.clear();
    stats = {};
    std::ifstream stream(path);
    if (!stream) {
        error = "cannot open map " + path.string();
        return false;
    }

    std::string header;
    if (!std::getline(stream, header)) {
        error = "map is empty";
        return false;
    }
    const std::size_t firstTab = header.find('\t');
    const std::size_t secondTab = firstTab == std::string::npos ?
        std::string::npos : header.find('\t', firstTab + 1U);
    if (firstTab == std::string::npos || secondTab == std::string::npos ||
        header.substr(0, firstTab) != "HSM1" ||
        !ParseDimension(header.substr(firstTab + 1U, secondTab - firstTab - 1U), stats.width) ||
        !ParseDimension(header.substr(secondTab + 1U), stats.height)) {
        error = "invalid map header";
        return false;
    }

    std::vector<Tile> tiles;
    tiles.reserve(static_cast<std::size_t>(stats.width) * stats.height);
    std::string row;
    for (std::uint16_t y = 0; y < stats.height; ++y) {
        if (!std::getline(stream, row) || row.size() != stats.width) {
            error = "map row has an invalid width at y=" + std::to_string(y);
            return false;
        }
        for (std::uint16_t x = 0; x < stats.width; ++x) {
            Tile tile{};
            if (!DecodeTile(row[x], tile)) {
                error = "unknown map symbol at x=" + std::to_string(x) +
                    ", y=" + std::to_string(y);
                return false;
            }
            tiles.push_back(tile);
        }
    }
    while (std::getline(stream, row)) {
        if (!row.empty()) {
            error = "map contains extra rows";
            return false;
        }
    }

    bytes.insert(bytes.end(), {'H', 'S', 'T', 'M'});
    WriteU16(bytes, 1);
    WriteU16(bytes, 24);
    WriteU16(bytes, stats.width);
    WriteU16(bytes, stats.height);
    WriteU16(bytes, TileSize);
    WriteU16(bytes, TileChunkSize);
    WriteU16(bytes, 3);
    WriteU16(bytes, 6);
    WriteU32(bytes, static_cast<std::uint32_t>(tiles.size()));
    for (const Tile& tile : tiles) {
        WriteU16(bytes, tile.ground);
        WriteU16(bytes, tile.object);
        bytes.push_back(tile.flags);
        bytes.push_back(tile.variant);
    }
    stats.byteCount = bytes.size();
    return true;
}

} // namespace Homestead::AssetPacker

#include "Pak.hpp"

#include "Map.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Homestead::AssetPacker {
namespace {

constexpr std::uint16_t PakVersion = 1;
constexpr std::uint16_t PakHeaderSize = 32;
constexpr std::uint32_t PakEntrySize = 24;

struct Payload {
    std::uint32_t id = 0;
    std::uint16_t type = 0;
    std::vector<std::uint8_t> bytes;
};

std::uint32_t Hash(std::string_view text) {
    std::uint32_t value = 2166136261U;
    for (const char character : text) {
        value ^= static_cast<std::uint8_t>(character);
        value *= 16777619U;
    }
    return value;
}

std::uint32_t Checksum(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t value = 2166136261U;
    for (const std::uint8_t byte : bytes) {
        value ^= byte;
        value *= 16777619U;
    }
    return value;
}

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

bool ToU16(const std::string& text, std::uint16_t& value) {
    std::uint32_t parsed = 0;
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto result = std::from_chars(begin, end, parsed);
    if (text.empty() || result.ec != std::errc{} || result.ptr != end || parsed > UINT16_MAX) {
        return false;
    }
    value = static_cast<std::uint16_t>(parsed);
    return true;
}

std::vector<std::string> Split(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t begin = 0;
    for (;;) {
        const std::size_t end = line.find('\t', begin);
        fields.emplace_back(line.substr(begin, end - begin));
        if (end == std::string::npos) {
            return fields;
        }
        begin = end + 1;
    }
}

bool ReadFile(
    const std::filesystem::path& path,
    std::vector<std::uint8_t>& bytes,
    std::string& error) {
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream) {
        error = "cannot open payload " + path.string();
        return false;
    }
    const std::streamoff length = stream.tellg();
    if (length < 0 || static_cast<std::uintmax_t>(length) > UINT32_MAX) {
        error = "payload is too large " + path.string();
        return false;
    }
    bytes.resize(static_cast<std::size_t>(length));
    stream.seekg(0);
    if (!bytes.empty()) {
        stream.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }
    if (!stream) {
        error = "cannot read payload " + path.string();
        return false;
    }
    return true;
}

bool BuildSpriteTable(
    const std::filesystem::path& path,
    std::vector<std::uint8_t>& bytes,
    std::size_t& spriteCount,
    std::string& error) {
    std::ifstream stream(path);
    if (!stream) {
        error = "cannot open atlas metadata";
        return false;
    }
    std::string line;
    if (!std::getline(stream, line) ||
        line != "name\tx\ty\twidth\theight\ttrim_x\ttrim_y\tsource_width\tsource_height") {
        error = "invalid atlas metadata header";
        return false;
    }
    struct Record {
        std::uint32_t id = 0;
        std::array<std::uint16_t, 8> values{};
    };
    std::vector<Record> records;
    std::unordered_map<std::uint32_t, std::string> ids;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = Split(line);
        if (fields.size() != 9) {
            error = "atlas metadata row must contain 9 fields";
            return false;
        }
        Record record{};
        record.id = Hash(fields[0]);
        const auto existing = ids.find(record.id);
        if (existing != ids.end() && existing->second != fields[0]) {
            error = "AssetId collision between " + existing->second + " and " + fields[0];
            return false;
        }
        ids.emplace(record.id, fields[0]);
        for (std::size_t index = 0; index < record.values.size(); ++index) {
            if (!ToU16(fields[index + 1U], record.values[index])) {
                error = "invalid atlas metadata value for " + fields[0];
                return false;
            }
        }
        records.push_back(record);
    }
    std::sort(records.begin(), records.end(), [](const Record& left, const Record& right) {
        return left.id < right.id;
    });
    WriteU32(bytes, static_cast<std::uint32_t>(records.size()));
    for (const Record& record : records) {
        WriteU32(bytes, record.id);
        for (const std::uint16_t value : record.values) {
            WriteU16(bytes, value);
        }
    }
    spriteCount = records.size();
    return true;
}

} // namespace

bool BuildPak(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& atlasDirectory,
    const std::filesystem::path& outputPath,
    PakStats& stats,
    std::string& error) {
    stats = {};
    std::vector<Payload> payloads;
    Payload atlas{};
    atlas.id = Hash("atlas/main");
    atlas.type = 1;
    if (!ReadFile(atlasDirectory / "atlas.pal", atlas.bytes, error)) {
        return false;
    }
    payloads.push_back(std::move(atlas));
    Payload sprites{};
    sprites.id = Hash("sprites/main");
    sprites.type = 2;
    if (!BuildSpriteTable(
            atlasDirectory / "atlas.tsv", sprites.bytes, stats.spriteCount, error)) {
        return false;
    }
    payloads.push_back(std::move(sprites));
    Payload map{};
    map.id = Hash("map/farm");
    map.type = 3;
    MapStats mapStats{};
    if (!BuildMapPayload(assetRoot / "maps" / "farm.map", map.bytes, mapStats, error)) {
        return false;
    }
    stats.mapBytes = mapStats.byteCount;
    payloads.push_back(std::move(map));
    std::sort(payloads.begin(), payloads.end(), [](const Payload& left, const Payload& right) {
        return left.id < right.id;
    });
    for (std::size_t index = 1; index < payloads.size(); ++index) {
        if (payloads[index - 1].id == payloads[index].id) {
            error = "pak entry AssetId collision";
            return false;
        }
    }

    const std::uint32_t indexOffset = PakHeaderSize;
    const std::uint32_t payloadOffset =
        indexOffset + static_cast<std::uint32_t>(payloads.size()) * PakEntrySize;
    std::vector<std::uint8_t> payloadBytes;
    std::vector<std::uint32_t> offsets;
    for (const Payload& payload : payloads) {
        while ((payloadBytes.size() & 3U) != 0U) {
            payloadBytes.push_back(0);
        }
        offsets.push_back(payloadOffset + static_cast<std::uint32_t>(payloadBytes.size()));
        payloadBytes.insert(payloadBytes.end(), payload.bytes.begin(), payload.bytes.end());
    }
    const std::uint32_t fileSize = payloadOffset + static_cast<std::uint32_t>(payloadBytes.size());
    std::vector<std::uint8_t> indexBytes;
    for (std::size_t index = 0; index < payloads.size(); ++index) {
        const Payload& payload = payloads[index];
        WriteU32(indexBytes, payload.id);
        WriteU16(indexBytes, payload.type);
        WriteU16(indexBytes, 0);
        WriteU32(indexBytes, offsets[index]);
        WriteU32(indexBytes, static_cast<std::uint32_t>(payload.bytes.size()));
        WriteU32(indexBytes, static_cast<std::uint32_t>(payload.bytes.size()));
        WriteU32(indexBytes, Checksum(payload.bytes));
    }
    std::vector<std::uint8_t> checkedBytes = indexBytes;
    checkedBytes.insert(checkedBytes.end(), payloadBytes.begin(), payloadBytes.end());

    std::vector<std::uint8_t> header;
    header.insert(header.end(), {'H', 'S', 'P', 'K'});
    WriteU16(header, PakVersion);
    WriteU16(header, PakHeaderSize);
    WriteU32(header, static_cast<std::uint32_t>(payloads.size()));
    WriteU32(header, indexOffset);
    WriteU32(header, payloadOffset);
    WriteU32(header, fileSize);
    WriteU32(header, Checksum(checkedBytes));
    WriteU32(header, 0);

    std::ofstream output(outputPath, std::ios::binary);
    output.write(
        reinterpret_cast<const char*>(header.data()),
        static_cast<std::streamsize>(header.size()));
    output.write(
        reinterpret_cast<const char*>(indexBytes.data()),
        static_cast<std::streamsize>(indexBytes.size()));
    output.write(
        reinterpret_cast<const char*>(payloadBytes.data()),
        static_cast<std::streamsize>(payloadBytes.size()));
    if (!output) {
        error = "cannot write " + outputPath.string();
        return false;
    }
    stats.entryCount = payloads.size();
    stats.byteCount = fileSize;
    return true;
}

} // namespace Homestead::AssetPacker

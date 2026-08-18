#include "Manifest.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Homestead::AssetPacker {
namespace {

struct Rect {
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

using Fields = std::vector<std::string>;

bool ReadRows(
    const std::filesystem::path& path,
    std::string_view expectedHeader,
    std::vector<Fields>& rows,
    std::string& error) {
    std::ifstream stream(path);
    if (!stream) {
        error = "cannot open " + path.string();
        return false;
    }

    std::string line;
    if (!std::getline(stream, line) || line != expectedHeader) {
        error = "invalid header in " + path.string();
        return false;
    }

    std::size_t lineNumber = 1;
    while (std::getline(stream, line)) {
        ++lineNumber;
        if (line.empty()) {
            continue;
        }
        Fields fields;
        std::size_t begin = 0;
        for (;;) {
            const std::size_t end = line.find('\t', begin);
            fields.emplace_back(line.substr(begin, end - begin));
            if (end == std::string::npos) {
                break;
            }
            begin = end + 1;
        }
        if (fields.empty()) {
            error = path.string() + ":" + std::to_string(lineNumber) + ": empty row";
            return false;
        }
        rows.push_back(std::move(fields));
    }
    return true;
}

bool ParseUint(std::string_view text, std::uint32_t& value) {
    if (text.empty()) {
        return false;
    }
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc{} && result.ptr == end;
}

bool ParseRect(const Fields& fields, std::size_t first, Rect& rect) {
    return ParseUint(fields[first], rect.x) &&
        ParseUint(fields[first + 1], rect.y) &&
        ParseUint(fields[first + 2], rect.width) &&
        ParseUint(fields[first + 3], rect.height) &&
        rect.width != 0 && rect.height != 0;
}

bool Fits(const Rect& inner, const Rect& outer) {
    return inner.x <= outer.width && inner.y <= outer.height &&
        inner.width <= outer.width - inner.x &&
        inner.height <= outer.height - inner.y;
}

std::uint32_t ReadBigEndian32(const std::uint8_t* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24U) |
        (static_cast<std::uint32_t>(bytes[1]) << 16U) |
        (static_cast<std::uint32_t>(bytes[2]) << 8U) |
        static_cast<std::uint32_t>(bytes[3]);
}

bool ReadPngSize(
    const std::filesystem::path& path,
    std::uint32_t& width,
    std::uint32_t& height) {
    std::ifstream stream(path, std::ios::binary);
    std::array<std::uint8_t, 24> header{};
    if (!stream.read(reinterpret_cast<char*>(header.data()), header.size())) {
        return false;
    }
    constexpr std::array<std::uint8_t, 8> signature{
        0x89U, 0x50U, 0x4EU, 0x47U, 0x0DU, 0x0AU, 0x1AU, 0x0AU};
    if (!std::equal(signature.begin(), signature.end(), header.begin()) ||
        header[12] != 'I' || header[13] != 'H' ||
        header[14] != 'D' || header[15] != 'R') {
        return false;
    }
    width = ReadBigEndian32(header.data() + 16);
    height = ReadBigEndian32(header.data() + 20);
    return width != 0 && height != 0;
}

} // namespace

bool ValidateManifests(
    const std::filesystem::path& assetRoot,
    ManifestStats& stats,
    std::string& error) {
    stats = {};
    std::vector<Fields> sourceRows;
    if (!ReadRows(assetRoot / "manifest.tsv", "name\tpath\tx\ty\twidth\theight", sourceRows, error)) {
        return false;
    }

    std::unordered_map<std::string, Rect> sources;
    for (const Fields& fields : sourceRows) {
        if (fields.size() != 6) {
            error = "manifest.tsv row must contain 6 fields";
            return false;
        }
        Rect rect{};
        if (!ParseRect(fields, 2, rect)) {
            error = "invalid source rectangle for " + fields[0];
            return false;
        }
        if (!sources.emplace(fields[0], rect).second) {
            error = "duplicate source name " + fields[0];
            return false;
        }
        std::uint32_t imageWidth = 0;
        std::uint32_t imageHeight = 0;
        if (!ReadPngSize(assetRoot / fields[1], imageWidth, imageHeight)) {
            error = "invalid or missing PNG " + fields[1];
            return false;
        }
        const Rect image{0, 0, imageWidth, imageHeight};
        if (!Fits(rect, image)) {
            error = "source rectangle is out of bounds for " + fields[0];
            return false;
        }
    }

    std::vector<Fields> spriteRows;
    if (!ReadRows(assetRoot / "sprites.tsv", "name\tsource\tx\ty\twidth\theight", spriteRows, error)) {
        return false;
    }
    std::unordered_set<std::string> spriteNames;
    for (const Fields& fields : spriteRows) {
        if (fields.size() != 6) {
            error = "sprites.tsv row must contain 6 fields";
            return false;
        }
        const auto source = sources.find(fields[1]);
        if (source == sources.end()) {
            error = "unknown sprite source " + fields[1];
            return false;
        }
        Rect rect{};
        if (!ParseRect(fields, 2, rect) || !Fits(rect, source->second)) {
            error = "invalid sprite rectangle for " + fields[0];
            return false;
        }
        if (!spriteNames.emplace(fields[0]).second) {
            error = "duplicate sprite name " + fields[0];
            return false;
        }
    }

    std::vector<Fields> frameRows;
    if (!ReadRows(
            assetRoot / "player-frames.tsv",
            "clip\tdirection\tframe\tplayer_x\tplayer_y\ttool_x\ttool_y\tduration_ms",
            frameRows,
            error)) {
        return false;
    }
    std::unordered_set<std::string> frameNames;
    for (const Fields& fields : frameRows) {
        if (fields.size() != 8) {
            error = "player-frames.tsv row must contain 8 fields";
            return false;
        }
        std::uint32_t frame = 0;
        std::uint32_t playerX = 0;
        std::uint32_t playerY = 0;
        std::uint32_t duration = 0;
        if (!ParseUint(fields[2], frame) || !ParseUint(fields[3], playerX) ||
            !ParseUint(fields[4], playerY) || !ParseUint(fields[7], duration) ||
            playerX > 576U - 64U || playerY > 3584U - 64U) {
            error = "invalid player frame " + fields[0] + "/" + fields[1];
            return false;
        }
        if ((fields[5] == "-") != (fields[6] == "-")) {
            error = "incomplete tool coordinates";
            return false;
        }
        if (fields[5] != "-") {
            std::uint32_t toolX = 0;
            std::uint32_t toolY = 0;
            if (!ParseUint(fields[5], toolX) || !ParseUint(fields[6], toolY) ||
                toolX > 384U - 64U || toolY > 768U - 64U) {
                error = "invalid tool frame " + fields[0] + "/" + fields[1];
                return false;
            }
        }
        const std::string key = fields[0] + "/" + fields[1] + "/" + fields[2];
        if (!frameNames.emplace(key).second) {
            error = "duplicate player frame " + key;
            return false;
        }
    }

    stats.sourceCount = sourceRows.size();
    stats.spriteCount = spriteRows.size();
    stats.playerFrameCount = frameRows.size();
    return true;
}

} // namespace Homestead::AssetPacker

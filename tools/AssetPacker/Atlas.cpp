#include "Atlas.hpp"

#include "Image.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Homestead::AssetPacker {
namespace {

struct Source {
    std::filesystem::path path;
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct PackedSprite {
    std::string name;
    Image image;
    std::uint32_t trimX = 0;
    std::uint32_t trimY = 0;
    std::uint32_t sourceWidth = 0;
    std::uint32_t sourceHeight = 0;
    std::size_t uniqueIndex = 0;
};

struct Location {
    std::uint32_t x = 0;
    std::uint32_t y = 0;
};

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

bool ToUint(const std::string& text, std::uint32_t& value) {
    const char* const begin = text.data();
    const char* const end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    return !text.empty() && result.ec == std::errc{} && result.ptr == end;
}

bool Crop(
    const Image& source,
    std::uint32_t x,
    std::uint32_t y,
    std::uint32_t width,
    std::uint32_t height,
    Image& result) {
    if (x > source.width || y > source.height ||
        width > source.width - x || height > source.height - y) {
        return false;
    }
    result.width = width;
    result.height = height;
    result.pixels.resize(static_cast<std::size_t>(width) * height * 4U);
    for (std::uint32_t row = 0; row < height; ++row) {
        const std::size_t sourceOffset =
            (static_cast<std::size_t>(y + row) * source.width + x) * 4U;
        const std::size_t resultOffset = static_cast<std::size_t>(row) * width * 4U;
        std::copy_n(
            source.pixels.data() + sourceOffset,
            static_cast<std::size_t>(width) * 4U,
            result.pixels.data() + resultOffset);
    }
    return true;
}

void Blend(Image& destination, const Image& source) {
    for (std::size_t index = 0; index < destination.pixels.size(); index += 4U) {
        const std::uint32_t sourceAlpha = source.pixels[index + 3U];
        if (sourceAlpha == 0) {
            continue;
        }
        const std::uint32_t inverseAlpha = 255U - sourceAlpha;
        const std::uint32_t destinationAlpha = destination.pixels[index + 3U];
        const std::uint32_t outputAlpha = sourceAlpha +
            (destinationAlpha * inverseAlpha + 127U) / 255U;
        for (std::size_t channel = 0; channel < 3U; ++channel) {
            const std::uint32_t sourcePremultiplied =
                static_cast<std::uint32_t>(source.pixels[index + channel]) * sourceAlpha;
            const std::uint32_t destinationPremultiplied =
                static_cast<std::uint32_t>(destination.pixels[index + channel]) *
                destinationAlpha * inverseAlpha / 255U;
            destination.pixels[index + channel] = outputAlpha == 0 ? 0U :
                static_cast<std::uint8_t>(
                    (sourcePremultiplied + destinationPremultiplied) / outputAlpha);
        }
        destination.pixels[index + 3U] = static_cast<std::uint8_t>(outputAlpha);
    }
}

bool Trim(PackedSprite& sprite, std::string& error) {
    std::uint32_t minX = sprite.image.width;
    std::uint32_t minY = sprite.image.height;
    std::uint32_t maxX = 0;
    std::uint32_t maxY = 0;
    for (std::uint32_t y = 0; y < sprite.image.height; ++y) {
        for (std::uint32_t x = 0; x < sprite.image.width; ++x) {
            const std::size_t alpha =
                (static_cast<std::size_t>(y) * sprite.image.width + x) * 4U + 3U;
            if (sprite.image.pixels[alpha] != 0) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    if (minX == sprite.image.width) {
        error = "selected sprite is empty: " + sprite.name;
        return false;
    }
    Image trimmed;
    if (!Crop(sprite.image, minX, minY, maxX - minX + 1U, maxY - minY + 1U, trimmed)) {
        error = "cannot trim sprite " + sprite.name;
        return false;
    }
    sprite.trimX = minX;
    sprite.trimY = minY;
    sprite.image = std::move(trimmed);
    return true;
}

bool SamePixels(const Image& left, const Image& right) {
    return left.width == right.width && left.height == right.height &&
        left.pixels == right.pixels;
}

bool LoadSources(
    const std::filesystem::path& root,
    std::unordered_map<std::string, Source>& sources,
    std::string& error) {
    std::ifstream stream(root / "manifest.tsv");
    std::string line;
    std::getline(stream, line);
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = Split(line);
        Source source{};
        if (fields.size() != 6 || !ToUint(fields[2], source.x) ||
            !ToUint(fields[3], source.y) || !ToUint(fields[4], source.width) ||
            !ToUint(fields[5], source.height)) {
            error = "cannot parse source while building atlas";
            return false;
        }
        source.path = root / fields[1];
        sources.emplace(fields[0], std::move(source));
    }
    return true;
}

bool GetImage(
    const std::string& name,
    const std::unordered_map<std::string, Source>& sources,
    std::unordered_map<std::string, Image>& images,
    Image*& image,
    std::string& error) {
    auto loaded = images.find(name);
    if (loaded == images.end()) {
        const auto source = sources.find(name);
        if (source == sources.end()) {
            error = "unknown atlas source " + name;
            return false;
        }
        Image decoded;
        if (!LoadPng(source->second.path, decoded, error)) {
            return false;
        }
        loaded = images.emplace(name, std::move(decoded)).first;
    }
    image = &loaded->second;
    return true;
}

} // namespace

bool BuildAtlas(
    const std::filesystem::path& assetRoot,
    const std::filesystem::path& outputDirectory,
    AtlasStats& stats,
    std::string& error) {
    stats = {};
    std::unordered_map<std::string, Source> sources;
    if (!LoadSources(assetRoot, sources, error)) {
        return false;
    }
    std::unordered_map<std::string, Image> images;
    std::vector<PackedSprite> sprites;

    std::ifstream spriteStream(assetRoot / "sprites.tsv");
    std::string line;
    std::getline(spriteStream, line);
    while (std::getline(spriteStream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = Split(line);
        std::uint32_t x = 0;
        std::uint32_t y = 0;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        if (fields.size() != 6 || !ToUint(fields[2], x) || !ToUint(fields[3], y) ||
            !ToUint(fields[4], width) || !ToUint(fields[5], height)) {
            error = "cannot parse static sprite while building atlas";
            return false;
        }
        const Source& source = sources.at(fields[1]);
        Image* image = nullptr;
        if (!GetImage(fields[1], sources, images, image, error)) {
            return false;
        }
        PackedSprite sprite{};
        sprite.name = fields[0];
        sprite.sourceWidth = width;
        sprite.sourceHeight = height;
        if (!Crop(*image, source.x + x, source.y + y, width, height, sprite.image) ||
            !Trim(sprite, error)) {
            return false;
        }
        sprites.push_back(std::move(sprite));
    }

    constexpr std::array<const char*, 5> playerLayers{
        "player.base", "player.pants", "player.shirt", "player.shoes", "player.hair"};
    std::ifstream frameStream(assetRoot / "player-frames.tsv");
    std::getline(frameStream, line);
    while (std::getline(frameStream, line)) {
        if (line.empty()) {
            continue;
        }
        const auto fields = Split(line);
        std::uint32_t playerX = 0;
        std::uint32_t playerY = 0;
        if (fields.size() != 8 || !ToUint(fields[3], playerX) || !ToUint(fields[4], playerY)) {
            error = "cannot parse player frame while building atlas";
            return false;
        }
        PackedSprite sprite{};
        sprite.name = "player." + fields[0] + "." + fields[1] + "." + fields[2];
        sprite.sourceWidth = 64;
        sprite.sourceHeight = 64;
        sprite.image.width = 64;
        sprite.image.height = 64;
        sprite.image.pixels.assign(64U * 64U * 4U, 0);
        for (const char* layerName : playerLayers) {
            Image* layer = nullptr;
            if (!GetImage(layerName, sources, images, layer, error)) {
                return false;
            }
            Image cell;
            if (!Crop(*layer, playerX, playerY, 64, 64, cell)) {
                error = "player layer frame is out of bounds";
                return false;
            }
            Blend(sprite.image, cell);
        }
        if (fields[5] != "-") {
            std::uint32_t toolX = 0;
            std::uint32_t toolY = 0;
            if (!ToUint(fields[5], toolX) || !ToUint(fields[6], toolY)) {
                error = "invalid tool coordinates";
                return false;
            }
            Image* tools = nullptr;
            if (!GetImage("tools.iron", sources, images, tools, error)) {
                return false;
            }
            Image cell;
            if (!Crop(*tools, toolX, toolY, 64, 64, cell)) {
                error = "tool frame is out of bounds";
                return false;
            }
            Blend(sprite.image, cell);
        }
        if (!Trim(sprite, error)) {
            return false;
        }
        sprites.push_back(std::move(sprite));
    }

    std::vector<std::size_t> uniqueSprites;
    for (std::size_t index = 0; index < sprites.size(); ++index) {
        auto duplicate = std::find_if(
            uniqueSprites.begin(), uniqueSprites.end(), [&](std::size_t candidate) {
                return SamePixels(sprites[index].image, sprites[candidate].image);
            });
        if (duplicate == uniqueSprites.end()) {
            sprites[index].uniqueIndex = index;
            uniqueSprites.push_back(index);
        } else {
            sprites[index].uniqueIndex = *duplicate;
        }
    }

    std::stable_sort(
        uniqueSprites.begin(), uniqueSprites.end(), [&](std::size_t left, std::size_t right) {
            const Image& leftImage = sprites[left].image;
            const Image& rightImage = sprites[right].image;
            if (leftImage.height != rightImage.height) {
                return leftImage.height > rightImage.height;
            }
            if (leftImage.width != rightImage.width) {
                return leftImage.width > rightImage.width;
            }
            return sprites[left].name < sprites[right].name;
        });

    constexpr std::uint32_t atlasWidth = 512;
    std::unordered_map<std::size_t, Location> locations;
    std::uint32_t cursorX = 1;
    std::uint32_t cursorY = 1;
    std::uint32_t shelfHeight = 0;
    for (const std::size_t index : uniqueSprites) {
        const Image& image = sprites[index].image;
        if (image.width + 2U > atlasWidth) {
            error = "sprite is wider than atlas: " + sprites[index].name;
            return false;
        }
        if (cursorX + image.width + 1U > atlasWidth) {
            cursorX = 1;
            cursorY += shelfHeight + 1U;
            shelfHeight = 0;
        }
        locations[index] = {cursorX, cursorY};
        cursorX += image.width + 1U;
        shelfHeight = std::max(shelfHeight, image.height);
    }
    const std::uint32_t usedHeight = cursorY + shelfHeight + 1U;
    const std::uint32_t atlasHeight = usedHeight;
    Image atlas{};
    atlas.width = atlasWidth;
    atlas.height = atlasHeight;
    atlas.pixels.assign(static_cast<std::size_t>(atlasWidth) * atlasHeight * 4U, 0);
    for (const std::size_t index : uniqueSprites) {
        const Image& image = sprites[index].image;
        const Location location = locations[index];
        for (std::uint32_t row = 0; row < image.height; ++row) {
            const std::size_t sourceOffset = static_cast<std::size_t>(row) * image.width * 4U;
            const std::size_t destinationOffset =
                (static_cast<std::size_t>(location.y + row) * atlasWidth + location.x) * 4U;
            std::copy_n(
                image.pixels.data() + sourceOffset,
                static_cast<std::size_t>(image.width) * 4U,
                atlas.pixels.data() + destinationOffset);
        }
    }

    std::filesystem::create_directories(outputDirectory);
    std::ofstream pixels(outputDirectory / "atlas.rgba", std::ios::binary);
    const std::array<std::uint32_t, 2> dimensions{atlas.width, atlas.height};
    pixels.write(
        reinterpret_cast<const char*>(dimensions.data()),
        static_cast<std::streamsize>(sizeof(dimensions)));
    pixels.write(
        reinterpret_cast<const char*>(atlas.pixels.data()),
        static_cast<std::streamsize>(atlas.pixels.size()));
    if (!pixels) {
        error = "cannot write atlas.rgba";
        return false;
    }

    std::vector<std::uint32_t> palette;
    std::unordered_map<std::uint32_t, std::uint8_t> paletteIndices;
    std::vector<std::uint8_t> indices;
    indices.reserve(static_cast<std::size_t>(atlas.width) * atlas.height);
    for (std::size_t offset = 0; offset < atlas.pixels.size(); offset += 4U) {
        std::uint32_t color = 0;
        if (atlas.pixels[offset + 3U] != 0) {
            color = static_cast<std::uint32_t>(atlas.pixels[offset]) |
                (static_cast<std::uint32_t>(atlas.pixels[offset + 1U]) << 8U) |
                (static_cast<std::uint32_t>(atlas.pixels[offset + 2U]) << 16U) |
                (static_cast<std::uint32_t>(atlas.pixels[offset + 3U]) << 24U);
        }
        auto found = paletteIndices.find(color);
        if (found == paletteIndices.end()) {
            if (palette.size() == 256U) {
                error = "atlas exceeds the 256-color palette limit";
                return false;
            }
            const auto index = static_cast<std::uint8_t>(palette.size());
            palette.push_back(color);
            found = paletteIndices.emplace(color, index).first;
        }
        indices.push_back(found->second);
    }
    std::ofstream paletted(outputDirectory / "atlas.pal", std::ios::binary);
    constexpr std::array<char, 4> paletteMagic{'H', 'S', 'P', 'A'};
    const std::array<std::uint16_t, 4> paletteHeader{
        1U,
        static_cast<std::uint16_t>(atlas.width),
        static_cast<std::uint16_t>(atlas.height),
        static_cast<std::uint16_t>(palette.size())};
    paletted.write(paletteMagic.data(), static_cast<std::streamsize>(paletteMagic.size()));
    paletted.write(
        reinterpret_cast<const char*>(paletteHeader.data()),
        static_cast<std::streamsize>(sizeof(paletteHeader)));
    paletted.write(
        reinterpret_cast<const char*>(palette.data()),
        static_cast<std::streamsize>(palette.size() * sizeof(std::uint32_t)));
    paletted.write(
        reinterpret_cast<const char*>(indices.data()),
        static_cast<std::streamsize>(indices.size()));
    if (!paletted) {
        error = "cannot write atlas.pal";
        return false;
    }
    std::ofstream metadata(outputDirectory / "atlas.tsv", std::ios::binary);
    metadata << "name\tx\ty\twidth\theight\ttrim_x\ttrim_y\tsource_width\tsource_height\n";
    for (const PackedSprite& sprite : sprites) {
        const Location location = locations.at(sprite.uniqueIndex);
        metadata << sprite.name << '\t' << location.x << '\t' << location.y << '\t'
                 << sprite.image.width << '\t' << sprite.image.height << '\t'
                 << sprite.trimX << '\t' << sprite.trimY << '\t'
                 << sprite.sourceWidth << '\t' << sprite.sourceHeight << '\n';
    }
    if (!metadata) {
        error = "cannot write atlas.tsv";
        return false;
    }

    stats.logicalSpriteCount = sprites.size();
    stats.uniqueSpriteCount = uniqueSprites.size();
    stats.width = atlas.width;
    stats.height = atlas.height;
    stats.paletteColorCount = palette.size();
    stats.pixelBytes = sizeof(dimensions) + atlas.pixels.size();
    stats.paletteBytes = paletteMagic.size() + sizeof(paletteHeader) +
        palette.size() * sizeof(std::uint32_t) + indices.size();
    return true;
}

} // namespace Homestead::AssetPacker

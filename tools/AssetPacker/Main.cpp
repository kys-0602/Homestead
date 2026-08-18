#include "Manifest.hpp"
#include "Atlas.hpp"
#include "Image.hpp"
#include "Pak.hpp"

#include <iostream>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 3) {
        std::cerr << "Usage: AssetPacker <assets-src directory> <output directory>\n";
        return 2;
    }

    Homestead::AssetPacker::ManifestStats stats{};
    std::string error;
    if (!Homestead::AssetPacker::ValidateManifests(arguments[1], stats, error)) {
        std::cerr << "Asset manifest error: " << error << '\n';
        return 1;
    }

    std::cout << "Validated " << stats.sourceCount << " sources, "
              << stats.spriteCount << " sprites, and "
              << stats.playerFrameCount << " player frames.\n";

    if (!Homestead::AssetPacker::InitializeImageDecoder(error)) {
        std::cerr << "Asset image error: " << error << '\n';
        return 1;
    }
    Homestead::AssetPacker::AtlasStats atlasStats{};
    const bool built = Homestead::AssetPacker::BuildAtlas(
        arguments[1], arguments[2], atlasStats, error);
    Homestead::AssetPacker::ShutdownImageDecoder();
    if (!built) {
        std::cerr << "Atlas error: " << error << '\n';
        return 1;
    }
    std::cout << "Packed " << atlasStats.logicalSpriteCount << " logical sprites into "
              << atlasStats.uniqueSpriteCount << " unique regions in a "
              << atlasStats.width << 'x' << atlasStats.height << " atlas ("
              << atlasStats.pixelBytes << " bytes).\n";
    std::cout << "Paletted atlas uses " << atlasStats.paletteColorCount
              << " colors and " << atlasStats.paletteBytes << " bytes.\n";
    Homestead::AssetPacker::PakStats pakStats{};
    const std::filesystem::path outputDirectory = arguments[2];
    if (!Homestead::AssetPacker::BuildPak(
            outputDirectory, outputDirectory / "data.pak", pakStats, error)) {
        std::cerr << "Pak error: " << error << '\n';
        return 1;
    }
    std::cout << "Wrote data.pak with " << pakStats.entryCount << " entries, "
              << pakStats.spriteCount << " sprite records, and "
              << pakStats.byteCount << " bytes.\n";
    return 0;
}

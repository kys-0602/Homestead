#include "Map.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

bool Write(const std::filesystem::path& path, const char* text) {
    std::ofstream stream(path, std::ios::binary);
    stream << text;
    return static_cast<bool>(stream);
}

} // namespace

int main() {
    const auto uniqueSuffix = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / ("homestead-map-tests-" + std::to_string(uniqueSuffix));
    std::error_code errorCode;
    std::filesystem::create_directories(directory, errorCode);
    if (errorCode) {
        return 1;
    }
    const std::filesystem::path mapPath = directory / "map.txt";
    if (!Write(mapPath, "HSM1\t4\t2\n.Tf.\n=|-s\n")) {
        return 2;
    }

    Homestead::AssetPacker::MapStats firstStats{};
    Homestead::AssetPacker::MapStats secondStats{};
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> second;
    std::string error;
    if (!Homestead::AssetPacker::BuildMapPayload(mapPath, first, firstStats, error) ||
        !Homestead::AssetPacker::BuildMapPayload(mapPath, second, secondStats, error) ||
        first != second || firstStats.width != 4 || firstStats.height != 2 ||
        firstStats.byteCount != 72) {
        return 3;
    }

    if (!Write(mapPath, "HSM1\t4\t2\n....\n...\n") ||
        Homestead::AssetPacker::BuildMapPayload(mapPath, first, firstStats, error)) {
        return 4;
    }
    if (!Write(mapPath, "HSM1\t4\t1\n..?.\n") ||
        Homestead::AssetPacker::BuildMapPayload(mapPath, first, firstStats, error)) {
        return 5;
    }
    if (!Write(mapPath, "HSM1\t129\t1\n") ||
        Homestead::AssetPacker::BuildMapPayload(mapPath, first, firstStats, error)) {
        return 6;
    }

    std::filesystem::remove(mapPath, errorCode);
    std::filesystem::remove(directory, errorCode);
    return 0;
}

#include "Manifest.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

bool WriteText(const std::filesystem::path& path, const char* text) {
    std::ofstream stream(path, std::ios::binary);
    stream << text;
    return static_cast<bool>(stream);
}

bool WritePngHeader(const std::filesystem::path& path) {
    constexpr std::array<std::uint8_t, 24> bytes{
        0x89U, 0x50U, 0x4EU, 0x47U, 0x0DU, 0x0AU, 0x1AU, 0x0AU,
        0x00U, 0x00U, 0x00U, 0x0DU, 0x49U, 0x48U, 0x44U, 0x52U,
        0x00U, 0x00U, 0x00U, 0x10U, 0x00U, 0x00U, 0x00U, 0x10U};
    std::ofstream stream(path, std::ios::binary);
    stream.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
    return static_cast<bool>(stream);
}

bool WriteValidFiles(const std::filesystem::path& root) {
    return WritePngHeader(root / "image.png") &&
        WriteText(
            root / "manifest.tsv",
            "name\tpath\tx\ty\twidth\theight\n"
            "image\timage.png\t0\t0\t16\t16\n") &&
        WriteText(
            root / "sprites.tsv",
            "name\tsource\tx\ty\twidth\theight\n"
            "sprite\timage\t0\t0\t16\t16\n") &&
        WriteText(
            root / "player-frames.tsv",
            "clip\tdirection\tframe\tplayer_x\tplayer_y\ttool_x\ttool_y\tduration_ms\n"
            "idle\tdown\t0\t0\t0\t-\t-\t0\n");
}

bool IsValid(const std::filesystem::path& root) {
    Homestead::AssetPacker::ManifestStats stats{};
    std::string error;
    return Homestead::AssetPacker::ValidateManifests(root, stats, error);
}

} // namespace

int main() {
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "HomesteadAssetManifestTests";
    std::error_code removeError;
    std::filesystem::remove_all(root, removeError);
    if (!std::filesystem::create_directories(root) || !WriteValidFiles(root) || !IsValid(root)) {
        return 1;
    }

    if (!WriteText(
            root / "manifest.tsv",
            "name\tpath\tx\ty\twidth\theight\n"
            "image\timage.png\t0\t0\tbad\t16\n") ||
        IsValid(root)) {
        return 2;
    }

    if (!WriteValidFiles(root) || !WriteText(
            root / "manifest.tsv",
            "name\tpath\tx\ty\twidth\theight\n"
            "image\timage.png\t0\t0\t16\t16\n"
            "image\timage.png\t0\t0\t16\t16\n") ||
        IsValid(root)) {
        return 3;
    }

    if (!WriteValidFiles(root) || !WriteText(
            root / "sprites.tsv",
            "name\tsource\tx\ty\twidth\theight\n"
            "sprite\tmissing\t0\t0\t16\t16\n") ||
        IsValid(root)) {
        return 4;
    }

    if (!WriteValidFiles(root) || !WriteText(
            root / "player-frames.tsv",
            "clip\tdirection\tframe\tplayer_x\tplayer_y\ttool_x\ttool_y\tduration_ms\n"
            "idle\tdown\t0\t576\t0\t-\t-\t0\n") ||
        IsValid(root)) {
        return 5;
    }

    std::filesystem::remove_all(root, removeError);
    return 0;
}

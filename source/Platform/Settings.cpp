#include "Homestead/Platform/Settings.hpp"

#include <Windows.h>

#include <array>
#include <cstddef>

namespace Homestead {
namespace {

constexpr std::size_t SettingsSize = 12;

std::uint32_t Checksum(const std::uint8_t* bytes, std::size_t size) noexcept {
    std::uint32_t hash = 2166136261U;
    for (std::size_t index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 16777619U;
    }
    return hash;
}

bool SettingsPath(wchar_t (&path)[MAX_PATH], const wchar_t* suffix) noexcept {
    wchar_t local[MAX_PATH]{};
    const DWORD localLength = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
    if (localLength == 0 || localLength >= MAX_PATH) return false;
    const int length = swprintf_s(path, L"%s\\Homestead\\settings.cfg%s", local, suffix);
    return length > 0 && length < MAX_PATH;
}

bool EnsureDirectory() noexcept {
    wchar_t path[MAX_PATH]{};
    wchar_t local[MAX_PATH]{};
    const DWORD localLength = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
    if (localLength == 0 || localLength >= MAX_PATH) return false;
    const int length = swprintf_s(path, L"%s\\Homestead", local);
    return length > 0 && length < MAX_PATH &&
        (CreateDirectoryW(path, nullptr) != FALSE || GetLastError() == ERROR_ALREADY_EXISTS);
}

void Put32(std::uint8_t* output, std::uint32_t value) noexcept {
    output[0] = static_cast<std::uint8_t>(value);
    output[1] = static_cast<std::uint8_t>(value >> 8U);
    output[2] = static_cast<std::uint8_t>(value >> 16U);
    output[3] = static_cast<std::uint8_t>(value >> 24U);
}

std::uint32_t Get32(const std::uint8_t* input) noexcept {
    return input[0] | (static_cast<std::uint32_t>(input[1]) << 8U) |
        (static_cast<std::uint32_t>(input[2]) << 16U) |
        (static_cast<std::uint32_t>(input[3]) << 24U);
}

} // namespace

bool IsValidSettings(const Settings& settings) noexcept {
    return settings.windowScale >= 2 && settings.windowScale <= 4 &&
        settings.masterVolume <= 10 && settings.musicVolume <= 10 &&
        settings.effectVolume <= 10;
}

bool SettingsSystem::Load(Settings& settings) const noexcept {
    wchar_t path[MAX_PATH]{};
    if (!SettingsPath(path, L"")) return false;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    std::array<std::uint8_t, SettingsSize> bytes{};
    DWORD read = 0;
    const bool ok = ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) != FALSE;
    LARGE_INTEGER size{};
    const bool sized = GetFileSizeEx(file, &size) != FALSE;
    CloseHandle(file);
    if (!ok || !sized || read != bytes.size() || size.QuadPart != SettingsSize ||
        bytes[0] != 'H' || bytes[1] != 'S' || bytes[2] != 'C' || bytes[3] != 'F' ||
        bytes[4] != 1 || bytes[6] > 21 ||
        Get32(bytes.data() + 8) != Checksum(bytes.data(), 8)) return false;
    Settings decoded{};
    decoded.windowScale = bytes[5];
    decoded.fullscreen = (bytes[6] & 1U) != 0;
    decoded.masterVolume = static_cast<std::uint8_t>(bytes[7] & 0x0FU);
    decoded.musicVolume = static_cast<std::uint8_t>((bytes[7] >> 4U) & 0x0FU);
    decoded.effectVolume = static_cast<std::uint8_t>(bytes[6] >> 1U);
    if (!IsValidSettings(decoded)) return false;
    settings = decoded;
    return true;
}

bool SettingsSystem::Save(const Settings& settings) const noexcept {
    if (!IsValidSettings(settings) || !EnsureDirectory()) return false;
    std::array<std::uint8_t, SettingsSize> bytes{};
    bytes[0] = 'H'; bytes[1] = 'S'; bytes[2] = 'C'; bytes[3] = 'F'; bytes[4] = 1;
    bytes[5] = settings.windowScale;
    bytes[6] = static_cast<std::uint8_t>((settings.effectVolume << 1U) | (settings.fullscreen ? 1U : 0U));
    bytes[7] = static_cast<std::uint8_t>((settings.musicVolume << 4U) | settings.masterVolume);
    Put32(bytes.data() + 8, Checksum(bytes.data(), 8));
    wchar_t path[MAX_PATH]{}; wchar_t temporary[MAX_PATH]{};
    if (!SettingsPath(path, L"") || !SettingsPath(temporary, L".tmp")) return false;
    HANDLE file = CreateFileW(temporary, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    const bool wrote = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) != FALSE &&
        written == bytes.size() && FlushFileBuffers(file) != FALSE;
    CloseHandle(file);
    if (!wrote) { DeleteFileW(temporary); return false; }
    if (MoveFileExW(temporary, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == FALSE) {
        DeleteFileW(temporary); return false;
    }
    return true;
}

} // namespace Homestead

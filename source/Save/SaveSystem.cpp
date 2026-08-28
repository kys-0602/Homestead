#include "Homestead/Save/SaveSystem.hpp"

#include <Windows.h>

#include <algorithm>
#include <cwchar>
#include <iterator>
#include <vector>

namespace Homestead {
namespace {

bool Paths(wchar_t (&primary)[MAX_PATH], wchar_t (&temporary)[MAX_PATH],
           wchar_t (&backup)[MAX_PATH]) noexcept {
    wchar_t local[MAX_PATH]{};
    const DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return false;
    constexpr wchar_t directory[] = L"\\Homestead";
    if (length + std::size(directory) >= MAX_PATH) return false;
    std::copy(std::begin(directory), std::end(directory), local + length);
    if (!CreateDirectoryW(local, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) return false;
    const auto make = [&](wchar_t (&out)[MAX_PATH], const wchar_t* name) noexcept {
        const std::size_t base = wcslen(local), file = wcslen(name);
        if (base + file + 1 >= MAX_PATH) return false;
        std::copy(local, local + base, out); std::copy(name, name + file + 1, out + base); return true;
    };
    return make(primary, L"\\representative.sav") && make(temporary, L"\\representative.tmp") &&
        make(backup, L"\\representative.bak");
}

bool Read(const wchar_t* path, std::vector<std::uint8_t>& bytes, bool& found) noexcept {
    found = false;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return GetLastError() == ERROR_FILE_NOT_FOUND;
    found = true;
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) || size.QuadPart <= 0 ||
        size.QuadPart > static_cast<LONGLONG>(MaximumSaveBytes)) { CloseHandle(file); return false; }
    bytes.resize(static_cast<std::size_t>(size.QuadPart));
    DWORD read = 0;
    const bool ok = ReadFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr) &&
        read == static_cast<DWORD>(bytes.size());
    CloseHandle(file); return ok;
}

bool Write(const wchar_t* path, const std::vector<std::uint8_t>& bytes) noexcept {
    HANDLE file = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    const bool ok = WriteFile(file, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) &&
        written == static_cast<DWORD>(bytes.size()) && FlushFileBuffers(file);
    CloseHandle(file); return ok;
}

} // namespace

SaveLoadResult SaveSystem::Load(SaveSnapshot& snapshot) noexcept {
    wchar_t primary[MAX_PATH]{}, temporary[MAX_PATH]{}, backup[MAX_PATH]{};
    if (!Paths(primary, temporary, backup)) return SaveLoadResult::Invalid;
    std::vector<std::uint8_t> bytes; bool primaryFound = false;
    if (Read(primary, bytes, primaryFound) && primaryFound && DecodeSave(bytes.data(), bytes.size(), snapshot))
        return SaveLoadResult::LoadedPrimary;
    bytes.clear(); bool backupFound = false;
    if (Read(backup, bytes, backupFound) && backupFound && DecodeSave(bytes.data(), bytes.size(), snapshot))
        return SaveLoadResult::LoadedBackup;
    return primaryFound || backupFound ? SaveLoadResult::Invalid : SaveLoadResult::NotFound;
}

bool SaveSystem::Save(const SaveSnapshot& snapshot) noexcept {
    std::vector<std::uint8_t> bytes;
    if (!EncodeSave(snapshot, bytes)) return false;
    wchar_t primary[MAX_PATH]{}, temporary[MAX_PATH]{}, backup[MAX_PATH]{};
    if (!Paths(primary, temporary, backup) || !Write(temporary, bytes)) return false;
    std::vector<std::uint8_t> existingBytes; bool primaryFound = false;
    SaveSnapshot existingSnapshot;
    const bool primaryValid = Read(primary, existingBytes, primaryFound) && primaryFound &&
        DecodeSave(existingBytes.data(), existingBytes.size(), existingSnapshot);
    if (primaryValid && !CopyFileW(primary, backup, FALSE)) {
        DeleteFileW(temporary); return false;
    }
    if (!MoveFileExW(temporary, primary, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileW(temporary); return false;
    }
    return true;
}

bool SaveSystem::Reset() noexcept {
    wchar_t primary[MAX_PATH]{}, temporary[MAX_PATH]{}, backup[MAX_PATH]{};
    if (!Paths(primary, temporary, backup)) return false;
    const auto remove = [](const wchar_t* path) noexcept {
        return DeleteFileW(path) != FALSE || GetLastError() == ERROR_FILE_NOT_FOUND;
    };
    return remove(temporary) && remove(primary) && remove(backup);
}

} // namespace Homestead

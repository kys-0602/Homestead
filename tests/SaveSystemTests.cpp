#include "Homestead/Save/SaveSystem.hpp"

#include <Windows.h>

#include <cstdio>

namespace {

bool Exists(const wchar_t* path) {
    return GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
}

} // namespace

int main() {
    wchar_t temporary[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, temporary);
    if (length == 0 || length >= MAX_PATH) return 1;
    wchar_t root[MAX_PATH]{};
    if (swprintf_s(root, L"%sHomesteadSaveSystemTests-%lu", temporary,
                   GetCurrentProcessId()) <= 0 || CreateDirectoryW(root, nullptr) == FALSE ||
        SetEnvironmentVariableW(L"LOCALAPPDATA", root) == FALSE) return 2;

    Homestead::SaveSystem system;
    Homestead::SaveSnapshot snapshot;
    snapshot.playerX256 = 1234;
    snapshot.playerY256 = 5678;
    snapshot.inventory[0] = {Homestead::ItemId::Hoe, 1};
    if (!system.Save(snapshot)) return 3;
    snapshot.day = 2;
    if (!system.Save(snapshot)) return 4;
    Homestead::SaveSnapshot loaded;
    if (system.Load(loaded) != Homestead::SaveLoadResult::LoadedPrimary || loaded.day != 2) return 5;

    wchar_t directory[MAX_PATH]{};
    wchar_t primary[MAX_PATH]{};
    wchar_t backup[MAX_PATH]{};
    swprintf_s(directory, L"%s\\Homestead", root);
    swprintf_s(primary, L"%s\\representative.sav", directory);
    swprintf_s(backup, L"%s\\representative.bak", directory);
    if (!Exists(primary) || !Exists(backup) || !system.Reset() || Exists(primary) || Exists(backup) ||
        system.Load(loaded) != Homestead::SaveLoadResult::NotFound) return 6;

    RemoveDirectoryW(directory);
    RemoveDirectoryW(root);
    return 0;
}

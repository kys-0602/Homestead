#include "Homestead/Platform/Settings.hpp"

#include <Windows.h>

#include <cstdio>

int main() {
    Homestead::Settings settings{};
    if (!Homestead::IsValidSettings(settings)) return 1;
    settings.windowScale = 1;
    if (Homestead::IsValidSettings(settings)) return 2;
    settings = {}; settings.windowScale = 5;
    if (Homestead::IsValidSettings(settings)) return 3;
    settings = {}; settings.masterVolume = 11;
    if (Homestead::IsValidSettings(settings)) return 4;
    settings = {}; settings.musicVolume = 11;
    if (Homestead::IsValidSettings(settings)) return 5;
    settings = {}; settings.effectVolume = 11;
    if (Homestead::IsValidSettings(settings)) return 6;
    wchar_t temporary[MAX_PATH]{};
    const DWORD length = GetTempPathW(MAX_PATH, temporary);
    if (length == 0 || length >= MAX_PATH) return 7;
    wchar_t root[MAX_PATH]{};
    if (swprintf_s(root, L"%sHomesteadSettingsTests-%lu", temporary,
                   GetCurrentProcessId()) <= 0 || CreateDirectoryW(root, nullptr) == FALSE ||
        SetEnvironmentVariableW(L"LOCALAPPDATA", root) == FALSE) return 8;

    settings = {};
    settings.windowScale = 2; settings.fullscreen = true;
    settings.masterVolume = 7; settings.musicVolume = 6; settings.effectVolume = 5;
    Homestead::SettingsSystem system;
    Homestead::Settings loaded{};
    if (!system.Save(settings) || !system.Load(loaded) ||
        loaded.windowScale != 2 || !loaded.fullscreen || loaded.masterVolume != 7 ||
        loaded.musicVolume != 6 || loaded.effectVolume != 5) return 9;

    wchar_t filePath[MAX_PATH]{};
    swprintf_s(filePath, L"%s\\Homestead\\settings.cfg", root);
    HANDLE file = CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return 10;
    const unsigned char damaged = 'X'; DWORD written = 0;
    const bool damagedFile = WriteFile(file, &damaged, 1, &written, nullptr) != FALSE;
    CloseHandle(file);
    if (!damagedFile || system.Load(loaded)) return 11;

    wchar_t directory[MAX_PATH]{};
    swprintf_s(directory, L"%s\\Homestead", root);
    DeleteFileW(filePath); RemoveDirectoryW(directory); RemoveDirectoryW(root);
    return 0;
}

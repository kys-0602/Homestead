#include "Homestead/App/StartupLoader.hpp"
#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Audio/Audio.hpp"
#include "Homestead/World/TileMap.hpp"

#include <Windows.h>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;
    wchar_t pakPath[MAX_PATH]{};
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, arguments[1], -1,
                            pakPath, MAX_PATH) == 0) return 2;

    Homestead::AssetStore assets;
    Homestead::Audio audio;
    Homestead::SaveSystem saves;
    Homestead::TileMap farm;
    Homestead::TileMap house;
    Homestead::StartupLoader loader;
    if (!loader.Start(pakPath, assets, farm, house, audio, saves)) return 3;
    loader.Wait();
    if (loader.Stage() != Homestead::StartupStage::Ready || !loader.AudioPrepared() ||
        assets.SpriteCount() == 0 || farm.Width() == 0 || house.Width() == 0) return 4;

    Homestead::AssetStore missingAssets;
    Homestead::Audio missingAudio;
    Homestead::TileMap missingFarm;
    Homestead::TileMap missingHouse;
    Homestead::StartupLoader missing;
    if (!missing.Start(L"Homestead-file-that-does-not-exist.pak", missingAssets,
                       missingFarm, missingHouse, missingAudio, saves)) return 5;
    missing.Wait();
    if (missing.Stage() != Homestead::StartupStage::Failed) return 6;
    return 0;
}

#include "Homestead/Assets/AssetStore.hpp"
#include <Windows.h>
int wmain(int count,wchar_t** args){
    if(count!=2)return 1; Homestead::AssetStore assets;
    if(!assets.LoadFile(args[1]))return 2;
    constexpr Homestead::AssetId ids[]{Homestead::MakeAssetId("audio.music.farm"),
        Homestead::MakeAssetId("audio.hoe"),Homestead::MakeAssetId("audio.watering"),
        Homestead::MakeAssetId("audio.plant"),Homestead::MakeAssetId("audio.harvest"),
        Homestead::MakeAssetId("audio.ui.move"),Homestead::MakeAssetId("audio.ui.confirm")};
    for(auto id:ids)if(!assets.FindAudio(id))return 3; return 0;
}

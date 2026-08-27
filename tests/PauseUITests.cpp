#include "Homestead/UI/PauseUI.hpp"
#include "Homestead/UI/MarketUI.hpp"
#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Platform/Settings.hpp"

#include <Windows.h>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;
    wchar_t pakPath[MAX_PATH]{};
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, arguments[1], -1,
                            pakPath, MAX_PATH) == 0) return 2;
    if (Homestead::PauseItemAt(68, 48) != 0 ||
        Homestead::PauseItemAt(251, 145) != 6 ||
        Homestead::PauseItemAt(251, 159) != 7 ||
        Homestead::PauseItemAt(67, 48) != -1 ||
        Homestead::PauseItemAt(252, 48) != -1 ||
        Homestead::PauseItemAt(68, 47) != -1 ||
        Homestead::PauseItemAt(68, 160) != -1) return 3;
    if(Homestead::MarketItemAt(70,62)!=0||Homestead::MarketItemAt(249,151)!=5||
       Homestead::MarketItemAt(69,62)!=-1||Homestead::MarketItemAt(70,152)!=-1)return 5;
    if(Homestead::UpdateMarketFocus(2,true,false,true,80,62)!=1||
       Homestead::UpdateMarketFocus(2,false,true,true,80,62)!=3||
       Homestead::UpdateMarketFocus(2,false,false,true,80,62)!=0)return 6;
    Homestead::AssetStore assets;
    Homestead::RenderQueue queue;
    Homestead::Settings settings;
    if (!assets.LoadFile(pakPath) || !Homestead::AddPauseUI(settings, 7, assets, queue) ||
        queue.Empty()) return 7;
    return 0;
}

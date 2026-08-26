#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/DailyRequest.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/DailyRequestUI.hpp"

#include <Windows.h>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;
    if (!Homestead::DailyRequestButtonAt(105, 116) ||
        !Homestead::DailyRequestButtonAt(214, 133) ||
        Homestead::DailyRequestButtonAt(104, 116) ||
        Homestead::DailyRequestButtonAt(215, 133) ||
        Homestead::DailyRequestButtonAt(105, 134)) return 2;

    wchar_t pakPath[MAX_PATH]{};
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, arguments[1], -1,
                            pakPath, MAX_PATH) == 0) return 3;
    Homestead::AssetStore assets;
    Homestead::Inventory inventory;
    Homestead::DailyRequestState state;
    Homestead::RenderQueue queue;
    const Homestead::DailyRequest request = Homestead::BuildDailyRequest(1);
    if (!assets.LoadFile(pakPath) ||
        !Homestead::AddDailyRequestUI(request, state, inventory, 20, assets, queue) ||
        queue.Empty()) return 4;
    queue.Clear();
    if (inventory.Add(request.item, request.requiredCount) != 0 ||
        !Homestead::AddDailyRequestUI(request, state, inventory, 20, assets, queue) ||
        queue.Empty()) return 5;
    state.completed = true; queue.Clear();
    if (!Homestead::AddDailyRequestUI(request, state, inventory, 20, assets, queue) ||
        queue.Empty()) return 6;
    return 0;
}

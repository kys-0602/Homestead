#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Graphics/WeatherRenderer.hpp"

#include <Windows.h>

int main(int argumentCount, char** arguments) {
    if (argumentCount != 2) return 1;
    wchar_t pakPath[MAX_PATH]{};
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, arguments[1], -1,
                            pakPath, MAX_PATH) == 0) return 2;

    Homestead::AssetStore assets;
    Homestead::RenderQueue initial;
    Homestead::RenderQueue moved;
    Homestead::RenderQueue cameraMoved;
    Homestead::Camera2D camera(320.0F, 180.0F);
    camera.SetCenter({256.0F, 192.0F});
    if (!assets.LoadFile(pakPath) ||
        !Homestead::AddCloudShadows(0, camera, assets, initial) || initial.Size() != 4 ||
        !Homestead::AddCloudShadows(12, camera, assets, moved) || moved.Size() != initial.Size()) return 3;
    for (std::size_t index = 0; index < initial.Size(); ++index) {
        if (initial[index].layer != Homestead::SpriteLayer::Effect ||
            moved[index].x != initial[index].x + 1.0F ||
            moved[index].y != initial[index].y) return 4;
    }
    camera.SetCenter({272.0F, 192.0F});
    if (!Homestead::AddCloudShadows(0, camera, assets, cameraMoved) ||
        cameraMoved.Size() != initial.Size()) return 5;
    for (std::size_t index = 0; index < initial.Size(); ++index) {
        if (cameraMoved[index].x != initial[index].x - 16.0F ||
            cameraMoved[index].y != initial[index].y) return 6;
    }
    return 0;
}

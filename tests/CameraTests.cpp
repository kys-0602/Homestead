#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/TileMapRenderer.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"

#include <cmath>

namespace {

bool Near(float left, float right) {
    return std::fabs(left - right) < 0.001F;
}

} // namespace

int main() {
    Homestead::Camera2D camera(320.0F, 180.0F);
    camera.SetCenter({256.4F, 192.4F});
    const Homestead::Float2 screen = camera.WorldToScreen({256.0F, 192.0F});
    if (!Near(screen.x, 160.0F) || !Near(screen.y, 90.0F)) {
        return 1;
    }
    const Homestead::Float2 world = camera.ScreenToWorld(screen);
    if (!Near(world.x, 256.0F) || !Near(world.y, 192.0F)) {
        return 2;
    }
    const Homestead::FloatRect visible = camera.VisibleBounds();
    if (!Near(visible.left, 96.0F) || !Near(visible.top, 102.0F) ||
        !Near(visible.right, 416.0F) || !Near(visible.bottom, 282.0F)) {
        return 3;
    }

    const Homestead::TileBounds bounds =
        Homestead::CalculateVisibleTileBounds(camera, 128, 128);
    const std::size_t tileCount = static_cast<std::size_t>(bounds.lastX - bounds.firstX + 1) *
        static_cast<std::size_t>(bounds.lastY - bounds.firstY + 1);
    if (bounds.firstX != 5 || bounds.lastX != 26 ||
        bounds.firstY != 5 || bounds.lastY != 18 || tileCount != 308 ||
        tileCount * 2 > Homestead::RenderQueue::Capacity) {
        return 4;
    }
    const Homestead::TileBounds objects =
        Homestead::CalculateVisibleObjectBounds(camera, 128, 128);
    if (objects.firstX != 2 || objects.lastX != 29 ||
        objects.firstY != 5 || objects.lastY != 25) return 9;

    camera.SetCenter({-40.0F, -20.0F});
    const Homestead::TileBounds edge =
        Homestead::CalculateVisibleTileBounds(camera, 32, 24);
    if (edge.firstX != 0 || edge.firstY != 0 || edge.lastX >= 32 || edge.lastY >= 24) {
        return 5;
    }
    camera.SetCenterClamped({10.0F, 10.0F}, 512.0F, 384.0F);
    if (!Near(camera.Center().x, 160.0F) || !Near(camera.Center().y, 90.0F)) {
        return 6;
    }
    camera.SetCenterClamped({500.0F, 380.0F}, 512.0F, 384.0F);
    if (!Near(camera.Center().x, 352.0F) || !Near(camera.Center().y, 294.0F)) {
        return 7;
    }
    camera.SetCenterClamped({100.0F, 100.0F}, 128.0F, 96.0F);
    if (!Near(camera.Center().x, 64.0F) || !Near(camera.Center().y, 48.0F)) {
        return 8;
    }
    return 0;
}

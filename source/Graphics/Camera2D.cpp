#include "Homestead/Graphics/Camera2D.hpp"

#include <cmath>

namespace Homestead {

Camera2D::Camera2D(float viewportWidth, float viewportHeight) noexcept
    : viewportWidth_(viewportWidth), viewportHeight_(viewportHeight) {
}

void Camera2D::SetZoom(float zoom) noexcept {
    zoom_ = zoom > 0.0F ? zoom : 1.0F;
}

Float2 Camera2D::WorldToScreen(Float2 world) const noexcept {
    const Float2 center = SnappedCenter();
    return {
        (world.x - center.x) * zoom_ + viewportWidth_ * 0.5F,
        (world.y - center.y) * zoom_ + viewportHeight_ * 0.5F};
}

Float2 Camera2D::ScreenToWorld(Float2 screen) const noexcept {
    const Float2 center = SnappedCenter();
    return {
        (screen.x - viewportWidth_ * 0.5F) / zoom_ + center.x,
        (screen.y - viewportHeight_ * 0.5F) / zoom_ + center.y};
}

FloatRect Camera2D::VisibleBounds() const noexcept {
    const Float2 topLeft = ScreenToWorld({0.0F, 0.0F});
    const Float2 bottomRight = ScreenToWorld({viewportWidth_, viewportHeight_});
    return {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
}

Float2 Camera2D::SnappedCenter() const noexcept {
    return {std::round(center_.x * zoom_) / zoom_, std::round(center_.y * zoom_) / zoom_};
}

} // namespace Homestead

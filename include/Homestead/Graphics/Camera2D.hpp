#pragma once

#include <cstdint>

namespace Homestead {

struct Float2 {
    float x = 0.0F;
    float y = 0.0F;
};

struct FloatRect {
    float left = 0.0F;
    float top = 0.0F;
    float right = 0.0F;
    float bottom = 0.0F;
};

class Camera2D final {
public:
    Camera2D(float viewportWidth, float viewportHeight) noexcept;

    void SetCenter(Float2 center) noexcept { center_ = center; }
    void SetCenterClamped(Float2 center, float worldWidth, float worldHeight) noexcept;
    void SetZoom(float zoom) noexcept;

    [[nodiscard]] Float2 Center() const noexcept { return center_; }
    [[nodiscard]] float Zoom() const noexcept { return zoom_; }
    [[nodiscard]] Float2 WorldToScreen(Float2 world) const noexcept;
    [[nodiscard]] Float2 ScreenToWorld(Float2 screen) const noexcept;
    [[nodiscard]] FloatRect VisibleBounds() const noexcept;

private:
    [[nodiscard]] Float2 SnappedCenter() const noexcept;

    Float2 center_{};
    float viewportWidth_ = 0.0F;
    float viewportHeight_ = 0.0F;
    float zoom_ = 1.0F;
};

} // namespace Homestead

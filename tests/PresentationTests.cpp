#include "Homestead/Graphics/Presentation.hpp"

#include <cstdint>

namespace {

bool CheckViewport(
    std::uint32_t clientWidth,
    std::uint32_t clientHeight,
    std::uint32_t expectedX,
    std::uint32_t expectedY,
    std::uint32_t expectedWidth,
    std::uint32_t expectedHeight,
    std::uint32_t expectedScale) {
    const Homestead::PresentationViewport viewport =
        Homestead::CalculatePresentationViewport(clientWidth, clientHeight);
    return viewport.x == expectedX &&
        viewport.y == expectedY &&
        viewport.width == expectedWidth &&
        viewport.height == expectedHeight &&
        viewport.integerScale == expectedScale;
}

} // namespace

int main() {
    if (!CheckViewport(1280, 720, 0, 0, 1280, 720, 4) ||
        !CheckViewport(1000, 700, 20, 80, 960, 540, 3) ||
        !CheckViewport(640, 480, 0, 60, 640, 360, 2) ||
        !CheckViewport(319, 179, 0, 0, 318, 179, 0) ||
        !CheckViewport(160, 200, 0, 55, 160, 90, 0) ||
        !CheckViewport(0, 0, 0, 0, 0, 0, 0)) {
        return 1;
    }

    const Homestead::PresentationViewport viewport =
        Homestead::CalculatePresentationViewport(1000, 700);
    std::uint32_t logicalX = 0;
    std::uint32_t logicalY = 0;

    if (!Homestead::ClientToLogical(viewport, 20, 80, logicalX, logicalY) ||
        logicalX != 0 || logicalY != 0) {
        return 2;
    }

    if (!Homestead::ClientToLogical(viewport, 979, 619, logicalX, logicalY) ||
        logicalX != 319 || logicalY != 179) {
        return 3;
    }

    if (Homestead::ClientToLogical(viewport, 19, 80, logicalX, logicalY) ||
        Homestead::ClientToLogical(viewport, 980, 80, logicalX, logicalY) ||
        Homestead::ClientToLogical(viewport, 20, 79, logicalX, logicalY) ||
        Homestead::ClientToLogical(viewport, 20, 620, logicalX, logicalY)) {
        return 4;
    }

    return 0;
}

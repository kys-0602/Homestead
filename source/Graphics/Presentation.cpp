#include "Homestead/Graphics/Presentation.hpp"

#include <algorithm>
#include <cstdint>

namespace Homestead {

PresentationViewport CalculatePresentationViewport(
    std::uint32_t clientWidth,
    std::uint32_t clientHeight) noexcept {
    PresentationViewport viewport{};
    if (clientWidth == 0 || clientHeight == 0) {
        return viewport;
    }

    const std::uint32_t integerScale = std::min(
        clientWidth / LogicalWidth,
        clientHeight / LogicalHeight);
    if (integerScale > 0) {
        viewport.width = LogicalWidth * integerScale;
        viewport.height = LogicalHeight * integerScale;
        viewport.integerScale = integerScale;
    } else if (
        static_cast<std::uint64_t>(clientWidth) * LogicalHeight <=
        static_cast<std::uint64_t>(clientHeight) * LogicalWidth) {
        viewport.width = clientWidth;
        viewport.height = static_cast<std::uint32_t>(
            static_cast<std::uint64_t>(clientWidth) * LogicalHeight / LogicalWidth);
    } else {
        viewport.height = clientHeight;
        viewport.width = static_cast<std::uint32_t>(
            static_cast<std::uint64_t>(clientHeight) * LogicalWidth / LogicalHeight);
    }

    viewport.x = (clientWidth - viewport.width) / 2;
    viewport.y = (clientHeight - viewport.height) / 2;
    return viewport;
}

bool ClientToLogical(
    const PresentationViewport& viewport,
    std::int32_t clientX,
    std::int32_t clientY,
    std::uint32_t& logicalX,
    std::uint32_t& logicalY) noexcept {
    if (viewport.width == 0 || viewport.height == 0 || clientX < 0 || clientY < 0) {
        return false;
    }

    const std::uint32_t x = static_cast<std::uint32_t>(clientX);
    const std::uint32_t y = static_cast<std::uint32_t>(clientY);
    if (x < viewport.x || y < viewport.y ||
        x >= viewport.x + viewport.width || y >= viewport.y + viewport.height) {
        return false;
    }

    logicalX = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(x - viewport.x) * LogicalWidth / viewport.width);
    logicalY = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(y - viewport.y) * LogicalHeight / viewport.height);
    return true;
}

} // namespace Homestead

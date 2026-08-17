#pragma once

#include <cstdint>

namespace Homestead {

inline constexpr std::uint32_t LogicalWidth = 320;
inline constexpr std::uint32_t LogicalHeight = 180;

struct PresentationViewport {
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t integerScale = 0;
};

[[nodiscard]] PresentationViewport CalculatePresentationViewport(
    std::uint32_t clientWidth,
    std::uint32_t clientHeight) noexcept;

[[nodiscard]] bool ClientToLogical(
    const PresentationViewport& viewport,
    std::int32_t clientX,
    std::int32_t clientY,
    std::uint32_t& logicalX,
    std::uint32_t& logicalY) noexcept;

} // namespace Homestead

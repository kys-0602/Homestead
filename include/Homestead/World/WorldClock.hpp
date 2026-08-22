#pragma once

#include <cstdint>

namespace Homestead {

class WorldClock final {
public:
    static constexpr std::uint16_t StartMinute = 6U * 60U;
    static constexpr std::uint16_t TicksPerMinute = 60;
    static constexpr std::uint16_t TransitionTicks = 60;
    static constexpr std::uint16_t DayChangeTick = 30;

    [[nodiscard]] bool FixedUpdate() noexcept;
    [[nodiscard]] bool RequestEndDay() noexcept;
    void Reset() noexcept;

    [[nodiscard]] std::uint16_t Day() const noexcept { return day_; }
    [[nodiscard]] std::uint16_t Minute() const noexcept { return minute_; }
    [[nodiscard]] bool IsTransitioning() const noexcept { return transitionTick_ != 0; }
    [[nodiscard]] std::uint8_t FadeAlpha() const noexcept;

private:
    std::uint16_t day_ = 1;
    std::uint16_t minute_ = StartMinute;
    std::uint16_t minuteTick_ = 0;
    std::uint16_t transitionTick_ = 0;
};

} // namespace Homestead

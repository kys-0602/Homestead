#include "Homestead/World/WorldClock.hpp"

#include <algorithm>

namespace Homestead {

bool WorldClock::FixedUpdate() noexcept {
    if (transitionTick_ != 0) {
        ++transitionTick_;
        const bool dayChanged = transitionTick_ == DayChangeTick;
        if (dayChanged) {
            ++day_;
            minute_ = StartMinute;
            minuteTick_ = 0;
        }
        if (transitionTick_ > TransitionTicks) transitionTick_ = 0;
        return dayChanged;
    }
    ++minuteTick_;
    if (minuteTick_ >= TicksPerMinute) {
        minuteTick_ = 0;
        minute_ = static_cast<std::uint16_t>(minute_ + 1U);
    }
    return false;
}

bool WorldClock::RequestEndDay() noexcept {
    if (transitionTick_ != 0) return false;
    transitionTick_ = 1;
    return true;
}

void WorldClock::Reset() noexcept {
    day_ = 1;
    minute_ = StartMinute;
    minuteTick_ = 0;
    transitionTick_ = 0;
}

std::uint8_t WorldClock::FadeAlpha() const noexcept {
    if (transitionTick_ == 0) return 0;
    const std::uint16_t distance = transitionTick_ <= DayChangeTick ?
        transitionTick_ : static_cast<std::uint16_t>(TransitionTicks - transitionTick_);
    return static_cast<std::uint8_t>(std::min<std::uint16_t>(255,
        static_cast<std::uint16_t>(distance * 255U / DayChangeTick)));
}

} // namespace Homestead

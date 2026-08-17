#include "Homestead/Core/FixedStep.hpp"

#include <algorithm>
#include <cmath>

namespace Homestead {

FixedStepFrame FixedStepController::Advance(double frameSeconds) noexcept {
    const double clampedFrame = std::clamp(
        frameSeconds,
        0.0,
        MaximumFrameSeconds);
    accumulator_ += clampedFrame;

    FixedStepFrame frame{};
    while (accumulator_ + 1.0e-12 >= StepSeconds &&
           frame.steps < MaximumStepsPerFrame) {
        accumulator_ -= StepSeconds;
        if (accumulator_ < 0.0) {
            accumulator_ = 0.0;
        }
        ++frame.steps;
    }

    if (frame.steps == MaximumStepsPerFrame && accumulator_ >= StepSeconds) {
        accumulator_ = std::fmod(accumulator_, StepSeconds);
    }

    frame.alpha = accumulator_ / StepSeconds;
    return frame;
}

void FixedStepController::Reset() noexcept {
    accumulator_ = 0.0;
}

} // namespace Homestead

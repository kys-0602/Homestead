#include "Homestead/Core/FixedStep.hpp"

#include <cstdint>

namespace {

std::uint32_t Simulate(std::uint32_t frameRate, std::uint32_t seconds) {
    Homestead::FixedStepController controller;
    std::uint32_t steps = 0;
    const std::uint32_t frameCount = frameRate * seconds;
    const double frameSeconds = 1.0 / static_cast<double>(frameRate);
    for (std::uint32_t frame = 0; frame < frameCount; ++frame) {
        steps += controller.Advance(frameSeconds).steps;
    }
    return steps;
}

} // namespace

int main() {
    constexpr std::uint32_t expectedSteps = 120;
    if (Simulate(30, 2) != expectedSteps ||
        Simulate(60, 2) != expectedSteps ||
        Simulate(144, 2) != expectedSteps) {
        return 1;
    }

    Homestead::FixedStepController controller;
    const Homestead::FixedStepFrame clamped = controller.Advance(10.0);
    if (clamped.steps != Homestead::FixedStepController::MaximumStepsPerFrame ||
        clamped.alpha < 0.0 || clamped.alpha >= 1.0) {
        return 2;
    }

    controller.Reset();
    const Homestead::FixedStepFrame halfStep = controller.Advance(
        Homestead::FixedStepController::StepSeconds * 0.5);
    if (halfStep.steps != 0 || halfStep.alpha < 0.49 || halfStep.alpha > 0.51) {
        return 3;
    }

    controller.Reset();
    const Homestead::FixedStepFrame reset = controller.Advance(0.0);
    if (reset.steps != 0 || reset.alpha != 0.0) {
        return 4;
    }

    return 0;
}

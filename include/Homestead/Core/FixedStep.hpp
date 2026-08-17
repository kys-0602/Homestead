#pragma once

#include <cstdint>

namespace Homestead {

struct FixedStepFrame {
    std::uint32_t steps = 0;
    double alpha = 0.0;
};

class FixedStepController final {
public:
    static constexpr double StepSeconds = 1.0 / 60.0;
    static constexpr double MaximumFrameSeconds = 0.25;
    static constexpr std::uint32_t MaximumStepsPerFrame = 5;

    [[nodiscard]] FixedStepFrame Advance(double frameSeconds) noexcept;
    void Reset() noexcept;

private:
    double accumulator_ = 0.0;
};

} // namespace Homestead

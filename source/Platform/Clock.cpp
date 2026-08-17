#include "Homestead/Platform/Clock.hpp"

#include <Windows.h>

namespace Homestead {

bool Clock::Initialize() noexcept {
    LARGE_INTEGER frequency{};
    LARGE_INTEGER counter{};
    if (QueryPerformanceFrequency(&frequency) == FALSE ||
        QueryPerformanceCounter(&counter) == FALSE) {
        return false;
    }

    frequency_ = frequency.QuadPart;
    previousCounter_ = counter.QuadPart;
    return frequency_ > 0;
}

double Clock::Tick() noexcept {
    LARGE_INTEGER counter{};
    if (frequency_ <= 0 || QueryPerformanceCounter(&counter) == FALSE) {
        return 0.0;
    }

    const std::int64_t elapsed = counter.QuadPart - previousCounter_;
    previousCounter_ = counter.QuadPart;
    return static_cast<double>(elapsed) / static_cast<double>(frequency_);
}

} // namespace Homestead

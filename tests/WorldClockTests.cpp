#include "Homestead/World/WorldClock.hpp"

int main() {
    Homestead::WorldClock clock;
    if (clock.Day() != 1 || clock.Minute() != Homestead::WorldClock::StartMinute ||
        clock.IsTransitioning() || clock.FadeAlpha() != 0) return 1;
    for (unsigned tick = 0; tick < Homestead::WorldClock::TicksPerMinute; ++tick) {
        if (clock.FixedUpdate()) return 2;
    }
    if (clock.Minute() != Homestead::WorldClock::StartMinute + 1) return 3;
    if (!clock.RequestEndDay() || clock.RequestEndDay() || !clock.IsTransitioning()) return 4;

    unsigned changes = 0;
    unsigned maximumAlpha = 0;
    while (clock.IsTransitioning()) {
        if (clock.FixedUpdate()) ++changes;
        if (clock.FadeAlpha() > maximumAlpha) maximumAlpha = clock.FadeAlpha();
    }
    if (changes != 1 || clock.Day() != 2 ||
        clock.Minute() != Homestead::WorldClock::StartMinute || maximumAlpha != 255) return 5;
    if (!clock.RequestEndDay()) return 6;
    clock.Reset();
    if (clock.Day() != 1 || clock.Minute() != Homestead::WorldClock::StartMinute ||
        clock.IsTransitioning()) return 7;
    return 0;
}

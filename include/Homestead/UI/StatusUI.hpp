#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;
class WorldClock;

[[nodiscard]] bool CompletionContinueAt(std::uint32_t x, std::uint32_t y) noexcept;

[[nodiscard]] bool AddStatusUI(
    const WorldClock& clock,
    std::uint16_t gold,
    std::uint16_t goal,
    bool showInstructions,
    bool showSaved,
    bool showRequestComplete,
    bool complete,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

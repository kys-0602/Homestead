#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;
class WorldClock;

[[nodiscard]] bool AddStatusUI(
    const WorldClock& clock,
    std::uint8_t harvested,
    std::uint8_t goal,
    bool showInstructions,
    bool complete,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

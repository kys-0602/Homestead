#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class RenderQueue;
class WorldClock;

enum class InteractionPrompt : std::uint8_t {
    None,
    Market,
    DailyRequest
};

[[nodiscard]] bool AddStatusUI(
    const WorldClock& clock,
    std::uint16_t gold,
    std::uint16_t goal,
    bool showInstructions,
    bool showSaved,
    bool showRequestComplete,
    InteractionPrompt interactionPrompt,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

#pragma once

#include <cstdint>

namespace Homestead {

class AssetStore;
class Inventory;
class RenderQueue;
struct DailyRequest;
struct DailyRequestState;

[[nodiscard]] bool DailyRequestButtonAt(std::uint32_t x, std::uint32_t y) noexcept;
[[nodiscard]] bool AddDailyRequestUI(
    const DailyRequest& request,
    const DailyRequestState& state,
    const Inventory& inventory,
    std::uint16_t gold,
    const AssetStore& assets,
    RenderQueue& queue) noexcept;

} // namespace Homestead

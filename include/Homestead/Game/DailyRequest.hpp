#pragma once

#include <cstdint>

#include "Homestead/Game/Inventory.hpp"
#include "Homestead/World/CropField.hpp"

namespace Homestead {

struct DailyRequest {
    CropId crop = CropId::None;
    ItemId item = ItemId::None;
    std::uint8_t requiredCount = 0;
    std::uint16_t reward = 0;
};

struct DailyRequestState {
    bool completed = false;
};

enum class DailyRequestStatus : std::uint8_t {
    NeedMore,
    Ready,
    Completed,
    GoldFull
};

[[nodiscard]] DailyRequest BuildDailyRequest(std::uint16_t day) noexcept;
[[nodiscard]] DailyRequestStatus GetDailyRequestStatus(
    const DailyRequest& request,
    const DailyRequestState& state,
    const Inventory& inventory,
    std::uint16_t gold) noexcept;
[[nodiscard]] bool CompleteDailyRequest(
    const DailyRequest& request,
    DailyRequestState& state,
    Inventory& inventory,
    std::uint16_t& gold) noexcept;

} // namespace Homestead

#include "Homestead/Game/DailyRequest.hpp"

#include <array>
#include <limits>

#include "Homestead/Game/Economy.hpp"

namespace Homestead {
namespace {

struct RequestDefinition {
    CropId crop;
    ItemId item;
    std::uint8_t marketIndex;
    std::uint8_t minimumCount;
    std::uint8_t countRange;
};

constexpr std::array<RequestDefinition, MarketCropCount> Definitions{{
    {CropId::Wheat, ItemId::Wheat, 0, 3, 3},
    {CropId::Carrot, ItemId::Carrot, 1, 2, 3},
    {CropId::Tomato, ItemId::Tomato, 2, 1, 3},
    {CropId::Potato, ItemId::Potato, 3, 3, 3},
    {CropId::Corn, ItemId::Corn, 4, 2, 3},
    {CropId::Cabbage, ItemId::Cabbage, 5, 1, 3}}};
constexpr std::uint32_t DefinitionCount = static_cast<std::uint32_t>(Definitions.size());

} // namespace

DailyRequest BuildDailyRequest(std::uint16_t day) noexcept {
    const std::uint32_t normalizedDay = day == 0 ? 1U : day;
    const std::uint32_t dayIndex = normalizedDay - 1U;
    const RequestDefinition& definition = Definitions[dayIndex % DefinitionCount];
    std::uint32_t random = normalizedDay * 0x9E3779B9U;
    random ^= random >> 16U;
    const std::uint8_t count = static_cast<std::uint8_t>(
        definition.minimumCount + random % definition.countRange);
    const std::uint16_t saleValue = static_cast<std::uint16_t>(
        count * MarketTable[definition.marketIndex].sellPrice);
    const std::uint16_t reward = static_cast<std::uint16_t>((saleValue * 4U + 2U) / 3U);
    return {definition.crop, definition.item, count, reward};
}

DailyRequestStatus GetDailyRequestStatus(
    const DailyRequest& request,
    const DailyRequestState& state,
    const Inventory& inventory,
    std::uint16_t gold) noexcept {
    if (state.completed) return DailyRequestStatus::Completed;
    if (inventory.Count(request.item) < request.requiredCount) return DailyRequestStatus::NeedMore;
    if (request.reward > std::numeric_limits<std::uint16_t>::max() - gold) {
        return DailyRequestStatus::GoldFull;
    }
    return DailyRequestStatus::Ready;
}

bool CompleteDailyRequest(
    const DailyRequest& request,
    DailyRequestState& state,
    Inventory& inventory,
    std::uint16_t& gold) noexcept {
    if (GetDailyRequestStatus(request, state, inventory, gold) != DailyRequestStatus::Ready ||
        !inventory.Remove(request.item, request.requiredCount)) return false;
    gold = static_cast<std::uint16_t>(gold + request.reward);
    state.completed = true;
    return true;
}

} // namespace Homestead

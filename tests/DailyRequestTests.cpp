#include "Homestead/Game/DailyRequest.hpp"

#include <array>
#include <cstdint>
#include <limits>

int main() {
    const Homestead::DailyRequest first = Homestead::BuildDailyRequest(1);
    if (first.crop != Homestead::CropId::Wheat ||
        first.item != Homestead::ItemId::Wheat || first.requiredCount < 3 ||
        first.requiredCount > 5 || first.reward <= first.requiredCount * 7U ||
        Homestead::BuildDailyRequest(1).requiredCount != first.requiredCount) return 1;

    std::array<bool, 4> crops{};
    for (std::uint16_t day = 1; day <= 9; ++day) {
        const Homestead::DailyRequest request = Homestead::BuildDailyRequest(day);
        crops[static_cast<std::size_t>(request.crop)] = true;
    }
    if (!crops[static_cast<std::size_t>(Homestead::CropId::Wheat)] ||
        !crops[static_cast<std::size_t>(Homestead::CropId::Carrot)] ||
        !crops[static_cast<std::size_t>(Homestead::CropId::Tomato)]) return 2;

    Homestead::Inventory inventory;
    Homestead::DailyRequestState state;
    std::uint16_t gold = 20;
    if (Homestead::GetDailyRequestStatus(first, state, inventory, gold) !=
            Homestead::DailyRequestStatus::NeedMore ||
        Homestead::CompleteDailyRequest(first, state, inventory, gold) || gold != 20) return 3;
    if (inventory.Add(first.item, first.requiredCount) != 0 ||
        Homestead::GetDailyRequestStatus(first, state, inventory, gold) !=
            Homestead::DailyRequestStatus::Ready ||
        !Homestead::CompleteDailyRequest(first, state, inventory, gold) ||
        !state.completed || inventory.Count(first.item) != 0 || gold != 20 + first.reward) return 4;
    if (Homestead::CompleteDailyRequest(first, state, inventory, gold)) return 5;

    Homestead::Inventory overflowInventory;
    Homestead::DailyRequestState overflowState;
    if (overflowInventory.Add(first.item, first.requiredCount) != 0) return 6;
    std::uint16_t overflowGold = std::numeric_limits<std::uint16_t>::max();
    if (Homestead::GetDailyRequestStatus(first, overflowState, overflowInventory, overflowGold) !=
            Homestead::DailyRequestStatus::GoldFull ||
        Homestead::CompleteDailyRequest(first, overflowState, overflowInventory, overflowGold) ||
        overflowInventory.Count(first.item) != first.requiredCount || overflowState.completed) return 7;
    return 0;
}

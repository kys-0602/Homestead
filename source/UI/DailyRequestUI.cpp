#include "Homestead/UI/DailyRequestUI.hpp"

#include <cstddef>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Game/DailyRequest.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/UI/BitmapText.hpp"
#include "Homestead/UI/UIPanel.hpp"

namespace Homestead {
namespace {

constexpr float PanelX = 75.0F;
constexpr float PanelY = 38.0F;
constexpr float PanelWidth = 170.0F;
constexpr float PanelHeight = 104.0F;
constexpr std::uint32_t ButtonX = 105;
constexpr std::uint32_t ButtonY = 116;
constexpr std::uint32_t ButtonWidth = 110;
constexpr std::uint32_t ButtonHeight = 18;
constexpr std::uint32_t TextColor = 0xFF56331FU;

void AppendNumber(char* text, std::size_t& length, unsigned value) noexcept {
    char reversed[5]{};
    std::size_t count = 0;
    do { reversed[count++] = static_cast<char>('0' + value % 10U); value /= 10U; } while (value != 0);
    while (count != 0) text[length++] = reversed[--count];
    text[length] = '\0';
}

const char* CropName(CropId crop) noexcept {
    switch (crop) {
    case CropId::Wheat: return "WHEAT";
    case CropId::Carrot: return "CARROT";
    case CropId::Tomato: return "TOMATO";
    case CropId::None: return "NONE";
    }
    return "NONE";
}

} // namespace

bool DailyRequestButtonAt(std::uint32_t x, std::uint32_t y) noexcept {
    return x >= ButtonX && x < ButtonX + ButtonWidth &&
        y >= ButtonY && y < ButtonY + ButtonHeight;
}

bool AddDailyRequestUI(
    const DailyRequest& request,
    const DailyRequestState& state,
    const Inventory& inventory,
    std::uint16_t gold,
    const AssetStore& assets,
    RenderQueue& queue) noexcept {
    char need[24] = "NEED "; std::size_t needLength = 5;
    AppendNumber(need, needLength, request.requiredCount);
    need[needLength++] = ' ';
    const char* cropName = CropName(request.crop);
    while (*cropName != '\0') need[needLength++] = *cropName++;
    need[needLength] = '\0';

    char have[16] = "HAVE "; std::size_t haveLength = 5;
    AppendNumber(have, haveLength, inventory.Count(request.item));
    char reward[18] = "REWARD "; std::size_t rewardLength = 7;
    AppendNumber(reward, rewardLength, request.reward);
    reward[rewardLength++] = 'G'; reward[rewardLength] = '\0';

    const DailyRequestStatus status = GetDailyRequestStatus(request, state, inventory, gold);
    const char* action = status == DailyRequestStatus::Ready ? "DELIVER" :
        (status == DailyRequestStatus::Completed ? "COMPLETED" :
         (status == DailyRequestStatus::GoldFull ? "GOLD FULL" : "NEED MORE"));
    const std::uint32_t buttonColor = status == DailyRequestStatus::Ready ?
        0xFFFFFFFFU : 0xFFC8B8A0U;
    const float actionX = status == DailyRequestStatus::Completed ? 130.0F :
        (status == DailyRequestStatus::Ready ? 139.0F : 133.0F);

    return AddUIPanel(PanelX, PanelY, PanelWidth, PanelHeight, 0xFFFFFFFFU, 40, assets, queue) &&
        AddBitmapText("DAILY REQUEST", 121.0F, 49.0F, TextColor, 42, assets, queue) &&
        AddBitmapText(need, 103.0F, 68.0F, TextColor, 42, assets, queue) &&
        AddBitmapText(have, 103.0F, 80.0F, TextColor, 42, assets, queue) &&
        AddBitmapText(reward, 103.0F, 92.0F, TextColor, 42, assets, queue) &&
        AddUIPanel(static_cast<float>(ButtonX), static_cast<float>(ButtonY),
                   static_cast<float>(ButtonWidth), static_cast<float>(ButtonHeight),
                   buttonColor, 43, assets, queue) &&
        AddBitmapText(action, actionX, 123.0F, TextColor, 45, assets, queue);
}

} // namespace Homestead

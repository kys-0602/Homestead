#pragma once

#include <Windows.h>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Audio/Audio.hpp"
#include "Homestead/Core/FixedStep.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Game/Inventory.hpp"
#include "Homestead/Game/Economy.hpp"
#include "Homestead/Game/DailyRequest.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/Graphics.hpp"
#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Input/Input.hpp"
#include "Homestead/Platform/Clock.hpp"
#include "Homestead/Platform/Settings.hpp"
#include "Homestead/Platform/Window.hpp"
#include "Homestead/Save/SaveSystem.hpp"
#include "Homestead/World/TileMap.hpp"
#include "Homestead/World/CropField.hpp"
#include "Homestead/World/CropCatalogue.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/FarmAnimals.hpp"
#include "Homestead/World/WorldClock.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"

namespace Homestead {

enum class ApplicationInitializeResult : std::uint8_t {
    Ready,
    Cancelled,
    Failed
};

class Application final {
public:
    Application() = default;
    ~Application() noexcept;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    [[nodiscard]] ApplicationInitializeResult Initialize(
        HINSTANCE instance, int showCommand) noexcept;
    [[nodiscard]] int Run() noexcept;
    void Shutdown() noexcept;

private:
    [[nodiscard]] bool FixedUpdate() noexcept;
    [[nodiscard]] bool CaptureSave(SaveSnapshot& snapshot) const noexcept;
    [[nodiscard]] bool ApplySave(const SaveSnapshot& snapshot) noexcept;
    [[nodiscard]] bool SaveGame() noexcept;
    [[nodiscard]] bool ResetSave() noexcept;
    [[nodiscard]] bool ResetGameState() noexcept;
    [[nodiscard]] TileMap& ActiveMap() noexcept;
    [[nodiscard]] const TileMap& ActiveMap() const noexcept;
    [[nodiscard]] bool ChangeMap(MapId destination) noexcept;
    [[nodiscard]] bool UpdatePauseMenu() noexcept;
    [[nodiscard]] bool UpdateMarket() noexcept;
    [[nodiscard]] bool UpdateDailyRequest() noexcept;
    [[nodiscard]] bool UpdateCropCatalogue() noexcept;
    [[nodiscard]] bool ApplyDisplaySettings() noexcept;

    SettingsSystem settingsSystem_;
    Settings settings_;
    Window window_;
    Clock clock_;
    AssetStore assets_;
    Audio audio_;
    SaveSystem saves_;
    TileMap farmMap_;
    TileMap houseMap_;
    CropField crops_;
    CropCatalogue catalogue_;
    FarmAnimals farmAnimals_;
    WorldClock worldClock_;
    EntityWorld entityWorld_;
    PlayerState player_;
    Inventory inventory_;
    Graphics graphics_;
    Input input_;
    FixedStepController fixedStep_;
    Camera2D camera_{
        static_cast<float>(LogicalWidth),
        static_cast<float>(LogicalHeight)};
    RenderQueue renderQueue_;
    TileSelection selection_{};
    std::size_t selectedSlot_ = 0;
    std::size_t inventoryCursor_ = 0;
    std::size_t moveSource_ = Inventory::SlotCount;
    bool inventoryOpen_ = false;
    bool marketOpen_ = false;
    bool dailyRequestOpen_ = false;
    bool catalogueOpen_ = false;
    bool paused_ = false;
    bool collisionDebug_ = false;
    bool saveOnDayChange_ = false;
    bool resetConfirmation_ = false;
    std::uint8_t pauseFocus_ = 0;
    std::uint16_t instructionTicks_ = 600;
    std::uint16_t saveNoticeTicks_ = 0;
    std::uint16_t dailyRequestNoticeTicks_ = 0;
    std::uint32_t weatherTicks_ = 0;
    std::uint16_t gold_ = StartingGold;
    std::uint8_t marketFocus_ = 0;
    DailyRequestState dailyRequestState_{};
    MapId currentMap_ = MapId::Farm;
    bool initialized_ = false;
};

} // namespace Homestead

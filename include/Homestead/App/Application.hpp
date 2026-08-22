#pragma once

#include <Windows.h>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Core/FixedStep.hpp"
#include "Homestead/Game/PlayerState.hpp"
#include "Homestead/Game/Inventory.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/Graphics.hpp"
#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Input/Input.hpp"
#include "Homestead/Platform/Clock.hpp"
#include "Homestead/Platform/Window.hpp"
#include "Homestead/World/TileMap.hpp"
#include "Homestead/World/CropField.hpp"
#include "Homestead/World/EntityWorld.hpp"
#include "Homestead/World/WorldClock.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"

namespace Homestead {

class Application final {
public:
    Application() = default;
    ~Application() noexcept;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    [[nodiscard]] bool Initialize(HINSTANCE instance, int showCommand) noexcept;
    [[nodiscard]] int Run() noexcept;
    void Shutdown() noexcept;

private:
    [[nodiscard]] bool FixedUpdate() noexcept;

    Window window_;
    Clock clock_;
    AssetStore assets_;
    TileMap tileMap_;
    CropField crops_;
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
    std::uint16_t instructionTicks_ = 600;
    std::uint8_t harvestedCarrots_ = 0;
    bool goalComplete_ = false;
    bool initialized_ = false;
};

} // namespace Homestead

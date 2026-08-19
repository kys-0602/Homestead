#pragma once

#include <Windows.h>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Core/FixedStep.hpp"
#include "Homestead/Graphics/Camera2D.hpp"
#include "Homestead/Graphics/Graphics.hpp"
#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/RenderQueue.hpp"
#include "Homestead/Input/Input.hpp"
#include "Homestead/Platform/Clock.hpp"
#include "Homestead/Platform/Window.hpp"
#include "Homestead/World/TileMap.hpp"

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
    Graphics graphics_;
    Input input_;
    FixedStepController fixedStep_;
    Camera2D camera_{
        static_cast<float>(LogicalWidth),
        static_cast<float>(LogicalHeight)};
    RenderQueue renderQueue_;
    bool initialized_ = false;
};

} // namespace Homestead

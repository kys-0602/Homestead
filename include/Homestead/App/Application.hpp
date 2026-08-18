#pragma once

#include <Windows.h>

#include "Homestead/Core/FixedStep.hpp"
#include "Homestead/Graphics/Graphics.hpp"
#include "Homestead/Input/Input.hpp"
#include "Homestead/Platform/Clock.hpp"
#include "Homestead/Platform/Window.hpp"

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
    Graphics graphics_;
    Input input_;
    FixedStepController fixedStep_;
    float testSpriteX_ = 16.0F;
    float testSpriteY_ = 16.0F;
    bool testSpriteAlternateTint_ = false;
    bool initialized_ = false;
};

} // namespace Homestead

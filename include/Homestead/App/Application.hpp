#pragma once

#include <Windows.h>

#include "Homestead/Graphics/Graphics.hpp"
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
    Window window_;
    Clock clock_;
    Graphics graphics_;
    bool initialized_ = false;
};

} // namespace Homestead

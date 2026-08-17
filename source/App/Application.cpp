#include "Homestead/App/Application.hpp"

#include <Windows.h>

namespace Homestead {

Application::~Application() noexcept {
    Shutdown();
}

bool Application::Initialize(HINSTANCE instance, int showCommand) noexcept {
    if (initialized_) {
        return true;
    }

    if (!clock_.Initialize()) {
        return false;
    }

    if (!window_.Initialize(instance, showCommand)) {
        return false;
    }

    if (!graphics_.Initialize(
            window_.Handle(),
            window_.ClientWidth(),
            window_.ClientHeight())) {
        window_.Shutdown();
        return false;
    }

    initialized_ = true;
    return true;
}

int Application::Run() noexcept {
    if (!initialized_) {
        return 1;
    }

    while (window_.ProcessMessages()) {
        [[maybe_unused]] const double deltaSeconds = clock_.Tick();

        if (window_.IsMinimized()) {
            Sleep(16);
            continue;
        }

        if (!graphics_.Resize(window_.ClientWidth(), window_.ClientHeight()) ||
            !graphics_.Render()) {
            return 1;
        }
    }

    return 0;
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    graphics_.Shutdown();
    window_.Shutdown();
    initialized_ = false;
}

} // namespace Homestead

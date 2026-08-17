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

    initialized_ = true;
    return true;
}

int Application::Run() noexcept {
    if (!initialized_) {
        return 1;
    }

    while (window_.ProcessMessages()) {
        [[maybe_unused]] const double deltaSeconds = clock_.Tick();
        Sleep(1);
    }

    return 0;
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    window_.Shutdown();
    initialized_ = false;
}

} // namespace Homestead

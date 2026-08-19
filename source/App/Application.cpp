#include "Homestead/App/Application.hpp"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/TileMapRenderer.hpp"
#include "Homestead/Input/Action.hpp"

namespace Homestead {
namespace {

bool GetPakPath(wchar_t (&path)[MAX_PATH]) noexcept {
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }
    wchar_t* separator = path + length;
    while (separator != path && separator[-1] != L'\\') {
        --separator;
    }
    constexpr wchar_t fileName[] = L"data.pak";
    if (static_cast<std::size_t>(separator - path) + std::size(fileName) > MAX_PATH) {
        return false;
    }
    std::copy(std::begin(fileName), std::end(fileName), separator);
    return true;
}

} // namespace

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

    if (!window_.Initialize(instance, showCommand, input_)) {
        return false;
    }

    wchar_t pakPath[MAX_PATH]{};
    if (!GetPakPath(pakPath) || !assets_.LoadFile(pakPath)) {
        window_.Shutdown();
        return false;
    }

    if (!tileMap_.LoadMemory(assets_.MapData(), assets_.MapSize())) {
        assets_.Clear();
        window_.Shutdown();
        return false;
    }

    camera_.SetCenter({
        static_cast<float>(tileMap_.Width() * TileSize) * 0.5F,
        static_cast<float>(tileMap_.Height() * TileSize) * 0.5F});

    if (!graphics_.Initialize(
            window_.Handle(),
            window_.ClientWidth(),
            window_.ClientHeight(),
            assets_)) {
        assets_.Clear();
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

    for (;;) {
        input_.BeginFrame();
        if (!window_.ProcessMessages()) {
            break;
        }

        std::uint32_t logicalMouseX = 0;
        std::uint32_t logicalMouseY = 0;
        const double deltaSeconds = clock_.Tick();

        if (window_.IsMinimized()) {
            fixedStep_.Reset();
            Sleep(16);
            continue;
        }

        if (!graphics_.Resize(window_.ClientWidth(), window_.ClientHeight())) {
            return 1;
        }

        const bool logicalMouseValid = graphics_.ClientToLogical(
            input_.ClientMouseX(),
            input_.ClientMouseY(),
            logicalMouseX,
            logicalMouseY);
        input_.SetLogicalMouse(logicalMouseX, logicalMouseY, logicalMouseValid);

        const FixedStepFrame fixedFrame = fixedStep_.Advance(deltaSeconds);
        for (std::uint32_t step = 0; step < fixedFrame.steps; ++step) {
            if (!FixedUpdate()) {
                return 1;
            }
        }

        if (!TileMapRenderer::Build(tileMap_, camera_, assets_, renderQueue_) ||
            !graphics_.Render(renderQueue_)) {
            return 1;
        }
    }

    return 0;
}

bool Application::FixedUpdate() noexcept {
    constexpr float movementPerStep =
        60.0F * static_cast<float>(FixedStepController::StepSeconds);
    Float2 center = camera_.Center();

    if (input_.Held(Action::MoveLeft)) {
        center.x -= movementPerStep;
    }
    if (input_.Held(Action::MoveRight)) {
        center.x += movementPerStep;
    }
    if (input_.Held(Action::MoveUp)) {
        center.y -= movementPerStep;
    }
    if (input_.Held(Action::MoveDown)) {
        center.y += movementPerStep;
    }

    [[maybe_unused]] const bool interactPressed = input_.ConsumePressed(Action::Interact);
    [[maybe_unused]] const bool useToolPressed = input_.ConsumePressed(Action::UseTool);
    [[maybe_unused]] const bool menuPressed = input_.ConsumePressed(Action::Menu);
    camera_.SetCenter(center);
    return true;
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    graphics_.Shutdown();
    tileMap_.Clear();
    assets_.Clear();
    window_.Shutdown();
    fixedStep_.Reset();
    initialized_ = false;
}

} // namespace Homestead

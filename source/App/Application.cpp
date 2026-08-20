#include "Homestead/App/Application.hpp"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/PlayerRenderer.hpp"
#include "Homestead/Graphics/SelectionRenderer.hpp"
#include "Homestead/Graphics/TileMapRenderer.hpp"
#include "Homestead/Input/Action.hpp"
#include "Homestead/Systems/PlayerMovement.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/Systems/ToolSystem.hpp"

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

    player_.entity = entityWorld_.Create(
        {static_cast<float>(tileMap_.Width() * TileSize) * 0.5F,
         static_cast<float>(tileMap_.Height() * TileSize) * 0.5F},
        MakeAssetId("player.idle.down.0"));
    if (!entityWorld_.IsAlive(player_.entity)) {
        tileMap_.Clear();
        assets_.Clear();
        window_.Shutdown();
        return false;
    }

    if (!graphics_.Initialize(
            window_.Handle(),
            window_.ClientWidth(),
            window_.ClientHeight(),
            assets_)) {
        entityWorld_.Clear();
        tileMap_.Clear();
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

        const TransformComponent* playerTransform = entityWorld_.Transform(player_.entity);
        if (playerTransform == nullptr) {
            return 1;
        }
        const float alpha = static_cast<float>(fixedFrame.alpha);
        const Float2 playerPosition{
            playerTransform->previous.x +
                (playerTransform->current.x - playerTransform->previous.x) * alpha,
            playerTransform->previous.y +
                (playerTransform->current.y - playerTransform->previous.y) * alpha};
        camera_.SetCenterClamped(
            playerPosition,
            static_cast<float>(tileMap_.Width() * TileSize),
            static_cast<float>(tileMap_.Height() * TileSize));

        if (!TileMapRenderer::Build(tileMap_, camera_, assets_, renderQueue_) ||
            !PlayerRenderer::Add(
                entityWorld_, player_, alpha, camera_, assets_, renderQueue_) ||
            !AddSelectionOverlay(selection_, camera_, assets_, renderQueue_)) {
            return 1;
        }
        renderQueue_.Sort();
        if (!graphics_.Render(renderQueue_)) {
            return 1;
        }
    }

    return 0;
}

bool Application::FixedUpdate() noexcept {
    MovementInput movement{};
    movement.x = (input_.Held(Action::MoveRight) ? 1.0F : 0.0F) -
        (input_.Held(Action::MoveLeft) ? 1.0F : 0.0F);
    movement.y = (input_.Held(Action::MoveDown) ? 1.0F : 0.0F) -
        (input_.Held(Action::MoveUp) ? 1.0F : 0.0F);

    PhysicalKey interactSource = PhysicalKey::Count;
    PhysicalKey toolSource = PhysicalKey::Count;
    const bool interactPressed = input_.ConsumePressed(Action::Interact, interactSource);
    const bool useToolPressed = input_.ConsumePressed(Action::UseTool, toolSource);
    [[maybe_unused]] const bool menuPressed = input_.ConsumePressed(Action::Menu);
    if (!UpdatePlayerMovement(
        entityWorld_, player_, tileMap_, movement,
        static_cast<float>(FixedStepController::StepSeconds))) {
        return false;
    }

    const TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) {
        return false;
    }
    camera_.SetCenterClamped(
        {transform->current.x, transform->current.y},
        static_cast<float>(tileMap_.Width() * TileSize),
        static_cast<float>(tileMap_.Height() * TileSize));
    const WorldPosition playerFeet = transform->current;
    TileSelection mouseSelection{};
    if (input_.IsLogicalMouseValid()) {
        const Float2 mouseWorld = camera_.ScreenToWorld({
            static_cast<float>(input_.LogicalMouseX()),
            static_cast<float>(input_.LogicalMouseY())});
        mouseSelection = SelectMouseTile(
            playerFeet, {mouseWorld.x, mouseWorld.y}, tileMap_);
        selection_ = mouseSelection;
    } else {
        selection_ = SelectFrontTile(playerFeet, player_.facing, tileMap_);
    }

    if (interactPressed) {
        const TileSelection interactionTarget =
            interactSource == PhysicalKey::MouseRight && input_.IsLogicalMouseValid() ?
            mouseSelection : SelectFrontTile(playerFeet, player_.facing, tileMap_);
        selection_ = interactionTarget;
        [[maybe_unused]] const bool interacted =
            TryInteract(player_, tileMap_, interactionTarget);
    }
    if (useToolPressed) {
        const TileSelection toolTarget =
            toolSource == PhysicalKey::MouseLeft && input_.IsLogicalMouseValid() ?
            mouseSelection : SelectFrontTile(playerFeet, player_.facing, tileMap_);
        selection_ = toolTarget;
        if (TryStartToolUse(player_, tileMap_, toolTarget)) {
            FaceSelection(player_, playerFeet, toolTarget);
        }
    }
    return UpdateToolUse(entityWorld_, player_, tileMap_);
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    graphics_.Shutdown();
    entityWorld_.Clear();
    tileMap_.Clear();
    assets_.Clear();
    window_.Shutdown();
    fixedStep_.Reset();
    initialized_ = false;
}

} // namespace Homestead

#include "Homestead/App/Application.hpp"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "Homestead/Graphics/Presentation.hpp"
#include "Homestead/Graphics/PlayerRenderer.hpp"
#include "Homestead/Graphics/CropRenderer.hpp"
#include "Homestead/Graphics/SelectionRenderer.hpp"
#include "Homestead/Graphics/TileMapRenderer.hpp"
#include "Homestead/Input/Action.hpp"
#include "Homestead/Systems/PlayerMovement.hpp"
#include "Homestead/Systems/InteractionSystem.hpp"
#include "Homestead/Systems/ToolSystem.hpp"
#include "Homestead/UI/InventoryUI.hpp"

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
    if (inventory_.Add(ItemId::Hoe, 1) != 0 ||
        inventory_.Add(ItemId::WateringCan, 1) != 0 ||
        inventory_.Add(ItemId::CarrotSeed, 12) != 0 ||
        inventory_.Add(ItemId::Carrot, 3) != 0) {
        inventory_.Clear();
        entityWorld_.Clear();
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
        inventory_.Clear();
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
            !AddCrops(crops_, camera_, assets_, renderQueue_) ||
            !PlayerRenderer::Add(
                entityWorld_, player_, alpha, camera_, assets_, renderQueue_) ||
            (!inventoryOpen_ && !AddSelectionOverlay(selection_, camera_, assets_, renderQueue_)) ||
            !AddInventoryUI(inventory_, selectedSlot_, inventoryCursor_,
                            inventoryOpen_, assets_, renderQueue_)) {
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
    if (input_.ConsumePressed(Action::Menu)) {
        inventoryOpen_ = !inventoryOpen_;
        inventoryCursor_ = selectedSlot_;
        moveSource_ = Inventory::SlotCount;
    }

    for (std::size_t index = 0; index < Inventory::HotbarSlotCount; ++index) {
        const Action action = static_cast<Action>(
            static_cast<std::uint8_t>(Action::Hotbar1) + static_cast<std::uint8_t>(index));
        if (input_.ConsumePressed(action)) {
            selectedSlot_ = index;
            inventoryCursor_ = index;
        }
    }

    if (inventoryOpen_) {
        const bool left = input_.ConsumePressed(Action::MoveLeft);
        const bool right = input_.ConsumePressed(Action::MoveRight);
        const bool up = input_.ConsumePressed(Action::MoveUp);
        const bool down = input_.ConsumePressed(Action::MoveDown);
        if (left && inventoryCursor_ % 8 > 0) --inventoryCursor_;
        if (right && inventoryCursor_ % 8 < 7) ++inventoryCursor_;
        if ((up || down)) inventoryCursor_ = (inventoryCursor_ + 8) % Inventory::SlotCount;

        PhysicalKey interactSource = PhysicalKey::Count;
        PhysicalKey source = PhysicalKey::Count;
        const bool interactActivate = input_.ConsumePressed(Action::Interact, interactSource);
        const bool toolActivate = input_.ConsumePressed(Action::UseTool, source);
        bool activate = interactActivate || toolActivate;
        const bool mouseActivate = source == PhysicalKey::MouseLeft ||
            interactSource == PhysicalKey::MouseRight;
        if (mouseActivate && input_.IsLogicalMouseValid()) {
            const int hit = InventorySlotAt(input_.LogicalMouseX(), input_.LogicalMouseY(), true);
            if (hit >= 0) inventoryCursor_ = static_cast<std::size_t>(hit);
            else activate = false;
        } else if (mouseActivate) {
            activate = false;
        }
        if (activate) {
            if (moveSource_ == Inventory::SlotCount) {
                if (inventory_.Slot(inventoryCursor_).item != ItemId::None) moveSource_ = inventoryCursor_;
            } else {
                if (!inventory_.Move(moveSource_, inventoryCursor_)) {
                    [[maybe_unused]] const bool exchanged = inventory_.Exchange(moveSource_, inventoryCursor_);
                }
                moveSource_ = Inventory::SlotCount;
            }
        }
        return true;
    }

    MovementInput movement{};
    movement.x = (input_.Held(Action::MoveRight) ? 1.0F : 0.0F) -
        (input_.Held(Action::MoveLeft) ? 1.0F : 0.0F);
    movement.y = (input_.Held(Action::MoveDown) ? 1.0F : 0.0F) -
        (input_.Held(Action::MoveUp) ? 1.0F : 0.0F);

    PhysicalKey interactSource = PhysicalKey::Count;
    PhysicalKey toolSource = PhysicalKey::Count;
    const bool interactPressed = input_.ConsumePressed(Action::Interact, interactSource);
    const bool useToolPressed = input_.ConsumePressed(Action::UseTool, toolSource);

    if (useToolPressed && toolSource == PhysicalKey::MouseLeft && input_.IsLogicalMouseValid()) {
        const int hotbar = InventorySlotAt(input_.LogicalMouseX(), input_.LogicalMouseY(), false);
        if (hotbar >= 0) {
            selectedSlot_ = static_cast<std::size_t>(hotbar);
            inventoryCursor_ = selectedSlot_;
            return true;
        }
    }

    const TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) {
        return false;
    }
    camera_.SetCenterClamped(
        {transform->current.x, transform->current.y},
        static_cast<float>(tileMap_.Width() * TileSize),
        static_cast<float>(tileMap_.Height() * TileSize));
    WorldPosition playerFeet = transform->current;
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
        if (player_.toolUse.action == ToolAction::None &&
            crops_.Harvest(inventory_, interactionTarget)) {
            FaceSelection(player_, playerFeet, interactionTarget);
        } else {
            [[maybe_unused]] const bool interacted =
                TryInteract(player_, tileMap_, interactionTarget);
        }
    }
    if (useToolPressed) {
        const TileSelection toolTarget =
            toolSource == PhysicalKey::MouseLeft && input_.IsLogicalMouseValid() ?
            mouseSelection : SelectFrontTile(playerFeet, player_.facing, tileMap_);
        selection_ = toolTarget;
        ToolAction action = ToolAction::None;
        const ItemId selectedItem = inventory_.Slot(selectedSlot_).item;
        if (selectedItem == ItemId::Hoe) action = ToolAction::Hoe;
        else if (selectedItem == ItemId::WateringCan) action = ToolAction::Watering;
        if (selectedItem == ItemId::CarrotSeed &&
            player_.toolUse.action == ToolAction::None &&
            crops_.Plant(tileMap_, inventory_, toolTarget, selectedItem)) {
            FaceSelection(player_, playerFeet, toolTarget);
        } else if (TryStartToolUse(player_, tileMap_, toolTarget, action)) {
            FaceSelection(player_, playerFeet, toolTarget);
        }
    }

    if (!UpdatePlayerMovement(
        entityWorld_, player_, tileMap_, movement,
        static_cast<float>(FixedStepController::StepSeconds))) {
        return false;
    }
    transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) {
        return false;
    }
    playerFeet = transform->current;
    camera_.SetCenterClamped(
        {playerFeet.x, playerFeet.y},
        static_cast<float>(tileMap_.Width() * TileSize),
        static_cast<float>(tileMap_.Height() * TileSize));
    if (!interactPressed && !useToolPressed) {
        if (input_.IsLogicalMouseValid()) {
            const Float2 mouseWorld = camera_.ScreenToWorld({
                static_cast<float>(input_.LogicalMouseX()),
                static_cast<float>(input_.LogicalMouseY())});
            selection_ = SelectMouseTile(
                playerFeet, {mouseWorld.x, mouseWorld.y}, tileMap_);
        } else {
            selection_ = SelectFrontTile(playerFeet, player_.facing, tileMap_);
        }
    }
    return UpdateToolUse(entityWorld_, player_, tileMap_);
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    graphics_.Shutdown();
    crops_.Clear();
    inventory_.Clear();
    entityWorld_.Clear();
    tileMap_.Clear();
    assets_.Clear();
    window_.Shutdown();
    fixedStep_.Reset();
    initialized_ = false;
}

} // namespace Homestead

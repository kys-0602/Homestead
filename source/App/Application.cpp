#include "Homestead/App/Application.hpp"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <cmath>

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
#include "Homestead/UI/PauseUI.hpp"
#include "Homestead/UI/StatusUI.hpp"

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

    [[maybe_unused]] const bool loadedSettings = settingsSystem_.Load(settings_);
    if (!IsValidSettings(settings_)) settings_ = {};
    if (!window_.Initialize(instance, showCommand, input_,
                            settings_.windowScale, settings_.fullscreen)) {
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

    SaveSnapshot snapshot;
    const SaveLoadResult loadResult = saves_.Load(snapshot);
    if ((loadResult == SaveLoadResult::LoadedPrimary ||
         loadResult == SaveLoadResult::LoadedBackup) && !ApplySave(snapshot)) {
        crops_.Clear();
        inventory_.Clear();
        [[maybe_unused]] const std::uint16_t hoe = inventory_.Add(ItemId::Hoe, 1);
        [[maybe_unused]] const std::uint16_t watering = inventory_.Add(ItemId::WateringCan, 1);
        [[maybe_unused]] const std::uint16_t seeds = inventory_.Add(ItemId::CarrotSeed, 12);
        [[maybe_unused]] const std::uint16_t carrots = inventory_.Add(ItemId::Carrot, 3);
        worldClock_.Reset();
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
            (!inventoryOpen_ && !paused_ && !AddSelectionOverlay(selection_, camera_, assets_, renderQueue_)) ||
            !AddInventoryUI(inventory_, selectedSlot_, inventoryCursor_,
                            inventoryOpen_, assets_, renderQueue_) ||
            !AddStatusUI(worldClock_, harvestedCarrots_, 3,
                         instructionTicks_ != 0, goalComplete_, assets_, renderQueue_) ||
            (paused_ && !AddPauseUI(settings_, pauseFocus_, assets_, renderQueue_))) {
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
    if (worldClock_.IsTransitioning()) {
        input_.DiscardPending();
        if (worldClock_.FixedUpdate()) {
            crops_.OnDayChanged(tileMap_);
            [[maybe_unused]] const bool saved = SaveGame();
        }
        return true;
    }
    if (input_.ConsumePressed(Action::Menu)) {
        paused_ = !paused_;
        inventoryOpen_ = false;
        moveSource_ = Inventory::SlotCount;
        if (!paused_) [[maybe_unused]] const bool savedSettings = settingsSystem_.Save(settings_);
        input_.DiscardPending();
        return true;
    }
    if (paused_) return UpdatePauseMenu();
    if (goalComplete_) {
        PhysicalKey interactSource = PhysicalKey::Count;
        PhysicalKey toolSource = PhysicalKey::Count;
        const bool interact = input_.ConsumePressed(Action::Interact, interactSource);
        const bool tool = input_.ConsumePressed(Action::UseTool, toolSource);
        bool resume = interact || tool;
        const bool mouse = interactSource == PhysicalKey::MouseRight ||
            toolSource == PhysicalKey::MouseLeft;
        if (mouse) {
            resume = input_.IsLogicalMouseValid() &&
                CompletionContinueAt(input_.LogicalMouseX(), input_.LogicalMouseY());
        }
        if (resume) goalComplete_ = false;
        input_.DiscardPending();
        return true;
    }
    if (input_.ConsumePressed(Action::Inventory)) {
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
        [[maybe_unused]] const bool ignoredEndDay = input_.ConsumePressed(Action::EndDay);
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

    if (input_.ConsumePressed(Action::EndDay) &&
        player_.toolUse.action == ToolAction::None) {
        [[maybe_unused]] const bool started = worldClock_.RequestEndDay();
        instructionTicks_ = 0;
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
            if (harvestedCarrots_ < 3) ++harvestedCarrots_;
            goalComplete_ = harvestedCarrots_ >= 3;
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
    if (!UpdateToolUse(entityWorld_, player_, tileMap_)) return false;
    if (instructionTicks_ != 0) --instructionTicks_;
    [[maybe_unused]] const bool unexpectedDayChange = worldClock_.FixedUpdate();
    return !unexpectedDayChange;
}

void Application::Shutdown() noexcept {
    if (!initialized_) {
        return;
    }

    [[maybe_unused]] const bool saved = SaveGame();
    [[maybe_unused]] const bool savedSettings = settingsSystem_.Save(settings_);
    graphics_.Shutdown();
    crops_.Clear();
    worldClock_.Reset();
    inventory_.Clear();
    entityWorld_.Clear();
    tileMap_.Clear();
    assets_.Clear();
    window_.Shutdown();
    fixedStep_.Reset();
    instructionTicks_ = 600;
    harvestedCarrots_ = 0;
    goalComplete_ = false;
    inventoryOpen_ = false;
    paused_ = false;
    pauseFocus_ = 0;
    initialized_ = false;
}

bool Application::UpdatePauseMenu() noexcept {
    [[maybe_unused]] const bool ignoredInventory = input_.ConsumePressed(Action::Inventory);
    const bool up = input_.ConsumePressed(Action::MoveUp);
    const bool down = input_.ConsumePressed(Action::MoveDown);
    const bool left = input_.ConsumePressed(Action::MoveLeft);
    const bool right = input_.ConsumePressed(Action::MoveRight);
    if (up) pauseFocus_ = pauseFocus_ == 0 ? PauseItemCount - 1 : pauseFocus_ - 1;
    if (down) pauseFocus_ = static_cast<std::uint8_t>((pauseFocus_ + 1) % PauseItemCount);

    PhysicalKey interactSource = PhysicalKey::Count;
    PhysicalKey toolSource = PhysicalKey::Count;
    const bool interact = input_.ConsumePressed(Action::Interact, interactSource);
    const bool tool = input_.ConsumePressed(Action::UseTool, toolSource);
    bool activate = interact || tool;
    const bool mouse = interactSource == PhysicalKey::MouseRight || toolSource == PhysicalKey::MouseLeft;
    if (input_.IsLogicalMouseValid()) {
        const int hover = PauseItemAt(input_.LogicalMouseX(), input_.LogicalMouseY());
        if (hover >= 0) pauseFocus_ = static_cast<std::uint8_t>(hover);
        else if (mouse) activate = false;
    } else if (mouse) activate = false;

    bool displayChanged = false;
    bool settingsChanged = false;
    const int direction = right ? 1 : (left ? -1 : 0);
    if (pauseFocus_ == 0 && activate) paused_ = false;
    else if (pauseFocus_ == 1 && activate) {
        paused_ = false; inventoryOpen_ = true; inventoryCursor_ = selectedSlot_;
    } else if (pauseFocus_ == 2 && (activate || direction != 0)) {
        settings_.windowScale = direction < 0 ?
            (settings_.windowScale == 2 ? 4 : settings_.windowScale - 1) :
            (settings_.windowScale == 4 ? 2 : settings_.windowScale + 1);
        displayChanged = settingsChanged = true;
    } else if (pauseFocus_ == 3 && (activate || direction != 0)) {
        settings_.fullscreen = !settings_.fullscreen;
        displayChanged = settingsChanged = true;
    } else if (pauseFocus_ >= 4 && (activate || direction != 0)) {
        std::uint8_t* volume = pauseFocus_ == 4 ? &settings_.masterVolume :
            (pauseFocus_ == 5 ? &settings_.musicVolume : &settings_.effectVolume);
        const int step = direction != 0 ? direction : 1;
        *volume = static_cast<std::uint8_t>(step < 0 ? (*volume == 0 ? 10 : *volume - 1) :
                                                    (*volume == 10 ? 0 : *volume + 1));
        settingsChanged = true;
    }
    if (displayChanged && !ApplyDisplaySettings()) return false;
    if (settingsChanged || !paused_) [[maybe_unused]] const bool saved = settingsSystem_.Save(settings_);
    return true;
}

bool Application::ApplyDisplaySettings() noexcept {
    return window_.ApplyDisplaySettings(settings_.windowScale, settings_.fullscreen);
}

bool Application::CaptureSave(SaveSnapshot& snapshot) const noexcept {
    const TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) return false;
    snapshot = {};
    snapshot.playerX256 = static_cast<std::int32_t>(std::lround(transform->current.x * 256.0F));
    snapshot.playerY256 = static_cast<std::int32_t>(std::lround(transform->current.y * 256.0F));
    snapshot.day = worldClock_.Day(); snapshot.minute = worldClock_.Minute();
    snapshot.selectedSlot = static_cast<std::uint8_t>(selectedSlot_);
    snapshot.harvestedCarrots = harvestedCarrots_;
    for (std::size_t index = 0; index < Inventory::SlotCount; ++index)
        snapshot.inventory[index] = inventory_.Slot(index);
    for (std::int32_t y = 0; y < tileMap_.Height(); ++y) {
        for (std::int32_t x = 0; x < tileMap_.Width(); ++x) {
            const Tile* tile = tileMap_.Get(x, y);
            const std::uint8_t dynamic = tile == nullptr ? 0 : static_cast<std::uint8_t>(
                tile->flags & (TileFlagValue(TileFlag::Tilled) | TileFlagValue(TileFlag::Watered)));
            if (dynamic != 0) snapshot.tileDeltas.push_back({
                static_cast<std::uint8_t>(x), static_cast<std::uint8_t>(y), dynamic});
        }
    }
    for (const CropInstance& crop : crops_.Crops()) if (crop.active) snapshot.crops.push_back(crop);
    return snapshot.tileDeltas.size() <= MaximumTileDeltas;
}

bool Application::ApplySave(const SaveSnapshot& snapshot) noexcept {
    const float playerX = static_cast<float>(snapshot.playerX256) / 256.0F;
    const float playerY = static_cast<float>(snapshot.playerY256) / 256.0F;
    if (playerX < 0.0F || playerY < 0.0F ||
        playerX >= tileMap_.Width() * TileSize || playerY >= tileMap_.Height() * TileSize ||
        snapshot.selectedSlot >= Inventory::HotbarSlotCount || snapshot.harvestedCarrots > 3) return false;
    for (std::size_t index = 0; index < snapshot.tileDeltas.size(); ++index) {
        const SavedTileDelta& delta = snapshot.tileDeltas[index];
        const Tile* tile = tileMap_.Get(delta.x, delta.y);
        if (tile == nullptr || tile->object != 0 ||
            (tile->flags & (TileFlagValue(TileFlag::Blocked) | TileFlagValue(TileFlag::Water))) != 0) return false;
        for (std::size_t other = 0; other < index; ++other)
            if (snapshot.tileDeltas[other].x == delta.x && snapshot.tileDeltas[other].y == delta.y) return false;
    }
    for (std::size_t index = 0; index < snapshot.crops.size(); ++index) {
        const CropInstance& crop = snapshot.crops[index];
        bool tilled = false;
        for (const SavedTileDelta& delta : snapshot.tileDeltas)
            if (delta.x == crop.tileX && delta.y == crop.tileY &&
                (delta.flags & TileFlagValue(TileFlag::Tilled)) != 0) tilled = true;
        if (!tilled) return false;
        for (std::size_t other = 0; other < index; ++other)
            if (snapshot.crops[other].tileX == crop.tileX && snapshot.crops[other].tileY == crop.tileY) return false;
    }
    if (!worldClock_.Restore(snapshot.day, snapshot.minute)) return false;
    inventory_.Clear();
    for (std::size_t index = 0; index < Inventory::SlotCount; ++index) inventory_.Slot(index) = snapshot.inventory[index];
    for (const SavedTileDelta& delta : snapshot.tileDeltas) {
        Tile* tile = tileMap_.Get(delta.x, delta.y); tile->flags |= delta.flags;
    }
    crops_.Clear();
    for (const CropInstance& crop : snapshot.crops) if (!crops_.Restore(crop, tileMap_)) return false;
    TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) return false;
    transform->current = {playerX, playerY}; transform->previous = transform->current;
    selectedSlot_ = snapshot.selectedSlot; inventoryCursor_ = selectedSlot_;
    harvestedCarrots_ = snapshot.harvestedCarrots; goalComplete_ = harvestedCarrots_ >= 3;
    instructionTicks_ = 0;
    return true;
}

bool Application::SaveGame() noexcept {
    SaveSnapshot snapshot;
    return CaptureSave(snapshot) && saves_.Save(snapshot);
}

} // namespace Homestead

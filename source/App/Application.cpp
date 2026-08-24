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
#include "Homestead/UI/MarketUI.hpp"
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

    const MapAsset* farmAsset = assets_.FindMap(MakeAssetId("map/farm"));
    const MapAsset* houseAsset = assets_.FindMap(MakeAssetId("map/house"));
    if (farmAsset == nullptr || houseAsset == nullptr ||
        !farmMap_.LoadMemory(farmAsset->bytes.data(), farmAsset->bytes.size()) ||
        !houseMap_.LoadMemory(houseAsset->bytes.data(), houseAsset->bytes.size())) {
        assets_.Clear();
        window_.Shutdown();
        return false;
    }
    if (audio_.Initialize(assets_)) {
        audio_.SetVolumes(settings_.masterVolume, settings_.musicVolume, settings_.effectVolume);
        [[maybe_unused]] const bool musicStarted =
            audio_.PlayMusic(MakeAssetId("audio.music.farm"));
    }

    player_.entity = entityWorld_.Create(
        {16.5F * TileSize, 12.5F * TileSize},
        MakeAssetId("player.idle.down.0"));
    if (!entityWorld_.IsAlive(player_.entity)) {
        farmMap_.Clear(); houseMap_.Clear();
        assets_.Clear();
        window_.Shutdown();
        return false;
    }
    if (inventory_.Add(ItemId::Hoe, 1) != 0 ||
        inventory_.Add(ItemId::WateringCan, 1) != 0) {
        inventory_.Clear();
        entityWorld_.Clear();
        farmMap_.Clear(); houseMap_.Clear();
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
        worldClock_.Reset();
        gold_ = StartingGold;
    }

    if (!graphics_.Initialize(
            window_.Handle(),
            window_.ClientWidth(),
            window_.ClientHeight(),
            assets_)) {
        inventory_.Clear();
        entityWorld_.Clear();
        farmMap_.Clear(); houseMap_.Clear();
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
            static_cast<float>(ActiveMap().Width() * TileSize),
            static_cast<float>(ActiveMap().Height() * TileSize));

        if (!TileMapRenderer::Build(ActiveMap(), camera_, assets_, renderQueue_) ||
            (currentMap_ == MapId::Farm && !AddCrops(crops_, camera_, assets_, renderQueue_)) ||
            !PlayerRenderer::Add(
                entityWorld_, player_, alpha, camera_, assets_, renderQueue_) ||
            (!inventoryOpen_ && !paused_ && !AddSelectionOverlay(selection_, camera_, assets_, renderQueue_)) ||
            !AddInventoryUI(inventory_, selectedSlot_, inventoryCursor_,
                            inventoryOpen_, assets_, renderQueue_) ||
            !AddStatusUI(worldClock_, gold_, GoalGold, instructionTicks_ != 0,
                         saveNoticeTicks_ != 0, goalComplete_, assets_, renderQueue_) ||
            (marketOpen_ && !AddMarketUI(gold_, marketFocus_, assets_, renderQueue_)) ||
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
            crops_.OnDayChanged(farmMap_);
            if (SaveGame()) saveNoticeTicks_ = 180;
        }
        return true;
    }
    if (saveNoticeTicks_ != 0) --saveNoticeTicks_;
    if (input_.ConsumePressed(Action::Menu)) {
        if (marketOpen_) { marketOpen_ = false; input_.DiscardPending(); return true; }
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
    if (input_.ConsumePressed(Action::Market)) {
        marketOpen_ = !marketOpen_; inventoryOpen_ = false; input_.DiscardPending(); return true;
    }
    if (marketOpen_) return UpdateMarket();
    if (input_.ConsumePressed(Action::Inventory)) {
        inventoryOpen_ = !inventoryOpen_;
        inventoryCursor_ = selectedSlot_;
        moveSource_ = Inventory::SlotCount;
    }

    for (std::size_t index = 0; index < Inventory::HotbarSlotCount; ++index) {
        const Action action = static_cast<Action>(
            static_cast<std::uint8_t>(Action::Hotbar1) + static_cast<std::uint8_t>(index));
        if (input_.ConsumePressed(action)) {
            if (selectedSlot_ != index) audio_.PlayEffect(MakeAssetId("audio.ui.move"));
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
        if (left || right || up || down) audio_.PlayEffect(MakeAssetId("audio.ui.move"));

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
            audio_.PlayEffect(MakeAssetId("audio.ui.confirm"));
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
        static_cast<float>(ActiveMap().Width() * TileSize),
        static_cast<float>(ActiveMap().Height() * TileSize));
    WorldPosition playerFeet = transform->current;
    TileSelection mouseSelection{};
    if (input_.IsLogicalMouseValid()) {
        const Float2 mouseWorld = camera_.ScreenToWorld({
            static_cast<float>(input_.LogicalMouseX()),
            static_cast<float>(input_.LogicalMouseY())});
        mouseSelection = SelectMouseTile(
            playerFeet, {mouseWorld.x, mouseWorld.y}, ActiveMap());
        selection_ = mouseSelection;
    } else {
        selection_ = SelectFrontTile(playerFeet, player_.facing, ActiveMap());
    }

    if (interactPressed) {
        TileSelection interactionTarget =
            interactSource == PhysicalKey::MouseRight && input_.IsLogicalMouseValid() ?
            mouseSelection : SelectFrontTile(playerFeet, player_.facing, ActiveMap());
        if (interactSource != PhysicalKey::MouseRight) {
            const TileSelection nearby = SelectNearbySpecialObject(playerFeet, ActiveMap());
            if (nearby.valid) interactionTarget = nearby;
        }
        selection_ = interactionTarget;
        if (player_.toolUse.action == ToolAction::None &&
            currentMap_ == MapId::Farm && crops_.Harvest(inventory_, interactionTarget)) {
            FaceSelection(player_, playerFeet, interactionTarget);
            audio_.PlayEffect(MakeAssetId("audio.harvest"));
        } else {
            const Tile* tile = interactionTarget.valid ? ActiveMap().Get(interactionTarget.x, interactionTarget.y) : nullptr;
            const bool enterHouse = currentMap_ == MapId::Farm && interactionTarget.inRange &&
                tile != nullptr && tile->object == static_cast<std::uint16_t>(TileGraphic::Farmhouse);
            const bool leaveHouse = currentMap_ == MapId::House && interactionTarget.inRange &&
                tile != nullptr && tile->object == static_cast<std::uint16_t>(TileGraphic::Door);
            if ((enterHouse && ChangeMap(MapId::House)) || (leaveHouse && ChangeMap(MapId::Farm))) {
                audio_.PlayEffect(MakeAssetId("audio.ui.confirm"));
                return true;
            }
            if (currentMap_ == MapId::House && interactionTarget.inRange && tile != nullptr &&
                       tile->object == static_cast<std::uint16_t>(TileGraphic::Bed)) {
                if (worldClock_.RequestEndDay()) {
                    instructionTicks_ = 0;
                    audio_.PlayEffect(MakeAssetId("audio.ui.confirm"));
                }
            } else {
                [[maybe_unused]] const bool interacted =
                    TryInteract(player_, ActiveMap(), interactionTarget);
            }
        }
    }
    if (useToolPressed) {
        const TileSelection toolTarget =
            toolSource == PhysicalKey::MouseLeft && input_.IsLogicalMouseValid() ?
            mouseSelection : SelectFrontTile(playerFeet, player_.facing, ActiveMap());
        selection_ = toolTarget;
        ToolAction action = ToolAction::None;
        const ItemId selectedItem = inventory_.Slot(selectedSlot_).item;
        if (selectedItem == ItemId::Hoe) action = ToolAction::Hoe;
        else if (selectedItem == ItemId::WateringCan) action = ToolAction::Watering;
        const ItemDefinition* selectedDefinition = FindItemDefinition(selectedItem);
        if (selectedDefinition != nullptr && selectedDefinition->category == ItemCategory::Seed &&
            player_.toolUse.action == ToolAction::None &&
            currentMap_ == MapId::Farm && crops_.Plant(farmMap_, inventory_, toolTarget, selectedItem)) {
            FaceSelection(player_, playerFeet, toolTarget);
            audio_.PlayEffect(MakeAssetId("audio.plant"));
        } else if (currentMap_ == MapId::Farm && TryStartToolUse(player_, farmMap_, toolTarget, action)) {
            FaceSelection(player_, playerFeet, toolTarget);
        }
    }

    if (!UpdatePlayerMovement(
        entityWorld_, player_, ActiveMap(), movement,
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
        static_cast<float>(ActiveMap().Width() * TileSize),
        static_cast<float>(ActiveMap().Height() * TileSize));
    if (!interactPressed && !useToolPressed) {
        if (input_.IsLogicalMouseValid()) {
            const Float2 mouseWorld = camera_.ScreenToWorld({
                static_cast<float>(input_.LogicalMouseX()),
                static_cast<float>(input_.LogicalMouseY())});
            selection_ = SelectMouseTile(
                playerFeet, {mouseWorld.x, mouseWorld.y}, ActiveMap());
        } else {
            selection_ = SelectFrontTile(playerFeet, player_.facing, ActiveMap());
        }
    }
    const ToolAction impactAction = player_.toolUse.action;
    const bool impactNow = impactAction != ToolAction::None && !player_.toolUse.applied &&
        player_.toolUse.elapsedTicks + 1U >= ToolImpactTick;
    if (!UpdateToolUse(entityWorld_, player_, ActiveMap())) return false;
    if (impactNow) {
        const bool hoe = impactAction == ToolAction::Hoe;
        audio_.PlayEffect(hoe ? MakeAssetId("audio.hoe") : MakeAssetId("audio.watering"),
            hoe ? 1.7F : 1.9F);
    }
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
    audio_.Shutdown();
    crops_.Clear();
    worldClock_.Reset();
    inventory_.Clear();
    entityWorld_.Clear();
    farmMap_.Clear(); houseMap_.Clear();
    assets_.Clear();
    window_.Shutdown();
    fixedStep_.Reset();
    instructionTicks_ = 600;
    saveNoticeTicks_ = 0;
    gold_ = StartingGold;
    marketOpen_ = false;
    currentMap_ = MapId::Farm;
    goalComplete_ = false;
    inventoryOpen_ = false;
    paused_ = false;
    pauseFocus_ = 0;
    initialized_ = false;
}

bool Application::UpdatePauseMenu() noexcept {
    [[maybe_unused]] const bool ignoredInventory = input_.ConsumePressed(Action::Inventory);
    [[maybe_unused]] const bool ignoredMarket = input_.ConsumePressed(Action::Market);
    const bool up = input_.ConsumePressed(Action::MoveUp);
    const bool down = input_.ConsumePressed(Action::MoveDown);
    const bool left = input_.ConsumePressed(Action::MoveLeft);
    const bool right = input_.ConsumePressed(Action::MoveRight);
    if (up) pauseFocus_ = pauseFocus_ == 0 ? PauseItemCount - 1 : pauseFocus_ - 1;
    if (down) pauseFocus_ = static_cast<std::uint8_t>((pauseFocus_ + 1) % PauseItemCount);
    if (up || down) audio_.PlayEffect(MakeAssetId("audio.ui.move"));

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
    if (activate || direction != 0) audio_.PlayEffect(MakeAssetId("audio.ui.confirm"));
    if (settingsChanged) audio_.SetVolumes(
        settings_.masterVolume, settings_.musicVolume, settings_.effectVolume);
    if (settingsChanged || !paused_) [[maybe_unused]] const bool saved = settingsSystem_.Save(settings_);
    return true;
}

bool Application::ApplyDisplaySettings() noexcept {
    return window_.ApplyDisplaySettings(settings_.windowScale, settings_.fullscreen);
}

bool Application::UpdateMarket() noexcept {
    [[maybe_unused]] const bool ignoredInventory = input_.ConsumePressed(Action::Inventory);
    const bool up=input_.ConsumePressed(Action::MoveUp), down=input_.ConsumePressed(Action::MoveDown);
    marketFocus_=UpdateMarketFocus(marketFocus_,up,down,input_.IsLogicalMouseValid(),
        input_.LogicalMouseX(),input_.LogicalMouseY());
    if(up||down) audio_.PlayEffect(MakeAssetId("audio.ui.move"));
    PhysicalKey interactSource=PhysicalKey::Count,toolSource=PhysicalKey::Count;
    const bool interact=input_.ConsumePressed(Action::Interact,interactSource);
    const bool tool=input_.ConsumePressed(Action::UseTool,toolSource);
    bool activate=interact||tool;
    const bool mouse=interactSource==PhysicalKey::MouseRight||toolSource==PhysicalKey::MouseLeft;
    if(input_.IsLogicalMouseValid()) { const int hit=MarketItemAt(input_.LogicalMouseX(),input_.LogicalMouseY());
        if(hit<0&&mouse) activate=false;
    } else if(mouse) activate=false;
    if(activate) {
        const bool changed=marketFocus_<MarketCropCount?
            BuySeed(inventory_,gold_,marketFocus_):SellHarvest(inventory_,gold_,marketFocus_-MarketCropCount);
        if(changed) {
            audio_.PlayEffect(MakeAssetId("audio.ui.confirm"));
            goalComplete_=gold_>=GoalGold;
            if(goalComplete_) marketOpen_=false;
        }
    }
    input_.DiscardPending();
    return true;
}

bool Application::CaptureSave(SaveSnapshot& snapshot) const noexcept {
    const TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) return false;
    snapshot = {};
    snapshot.playerX256 = static_cast<std::int32_t>(std::lround(transform->current.x * 256.0F));
    snapshot.playerY256 = static_cast<std::int32_t>(std::lround(transform->current.y * 256.0F));
    snapshot.day = worldClock_.Day(); snapshot.minute = worldClock_.Minute();
    snapshot.selectedSlot = static_cast<std::uint8_t>(selectedSlot_);
    snapshot.mapId = currentMap_;
    snapshot.gold = gold_;
    for (std::size_t index = 0; index < Inventory::SlotCount; ++index)
        snapshot.inventory[index] = inventory_.Slot(index);
    for (std::int32_t y = 0; y < farmMap_.Height(); ++y) {
        for (std::int32_t x = 0; x < farmMap_.Width(); ++x) {
            const Tile* tile = farmMap_.Get(x, y);
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
    if (!IsValidMapId(snapshot.mapId)) return false;
    const TileMap& savedMap = snapshot.mapId == MapId::Farm ? farmMap_ : houseMap_;
    const Tile* playerTile = savedMap.Get(
        static_cast<std::int32_t>(playerX / TileSize),
        static_cast<std::int32_t>(playerY / TileSize));
    if (playerX < 0.0F || playerY < 0.0F ||
        playerX >= savedMap.Width() * TileSize || playerY >= savedMap.Height() * TileSize ||
        playerTile == nullptr ||
        (playerTile->flags & (TileFlagValue(TileFlag::Blocked) | TileFlagValue(TileFlag::Water))) != 0 ||
        snapshot.selectedSlot >= Inventory::HotbarSlotCount) return false;
    for (std::size_t index = 0; index < snapshot.tileDeltas.size(); ++index) {
        const SavedTileDelta& delta = snapshot.tileDeltas[index];
        const Tile* tile = farmMap_.Get(delta.x, delta.y);
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
        Tile* tile = farmMap_.Get(delta.x, delta.y); tile->flags |= delta.flags;
    }
    crops_.Clear();
    for (const CropInstance& crop : snapshot.crops) if (!crops_.Restore(crop, farmMap_)) return false;
    TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) return false;
    transform->current = {playerX, playerY}; transform->previous = transform->current;
    currentMap_ = snapshot.mapId;
    selectedSlot_ = snapshot.selectedSlot; inventoryCursor_ = selectedSlot_;
    gold_ = snapshot.gold;
    goalComplete_ = false;
    instructionTicks_ = 0;
    return true;
}

bool Application::SaveGame() noexcept {
    SaveSnapshot snapshot;
    return CaptureSave(snapshot) && saves_.Save(snapshot);
}

TileMap& Application::ActiveMap() noexcept {
    return currentMap_ == MapId::Farm ? farmMap_ : houseMap_;
}

const TileMap& Application::ActiveMap() const noexcept {
    return currentMap_ == MapId::Farm ? farmMap_ : houseMap_;
}

bool Application::ChangeMap(MapId destination) noexcept {
    if (!IsValidMapId(destination) || destination == currentMap_) return false;
    TransformComponent* transform = entityWorld_.Transform(player_.entity);
    if (transform == nullptr) return false;
    currentMap_ = destination;
    const WorldPosition spawn = destination == MapId::House ?
        WorldPosition{10.5F * TileSize, 9.5F * TileSize} :
        WorldPosition{8.5F * TileSize, 10.5F * TileSize};
    transform->current = spawn;
    transform->previous = spawn;
    player_.toolUse = {};
    player_.facing = destination == MapId::House ? FacingDirection::Down : FacingDirection::Up;
    selection_ = {};
    input_.DiscardPending();
    return true;
}

} // namespace Homestead

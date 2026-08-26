#pragma once

#include <Windows.h>

#include <atomic>
#include <cstdint>

#include "Homestead/Save/SaveSystem.hpp"

namespace Homestead {

class AssetStore;
class Audio;
class TileMap;

enum class StartupStage : std::uint8_t {
    Idle,
    Assets,
    Maps,
    Audio,
    Save,
    Ready,
    Failed,
    Cancelled
};

class StartupLoader final {
public:
    StartupLoader() = default;
    ~StartupLoader() noexcept;
    StartupLoader(const StartupLoader&) = delete;
    StartupLoader& operator=(const StartupLoader&) = delete;

    [[nodiscard]] bool Start(const wchar_t* pakPath, AssetStore& assets,
                             TileMap& farmMap, TileMap& houseMap,
                             Audio& audio, SaveSystem& saves) noexcept;
    void Cancel() noexcept;
    void Wait() noexcept;

    [[nodiscard]] StartupStage Stage() const noexcept {
        return stage_.load(std::memory_order_acquire);
    }
    [[nodiscard]] bool IsFinished() const noexcept;
    [[nodiscard]] bool AudioPrepared() const noexcept { return audioPrepared_; }
    [[nodiscard]] SaveLoadResult LoadResult() const noexcept { return loadResult_; }
    [[nodiscard]] const SaveSnapshot& Snapshot() const noexcept { return snapshot_; }

private:
    static unsigned __stdcall ThreadEntry(void* context) noexcept;
    void Run() noexcept;
    [[nodiscard]] bool Cancelled() const noexcept {
        return cancelRequested_.load(std::memory_order_acquire);
    }
    void Finish(StartupStage stage) noexcept {
        stage_.store(stage, std::memory_order_release);
    }

    HANDLE thread_ = nullptr;
    AssetStore* assets_ = nullptr;
    TileMap* farmMap_ = nullptr;
    TileMap* houseMap_ = nullptr;
    Audio* audio_ = nullptr;
    SaveSystem* saves_ = nullptr;
    wchar_t pakPath_[MAX_PATH]{};
    SaveSnapshot snapshot_{};
    SaveLoadResult loadResult_ = SaveLoadResult::NotFound;
    bool audioPrepared_ = false;
    std::atomic<StartupStage> stage_{StartupStage::Idle};
    std::atomic<bool> cancelRequested_{false};
};

} // namespace Homestead

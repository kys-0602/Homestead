#include "Homestead/App/StartupLoader.hpp"

#include <cwchar>
#include <process.h>

#include "Homestead/Assets/AssetStore.hpp"
#include "Homestead/Audio/Audio.hpp"
#include "Homestead/World/TileMap.hpp"

namespace Homestead {

StartupLoader::~StartupLoader() noexcept {
    Cancel();
    Wait();
}

bool StartupLoader::Start(const wchar_t* pakPath, AssetStore& assets,
                          TileMap& farmMap, TileMap& houseMap,
                          Audio& audio, SaveSystem& saves) noexcept {
    if (thread_ != nullptr || pakPath == nullptr || wcslen(pakPath) >= MAX_PATH) return false;
    wcscpy_s(pakPath_, pakPath);
    assets_ = &assets;
    farmMap_ = &farmMap;
    houseMap_ = &houseMap;
    audio_ = &audio;
    saves_ = &saves;
    cancelRequested_.store(false, std::memory_order_release);
    stage_.store(StartupStage::Assets, std::memory_order_release);
    thread_ = reinterpret_cast<HANDLE>(
        _beginthreadex(nullptr, 0, ThreadEntry, this, 0, nullptr));
    if (thread_ == nullptr) {
        Finish(StartupStage::Failed);
        return false;
    }
    return true;
}

void StartupLoader::Cancel() noexcept {
    cancelRequested_.store(true, std::memory_order_release);
}

void StartupLoader::Wait() noexcept {
    if (thread_ == nullptr) return;
    WaitForSingleObject(thread_, INFINITE);
    CloseHandle(thread_);
    thread_ = nullptr;
}

bool StartupLoader::IsFinished() const noexcept {
    const StartupStage stage = Stage();
    return stage == StartupStage::Ready || stage == StartupStage::Failed ||
        stage == StartupStage::Cancelled;
}

unsigned __stdcall StartupLoader::ThreadEntry(void* context) noexcept {
    static_cast<StartupLoader*>(context)->Run();
    return 0;
}

void StartupLoader::Run() noexcept {
    if (Cancelled()) { Finish(StartupStage::Cancelled); return; }
    if (!assets_->LoadFile(pakPath_)) { Finish(StartupStage::Failed); return; }

    Finish(StartupStage::Maps);
    if (Cancelled()) { Finish(StartupStage::Cancelled); return; }
    const MapAsset* farmAsset = assets_->FindMap(MakeAssetId("map/farm"));
    const MapAsset* houseAsset = assets_->FindMap(MakeAssetId("map/house"));
    if (farmAsset == nullptr || houseAsset == nullptr ||
        !farmMap_->LoadMemory(farmAsset->bytes.data(), farmAsset->bytes.size()) ||
        !houseMap_->LoadMemory(houseAsset->bytes.data(), houseAsset->bytes.size())) {
        Finish(StartupStage::Failed);
        return;
    }

    Finish(StartupStage::Audio);
    if (Cancelled()) { Finish(StartupStage::Cancelled); return; }
    audioPrepared_ = audio_->Prepare(*assets_);

    Finish(StartupStage::Save);
    if (Cancelled()) { Finish(StartupStage::Cancelled); return; }
    loadResult_ = saves_->Load(snapshot_);
    Finish(Cancelled() ? StartupStage::Cancelled : StartupStage::Ready);
}

} // namespace Homestead

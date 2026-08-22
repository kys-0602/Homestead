#pragma once

#include <xaudio2.h>

#include <array>
#include <cstdint>
#include <vector>

#include "Homestead/Assets/AssetStore.hpp"

namespace Homestead {

class Audio final {
public:
    Audio() = default;
    ~Audio() noexcept;
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    [[nodiscard]] bool Initialize(const AssetStore& assets) noexcept;
    void SetVolumes(std::uint8_t master, std::uint8_t music, std::uint8_t effects) noexcept;
    [[nodiscard]] bool PlayMusic(AssetId id) noexcept;
    void PlayEffect(AssetId id, float gain = 1.0F) noexcept;
    void Shutdown() noexcept;

private:
    struct Clip { AssetId id = 0; std::vector<std::int16_t> samples; };
    struct EffectVoice { IXAudio2SourceVoice* voice = nullptr; AssetId playing = 0; };
    [[nodiscard]] const Clip* Find(AssetId id) const noexcept;

    IXAudio2* engine_ = nullptr;
    IXAudio2MasteringVoice* mastering_ = nullptr;
    IXAudio2SourceVoice* musicVoice_ = nullptr;
    std::array<EffectVoice, 4> effects_{};
    std::vector<Clip> clips_;
    std::uint8_t masterVolume_ = 10, musicVolume_ = 8, effectVolume_ = 10;
};

} // namespace Homestead

#include "Homestead/Audio/Audio.hpp"
#include "Homestead/Audio/AudioCodec.hpp"

#include <algorithm>

namespace Homestead {
namespace {

WAVEFORMATEX Format() noexcept {
    WAVEFORMATEX format{}; format.wFormatTag=WAVE_FORMAT_PCM; format.nChannels=1;
    format.nSamplesPerSec=8000; format.wBitsPerSample=16; format.nBlockAlign=2;
    format.nAvgBytesPerSec=16000; return format;
}

} // namespace

Audio::~Audio() noexcept { Shutdown(); }

bool Audio::Prepare(const AssetStore& assets) noexcept {
    clips_.clear();
    constexpr AssetId ids[]{MakeAssetId("audio.music.farm"),MakeAssetId("audio.hoe"),
        MakeAssetId("audio.watering"),MakeAssetId("audio.plant"),MakeAssetId("audio.harvest"),
        MakeAssetId("audio.ui.move"),MakeAssetId("audio.ui.confirm")};
    for (AssetId id:ids) { const AudioAsset* asset=assets.FindAudio(id); if (!asset) { clips_.clear(); return false; }
        Clip clip{id,{}}; if (!DecodeAdpcm2(asset->bytes.data(),asset->bytes.size(),clip.samples)) {
            clips_.clear(); return false; } clips_.push_back(std::move(clip)); }
    return true;
}

bool Audio::InitializeOutput() noexcept {
    if (clips_.empty()) return false;
    if (FAILED(XAudio2Create(&engine_,0,XAUDIO2_DEFAULT_PROCESSOR)) ||
        FAILED(engine_->CreateMasteringVoice(&mastering_))) { Shutdown(); return false; }
    WAVEFORMATEX format=Format();
    if (FAILED(engine_->CreateSourceVoice(&musicVoice_,&format,0,XAUDIO2_MAX_FREQ_RATIO))) { Shutdown(); return false; }
    for (EffectVoice& effect:effects_) if (FAILED(engine_->CreateSourceVoice(
        &effect.voice,&format,0,XAUDIO2_MAX_FREQ_RATIO))) { Shutdown(); return false; }
    SetVolumes(masterVolume_,musicVolume_,effectVolume_); return true;
}

void Audio::SetVolumes(std::uint8_t master,std::uint8_t music,std::uint8_t effects) noexcept {
    masterVolume_=std::min<std::uint8_t>(master,10); musicVolume_=std::min<std::uint8_t>(music,10);
    effectVolume_=std::min<std::uint8_t>(effects,10);
    if (mastering_) mastering_->SetVolume(masterVolume_/10.0F);
    if (musicVoice_) musicVoice_->SetVolume(musicVolume_/10.0F);
    for (EffectVoice& effect:effects_) if (effect.voice) effect.voice->SetVolume(effectVolume_/10.0F);
}

bool Audio::PlayMusic(AssetId id) noexcept {
    const Clip* clip=Find(id); if (!clip || !musicVoice_) return false;
    musicVoice_->Stop(); musicVoice_->FlushSourceBuffers();
    XAUDIO2_BUFFER buffer{}; buffer.AudioBytes=static_cast<UINT32>(clip->samples.size()*2U);
    buffer.pAudioData=reinterpret_cast<const BYTE*>(clip->samples.data()); buffer.Flags=XAUDIO2_END_OF_STREAM;
    buffer.LoopCount=XAUDIO2_LOOP_INFINITE;
    return SUCCEEDED(musicVoice_->SubmitSourceBuffer(&buffer)) && SUCCEEDED(musicVoice_->Start());
}

void Audio::PlayEffect(AssetId id, float gain) noexcept {
    const Clip* clip=Find(id); if (!clip) return;
    EffectVoice* available=nullptr;
    for (EffectVoice& effect:effects_) { XAUDIO2_VOICE_STATE state{}; effect.voice->GetState(&state);
        if (state.BuffersQueued==0) { effect.playing=0; if (!available) available=&effect; }
        else if (effect.playing==id) return; }
    if (!available) return;
    available->voice->SetVolume((effectVolume_/10.0F)*std::clamp(gain,0.0F,2.0F));
    XAUDIO2_BUFFER buffer{}; buffer.AudioBytes=static_cast<UINT32>(clip->samples.size()*2U);
    buffer.pAudioData=reinterpret_cast<const BYTE*>(clip->samples.data()); buffer.Flags=XAUDIO2_END_OF_STREAM;
    if (SUCCEEDED(available->voice->SubmitSourceBuffer(&buffer))) { available->playing=id; available->voice->Start(); }
}

const Audio::Clip* Audio::Find(AssetId id) const noexcept {
    for (const Clip& clip:clips_) if (clip.id==id) return &clip; return nullptr;
}

void Audio::Shutdown() noexcept {
    for (EffectVoice& effect:effects_) { if (effect.voice) effect.voice->DestroyVoice(); effect={}; }
    if (musicVoice_) { musicVoice_->DestroyVoice(); musicVoice_=nullptr; }
    if (mastering_) { mastering_->DestroyVoice(); mastering_=nullptr; }
    if (engine_) { engine_->Release(); engine_=nullptr; }
    clips_.clear();
}

} // namespace Homestead

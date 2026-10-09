/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "sound_player.h"

#include "asset_manager.h"
#include "logger.h"
#include "sdl_audio_compat.h"

#include <cstring>

namespace app {
namespace {

constexpr uint32_t kSdlInitAudio = 0x00000010U;
constexpr uint16_t kAudioS16LittleEndian = 0x8010U;
constexpr int kSampleRate = 22050;
constexpr int kOutputChannels = 2;
constexpr int kBufferSamples = 512;
constexpr int kVolume = 26; // Preserve the previous 26/128 cue volume.

constexpr std::array<const char*, static_cast<std::size_t>(SoundCue::Count)> kCueAssets = {
    "audio/uisfx-arcade-focus.wav",
    "audio/uisfx-arcade-select.wav",
    "audio/uisfx-arcade-blocked.wav",
    "audio/uisfx-arcade-long-press.wav",
};

} // namespace

SoundPlayer::SoundPlayer(AssetManager& assets) : assets_(assets) {}

SoundPlayer::~SoundPlayer() {
    shutdown();
}

bool SoundPlayer::initialize() {
    if (available_) return true;

    if (SDL_InitSubSystem(kSdlInitAudio) != 0) {
        LOG_WARN("failed to initialize SDL audio: {}", SDL_GetError());
        return false;
    }
    audio_subsystem_initialized_ = true;

    for (std::size_t index = 0; index < kCueAssets.size(); ++index) {
        const auto path = assets_.resolve(kCueAssets[index]);
        if (path.empty()) {
            LOG_WARN("UI sound asset not found: {}", kCueAssets[index]);
            shutdown();
            return false;
        }

        auto* source = SDL_RWFromFile(path.string().c_str(), "rb");
        if (!source) {
            LOG_WARN("failed to open UI sound {}: {}", path.string(), SDL_GetError());
            shutdown();
            return false;
        }

        auto& cue = cues_[index];
        SDL_AudioSpec spec{};
        if (!SDL_LoadWAV_RW(source, 1, &spec, &cue.data, &cue.length)) {
            LOG_WARN("failed to load UI sound {}: {}", path.string(), SDL_GetError());
            shutdown();
            return false;
        }

        if (spec.freq != kSampleRate || spec.format != kAudioS16LittleEndian ||
            spec.channels != kOutputChannels || cue.length % sizeof(int16_t) != 0) {
            LOG_WARN("UI sound must be 22050 Hz, stereo, signed 16-bit PCM: {}", path.string());
            shutdown();
            return false;
        }
        for (uint32_t offset = 0; offset < cue.length; offset += sizeof(int16_t)) {
            int16_t sample;
            std::memcpy(&sample, cue.data + offset, sizeof(sample));
            sample = static_cast<int16_t>(static_cast<int>(sample) * kVolume / 128);
            std::memcpy(cue.data + offset, &sample, sizeof(sample));
        }
    }

    SDL_AudioSpec desired{};
    desired.freq = kSampleRate;
    desired.format = kAudioS16LittleEndian;
    desired.channels = kOutputChannels;
    desired.samples = kBufferSamples;
    // No callback: SDL copies and plays the queued PCM asynchronously.
    // With allowed_changes=0, SDL converts to the hardware format internally.
    device_ = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);
    if (!device_) {
        LOG_WARN("failed to open SDL audio device: {}", SDL_GetError());
        shutdown();
        return false;
    }
    SDL_PauseAudioDevice(device_, 0);
    available_ = true;
    LOG_INFO("UISFX Arcade sounds ready");
    return true;
}

bool SoundPlayer::available() const {
    return available_;
}

void SoundPlayer::play(SoundCue cue) {
    if (!available_) return;

    const auto index = static_cast<std::size_t>(cue);
    if (index >= cues_.size() || !cues_[index].data) return;

    SDL_ClearQueuedAudio(device_);
    if (SDL_QueueAudio(device_, cues_[index].data, cues_[index].length) < 0) {
        LOG_WARN("failed to play UISFX cue: {}", SDL_GetError());
    }
}

void SoundPlayer::shutdown() {
    if (device_) {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
    for (auto& cue : cues_) {
        if (cue.data) {
            SDL_FreeWAV(cue.data);
            cue = {};
        }
    }
    if (audio_subsystem_initialized_) {
        SDL_QuitSubSystem(kSdlInitAudio);
        audio_subsystem_initialized_ = false;
    }
    available_ = false;
}

} // namespace app

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace app {

class AssetManager;

enum class SoundCue {
    Focus = 0,
    Select,
    Blocked,
    LongPress,
    Count,
};

class SoundPlayer {
public:
    explicit SoundPlayer(AssetManager& assets);
    ~SoundPlayer();

    SoundPlayer(const SoundPlayer&) = delete;
    SoundPlayer& operator=(const SoundPlayer&) = delete;

    bool initialize();
    bool available() const;
    void play(SoundCue cue);

private:
    void shutdown();

    AssetManager& assets_;
    struct Cue {
        uint8_t* data{nullptr};
        uint32_t length{0};
    };
    std::array<Cue, static_cast<std::size_t>(SoundCue::Count)> cues_{};
    bool available_{false};
    bool audio_subsystem_initialized_{false};
    uint32_t device_{0};
};

} // namespace app

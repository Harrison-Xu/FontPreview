/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "asset_manager.h"
#include "preview_model.h"
#include "sound_player.h"

#include <cstdint>
#include <memory>

namespace view {
class PreviewScreen;
}

namespace app {

class KeyboardShortcuts;

class Application {
public:
    Application();
    ~Application();
    int run();

private:
    static void key_event(uint32_t key, bool pressed, void* user_data);
    void handle_key(uint32_t key, bool pressed);

    AssetManager assets_;
    SoundPlayer sound_player_{assets_};
    model::PreviewModel model_;
    std::unique_ptr<view::PreviewScreen> screen_;
    std::unique_ptr<KeyboardShortcuts> shortcuts_;
    bool running_{true};
};

} // namespace app

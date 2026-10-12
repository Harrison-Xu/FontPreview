/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "keyboard_shortcuts.h"
#include "linux_input.h"
#include "preview_screen.h"
#include <utility>

namespace app {

KeyboardShortcuts::KeyboardShortcuts(view::PreviewScreen& screen, std::function<void()> request_quit)
    : screen_(screen), request_quit_(std::move(request_quit)) {}

KeyboardShortcuts::~KeyboardShortcuts() {
    cancel_exit();
}

bool KeyboardShortcuts::handle_key(uint32_t key, bool pressed) {
    if (key == platform::kKeyHelp) {
        if (pressed) {
            cancel_exit();
            screen_.set_help_visible(!screen_.help_visible());
        }
        return true;
    }

    if (screen_.help_visible()) {
        if (key == LV_KEY_ESC && pressed) screen_.set_help_visible(false);
        return true;
    }

    if (key != LV_KEY_ESC) {
        if (pressed) cancel_exit();
        return false;
    }

    if (!pressed) {
        cancel_exit();
    }
    else if (!exit_timer_) {
        screen_.show_exit_hint();
        exit_started_at_ = lv_tick_get();
        // Match CardputerZero/Template: check the 700 ms hold every 40 ms.
        exit_timer_ = lv_timer_create(exit_timer_cb, 40, this);
    }
    return true;
}

void KeyboardShortcuts::cancel_exit() {
    if (exit_timer_) {
        lv_timer_delete(exit_timer_);
        exit_timer_ = nullptr;
    }
    screen_.hide_exit_hint();
}

void KeyboardShortcuts::exit_timer_cb(lv_timer_t* timer) {
    auto* shortcuts = static_cast<KeyboardShortcuts*>(lv_timer_get_user_data(timer));
    if (lv_tick_elaps(shortcuts->exit_started_at_) < 700) return;
    shortcuts->cancel_exit();
    shortcuts->request_quit_();
}

} // namespace app

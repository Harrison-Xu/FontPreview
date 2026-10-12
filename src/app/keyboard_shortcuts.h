/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lvgl.h"
#include <functional>

namespace view { class PreviewScreen; }

namespace app {

class KeyboardShortcuts {
public:
    KeyboardShortcuts(view::PreviewScreen& screen, std::function<void()> request_quit);
    ~KeyboardShortcuts();
    KeyboardShortcuts(const KeyboardShortcuts&) = delete;
    KeyboardShortcuts& operator=(const KeyboardShortcuts&) = delete;

    bool handle_key(uint32_t key, bool pressed);

private:
    void cancel_exit();
    static void exit_timer_cb(lv_timer_t* timer);

    view::PreviewScreen& screen_;
    std::function<void()> request_quit_;
    lv_timer_t* exit_timer_{nullptr};
    uint32_t exit_started_at_{0};
};

} // namespace app

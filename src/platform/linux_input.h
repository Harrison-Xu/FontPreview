#pragma once

#include "lvgl.h"

#include <cstdint>

namespace platform {

// fn+H is emitted as KEY_HELP by the CardputerZero keyboard, as in Template.
constexpr uint32_t kKeyHelp = 0x200000U | 19U;

using KeyEventHandler = void (*)(uint32_t key, bool pressed, void* user_data);

void init_key_input(lv_display_t* display);
void deinit_key_input(lv_display_t* display);
void attach_key_router(lv_indev_t* indev);
void set_key_event_handler(KeyEventHandler handler, void* user_data);

} // namespace platform

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "app.h"

#include "logger.h"
#include "linux_input.h"
#include "preview_screen.h"

#if USE_DESKTOP
#include "src/draw/snapshot/lv_snapshot.h"
#include <png.h>

#include <cstdio>
#include <cstdlib>
#include <vector>
#endif

#if !USE_DESKTOP
#if APP_USE_DRM
#include "src/drivers/display/drm/lv_linux_drm.h"
#else
#include "src/drivers/display/fb/lv_linux_fbdev.h"
#endif
#endif

#ifndef APP_FRAMEBUFFER_DEVICE
#define APP_FRAMEBUFFER_DEVICE "/dev/fb0"
#endif

#ifndef APP_DRM_DEVICE
#define APP_DRM_DEVICE "/dev/dri/card0"
#endif

#ifndef APP_DRM_CONNECTOR_ID
#define APP_DRM_CONNECTOR_ID -1
#endif

namespace app {
namespace {

#if USE_DESKTOP
bool write_snapshot_png(lv_obj_t* object, const char* path) {
    auto* snapshot = lv_snapshot_take(object, LV_COLOR_FORMAT_ARGB8888);
    if (!snapshot) return false;

    FILE* file = std::fopen(path, "wb");
    if (!file) {
        lv_draw_buf_destroy(snapshot);
        return false;
    }

    auto* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    auto* info = png ? png_create_info_struct(png) : nullptr;
    if (!png || !info) {
        if (png) png_destroy_write_struct(&png, nullptr);
        std::fclose(file);
        lv_draw_buf_destroy(snapshot);
        return false;
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        std::fclose(file);
        lv_draw_buf_destroy(snapshot);
        return false;
    }

    png_init_io(png, file);
    png_set_IHDR(png,
                 info,
                 snapshot->header.w,
                 snapshot->header.h,
                 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    std::vector<uint8_t> rgb(static_cast<std::size_t>(snapshot->header.w) * 3U);
    for (uint32_t y = 0; y < snapshot->header.h; ++y) {
        const auto* source = reinterpret_cast<const lv_color32_t*>(
            snapshot->data + static_cast<std::size_t>(y) * snapshot->header.stride);
        for (uint32_t x = 0; x < snapshot->header.w; ++x) {
            rgb[x * 3U] = source[x].red;
            rgb[x * 3U + 1U] = source[x].green;
            rgb[x * 3U + 2U] = source[x].blue;
        }
        png_write_row(png, rgb.data());
    }

    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
    std::fclose(file);
    lv_draw_buf_destroy(snapshot);
    return true;
}
#endif

lv_display_t* init_display() {
#if USE_DESKTOP
    auto* display = lv_sdl_window_create(view::kScreenWidth, view::kScreenHeight);
    if (!display) {
        return nullptr;
    }

    lv_sdl_window_set_title(display, "FontPreview");
    lv_sdl_window_set_resizeable(display, false);
    lv_sdl_mouse_create();
    lv_sdl_mousewheel_create();
    auto* keyboard = lv_sdl_keyboard_create();
    platform::attach_key_router(keyboard);
    return display;
#elif APP_USE_DRM
    auto* display = lv_linux_drm_create();
    if (!display) {
        return nullptr;
    }

    if (lv_linux_drm_set_file(display, APP_DRM_DEVICE, APP_DRM_CONNECTOR_ID) != LV_RESULT_OK) {
        lv_display_delete(display);
        return nullptr;
    }

    platform::init_key_input(display);
    return display;
#else
    auto* display = lv_linux_fbdev_create();
    if (!display) {
        return nullptr;
    }

    if (lv_linux_fbdev_set_file(display, APP_FRAMEBUFFER_DEVICE) != LV_RESULT_OK) {
        lv_display_delete(display);
        return nullptr;
    }

    platform::init_key_input(display);
    return display;
#endif
}

} // namespace

Application::Application() = default;
Application::~Application() = default;

int Application::run() {
    logger::Logger::init();
    logger::Logger::set_tag("fontpreview");

    lv_init();

    auto* display = init_display();
    if (!display) {
        LOG_ERROR("failed to initialize display");
        return 1;
    }

    for (const auto& root : assets_.roots()) {
        LOG_INFO("asset root: {}", root.string());
    }

    sound_player_.initialize();

    screen_ = std::make_unique<view::PreviewScreen>(model_, assets_, sound_player_);
    if (!screen_->build()) {
        LOG_ERROR("failed to build preview screen");
        return 1;
    }

    int result = 0;
    platform::set_key_event_handler(key_event, this);
    lv_screen_load(screen_->root());

#if USE_DESKTOP
    if (const char* capture_path = std::getenv("APP_CAPTURE_PATH")) {
        lv_obj_update_layout(screen_->root());
        if (!write_snapshot_png(screen_->root(), capture_path)) {
            LOG_ERROR("failed to write preview snapshot: {}", capture_path);
            result = 1;
        }
        else {
            LOG_INFO("wrote preview snapshot: {}", capture_path);
        }
        running_ = false;
    }
#endif

    if (running_) {
        LOG_INFO("LVGL app started at {}x{}", lv_display_get_horizontal_resolution(display),
                 lv_display_get_vertical_resolution(display));
    }
    while (running_) {
        lv_timer_handler();
        lv_delay_ms(5);
    }

    platform::set_key_event_handler(nullptr, nullptr);
    lv_display_delete(display);
    screen_.reset();
    return result;
}

void Application::key_event(uint32_t key, bool pressed, void* user_data) {
    auto* application = static_cast<Application*>(user_data);
    if (application) application->handle_key(key, pressed);
}

void Application::handle_key(uint32_t key, bool pressed) {
    if (key == LV_KEY_ESC && pressed) {
        running_ = false;
        return;
    }
    if (screen_) screen_->handle_key(key, pressed);
}

} // namespace app

#include "asset_manager.h"
#include "keyboard_shortcuts.h"
#include "linux_input.h"
#include "preview_screen.h"
#include "sound_player.h"
#include "src/draw/snapshot/lv_snapshot.h"
#include <SDL.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
uint32_t now = 100;
uint32_t tick() { return now; }
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

lv_obj_t* find_label(lv_obj_t* parent, const char* text) {
    if (lv_obj_check_type(parent, &lv_label_class) &&
        std::string(lv_label_get_text(parent)) == text) return parent;
    for (uint32_t i = 0; i < lv_obj_get_child_count(parent); ++i) {
        if (auto* found = find_label(lv_obj_get_child(parent, i), text)) return found;
    }
    return nullptr;
}

void snapshot(lv_obj_t* root, const std::filesystem::path& path) {
    auto* buffer = lv_snapshot_take(root, LV_COLOR_FORMAT_ARGB8888);
    require(buffer != nullptr, "snapshot must render");
    std::ofstream file(path, std::ios::binary);
    file << "P6\n" << buffer->header.w << ' ' << buffer->header.h << "\n255\n";
    for (uint32_t y = 0; y < buffer->header.h; ++y) {
        const auto* row = reinterpret_cast<const lv_color32_t*>(buffer->data + y * buffer->header.stride);
        for (uint32_t x = 0; x < buffer->header.w; ++x) {
            const char rgb[] = {static_cast<char>(row[x].red), static_cast<char>(row[x].green),
                                static_cast<char>(row[x].blue)};
            file.write(rgb, sizeof(rgb));
        }
    }
    lv_draw_buf_destroy(buffer);
    require(file.good(), "snapshot must be saved");
}

struct InputContext {
    app::KeyboardShortcuts& shortcuts;
    view::PreviewScreen& screen;
    int releases{0};
    static void event(uint32_t key, bool pressed, void* user_data) {
        auto& self = *static_cast<InputContext*>(user_data);
        if (!pressed) ++self.releases;
        if (!self.shortcuts.handle_key(key, pressed)) self.screen.handle_key(key, pressed);
    }
};

void send(SDL_Keycode key, bool pressed, bool repeat = false) {
    SDL_Event event{};
    event.type = pressed ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.keysym.sym = key;
    event.key.repeat = repeat;
    require(SDL_PushEvent(&event) == 1, "SDL keyboard event must queue");
    for (auto* indev = lv_indev_get_next(nullptr); indev; indev = lv_indev_get_next(indev)) {
        lv_indev_read(indev);
    }
}

void advance(uint32_t ms) {
    now += ms;
    lv_timer_handler();
}
}

int main(int argc, char** argv) {
    try {
        require(argc >= 2, "external UI asset directory is required");
        require(SDL_Init(SDL_INIT_EVENTS) == 0, "SDL events must initialize");
        lv_init();
        lv_tick_set_cb(tick);
        auto* display = lv_display_create(320, 170);
        std::array<lv_color32_t, 320 * 170> pixels{};
        lv_display_set_color_format(display, LV_COLOR_FORMAT_ARGB8888);
        lv_display_set_buffers(display, pixels.data(), nullptr, sizeof(pixels), LV_DISPLAY_RENDER_MODE_DIRECT);
        lv_display_set_flush_cb(display, [](lv_display_t* d, const lv_area_t*, uint8_t*) {
            lv_display_flush_ready(d);
        });
        app::AssetManager assets("fontpreview-test", argv[1]);
        app::SoundPlayer sounds(assets);
        model::PreviewModel model;
        {
            view::PreviewScreen screen(model, assets, sounds);
            require(screen.build(), "preview screen must build with existing Noto CJK faces");
            lv_screen_load(screen.root());
            bool quit = false;
            app::KeyboardShortcuts shortcuts(screen, [&quit] { quit = true; });
            InputContext input{shortcuts, screen};
            platform::set_key_event_handler(InputContext::event, &input);
            platform::init_key_input(display);

            send(SDLK_ESCAPE, true);
            auto* hint_label = find_label(screen.root(), "Hold #f2c97d ESC# to exit");
            require(hint_label != nullptr, "ESC hint must match Template and highlight ESC");
            auto* hint = lv_obj_get_parent(hint_label);
            require(!lv_obj_has_flag(hint, LV_OBJ_FLAG_HIDDEN), "ESC press must show hint");
            if (argc >= 3) snapshot(screen.root(), std::filesystem::path(argv[2]) / "escape-dark.ppm");
            advance(699);
            require(!quit, "ESC must not exit before 700 ms");
            send(SDLK_ESCAPE, false);
            require(lv_obj_has_flag(hint, LV_OBJ_FLAG_HIDDEN), "ESC release must hide hint like Template");
            advance(80);
            require(!quit, "short ESC must not exit after release");
            send(SDLK_ESCAPE, true);
            advance(720);
            require(quit, "held ESC must exit after 700 ms without OS key-repeat");
            send(SDLK_ESCAPE, false);
            quit = false;

            send(SDLK_h, true);
            send(SDLK_h, false);
            require(screen.help_visible(), "H must open help");
            require(find_label(screen.root(), "FontPreview") != nullptr, "help badge must preserve app name casing");
            for (const char* text : {"F / X", "Switch fields", "Z / C", "Change options", "L / M", "Scroll page content"}) {
                auto* label = find_label(screen.root(), text);
                require(label != nullptr, "help must contain the requested English controls");
                lv_obj_update_layout(label);
                require(lv_obj_get_height(label) <= 24, "help row text must fit on one line");
            }
            if (argc >= 3) snapshot(screen.root(), std::filesystem::path(argv[2]) / "help-dark.ppm");
            send(SDLK_x, true);
            send(SDLK_x, false);
            require(model.selected_field() == model::PreviewField::Font, "help must block field changes");
            send(SDLK_h, true, true);
            require(screen.help_visible(), "OS repeat must not toggle help");
            send(SDLK_ESCAPE, true);
            require(!screen.help_visible(), "ESC must close help");
            advance(800);
            require(!quit, "ESC used to close help must not arm exit");
            send(SDLK_ESCAPE, false);

            send(SDLK_ESCAPE, true);
            send(SDLK_F1, true);
            send(SDLK_F1, false);
            advance(800);
            require(screen.help_visible() && !quit, "opening help must cancel a pending ESC exit");
            send(SDLK_ESCAPE, false);
            send(SDLK_F1, true);
            send(SDLK_F1, false);
            require(!screen.help_visible(), "F1 must toggle help closed");

            for (int i = 0; i < 3; ++i) {
                send(SDLK_x, true);
                send(SDLK_x, false);
            }
            require(model.selected_field() == model::PreviewField::Size, "navigation must resume after help closes");
            const int size = model.font_size();
            send(SDLK_c, true);
            advance(430);
            require(model.font_size() == size + 2, "held C must adjust size repeatedly");
            send(SDLK_h, true);
            advance(800);
            require(model.font_size() == size + 2, "help must stop an active size repeat");
            send(SDLK_c, false);
            send(SDLK_h, false);
            send(SDLK_h, true);
            send(SDLK_h, false);

            // Exercise cache pruning and the light Template palette after using help.
            while (model.selected_field() != model::PreviewField::Color) {
                send(SDLK_x, true);
                send(SDLK_x, false);
            }
            send(SDLK_c, true);
            send(SDLK_c, false);
            send(SDLK_h, true);
            send(SDLK_h, false);
            if (argc >= 3) snapshot(screen.root(), std::filesystem::path(argv[2]) / "help-light.ppm");
            send(SDLK_h, true);
            send(SDLK_h, false);
            send(SDLK_ESCAPE, true);
            require(find_label(screen.root(), "Hold #f0a020 ESC# to exit") != nullptr, "light hint must use Template warning color");
            if (argc >= 3) snapshot(screen.root(), std::filesystem::path(argv[2]) / "escape-light.ppm");
            SDL_Event focus_lost{};
            focus_lost.type = SDL_WINDOWEVENT;
            focus_lost.window.event = SDL_WINDOWEVENT_FOCUS_LOST;
            require(SDL_PushEvent(&focus_lost) == 1, "focus loss must queue");
            for (auto* indev = lv_indev_get_next(nullptr); indev; indev = lv_indev_get_next(indev)) lv_indev_read(indev);
            advance(800);
            require(!quit, "losing desktop focus must cancel ESC hold");
            require(input.releases > 0, "platform must deliver real key releases");
            platform::set_key_event_handler(nullptr, nullptr);
            platform::deinit_key_input(display);
        }
        lv_display_delete(display);
        SDL_Quit();
        std::cout << "ESC hold, help modal, keyboard releases, and size repeat checks passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

#include "asset_manager.h"
#include "preview_model.h"
#include "sound_player.h"
#include <SDL.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

struct TemporaryAssets {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("fontpreview-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    TemporaryAssets() { std::filesystem::create_directories(path / "audio"); }
    ~TemporaryAssets() { std::filesystem::remove_all(path); }
};
}

int main(int argc, char** argv) {
    try {
        require(argc >= 2, "asset directory is required");
        lv_init();
        app::AssetManager assets("fontpreview-test", argv[1]);
        {
            app::SoundPlayer sounds(assets);
            require(sounds.initialize(), "valid PCM cues must initialize on SDL's dummy device");
            require(sounds.initialize(), "repeated initialization must be safe");
            for (int i = 0; i < 100; ++i) {
                sounds.play(static_cast<app::SoundCue>(i % static_cast<int>(app::SoundCue::Count)));
            }
            sounds.play(app::SoundCue::Count);
            require(sounds.available(), "playing cues must leave audio available");
        }
        {
            TemporaryAssets invalid;
            // Fail after two valid cues have loaded, to exercise partial cleanup.
            std::ofstream(invalid.path / "audio/uisfx-arcade-blocked.wav") << "invalid WAV";
            app::AssetManager invalid_assets("fontpreview-test", invalid.path);
            app::SoundPlayer sounds(invalid_assets);
            require(!sounds.initialize(), "invalid WAV must disable cues without crashing");
            require(!sounds.available(), "failed audio initialization must remain unavailable");
            sounds.play(app::SoundCue::Focus);
        }
        {
            TemporaryAssets invalid;
            const auto focus = invalid.path / "audio/uisfx-arcade-focus.wav";
            std::filesystem::copy_file(assets.resolve("audio/uisfx-arcade-focus.wav"), focus);
            std::fstream wav(focus, std::ios::binary | std::ios::in | std::ios::out);
            const char sample_rate_8000[] = {0x40, 0x1f, 0x00, 0x00};
            wav.seekp(24);
            wav.write(sample_rate_8000, sizeof(sample_rate_8000));
            wav.close();
            app::AssetManager invalid_assets("fontpreview-test", invalid.path);
            app::SoundPlayer sounds(invalid_assets);
            require(!sounds.initialize(), "unexpected PCM formats must disable cues safely");
        }
        SDL_setenv("SDL_AUDIODRIVER", "fontpreview-invalid-driver", 1);
        {
            app::SoundPlayer sounds(assets);
            require(!sounds.initialize(), "an unavailable audio device must be nonfatal");
            sounds.play(app::SoundCue::Select);
        }
        SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
        {
            app::SoundPlayer sounds(assets);
            require(sounds.initialize(), "audio resources must be released after failures");
        }

        if (argc >= 3 && argv[2][0] != '\0') {
            auto* ui_font = assets.load_font(argv[2], 12);
            require(ui_font != nullptr, "test font must load");
            auto* display = lv_display_create(320, 170);
            auto* label = lv_label_create(lv_screen_active());
            for (int cycle = 0; cycle < 10; ++cycle) {
                for (int size = 8; size <= 24; ++size) {
                    auto* preview = assets.load_font(argv[2], size);
                    require(preview != nullptr, "preview font must load after cache pruning");
                    lv_obj_set_style_text_font(label, preview, 0);
                    lv_label_set_text(label, "Font cache regression 0123456789");
                    lv_obj_update_layout(label);
                    assets.release_unused_fonts({ui_font, preview});
                    require(assets.load_font(argv[2], 12) == ui_font, "retained UI font must stay cached");
                }
            }
            lv_obj_delete(label);
            lv_display_delete(display);
            assets.release_unused_fonts({});
        }
        else {
            std::cout << "Font cache check skipped: set FONTPREVIEW_TEST_FONT.\n";
        }
        model::PreviewModel model;
        require(model.font_face_index() == 2, "Simplified Chinese must use SC face");
        model.select_next_field();
        model.select_next_value();
        require(model.font_face_index() == 3, "Traditional Chinese must use TC face");
        model.select_next_value();
        require(model.font_face_index() == 0, "Japanese must use JP face");
        model.select_next_value();
        require(model.font_face_index() == 1, "Korean must use KR face");
        std::cout << "Audio, font lifetime, and CJK region checks passed.\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

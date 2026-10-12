/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lvgl.h"
#include "preview_model.h"

#include <array>
#include <cstdint>

namespace app {
class AssetManager;
class SoundPlayer;
}

namespace view {

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 170;
constexpr int kSidebarWidth = 66;

class PreviewScreen {
public:
    PreviewScreen(model::PreviewModel& model, app::AssetManager& assets, app::SoundPlayer& sound_player);
    ~PreviewScreen();

    PreviewScreen(const PreviewScreen&) = delete;
    PreviewScreen& operator=(const PreviewScreen&) = delete;

    bool build();
    void handle_key(uint32_t key, bool pressed);
    lv_obj_t* root() const;
    void set_help_visible(bool visible);
    bool help_visible() const;
    void show_exit_hint();
    void hide_exit_hint();

private:
    struct FieldRow {
        lv_obj_t* container{nullptr};
        lv_obj_t* caption{nullptr};
        lv_obj_t* value{nullptr};
    };

    void refresh();
    void apply_colors();
    void refresh_values();
    void position_sample();
    void scroll_preview(int delta);
    void change_selected_value(int direction, uint32_t key);
    void start_size_repeat(uint32_t key);
    void stop_size_repeat();
    static void size_repeat_cb(lv_timer_t* timer);
    bool load_ui_fonts();
    void build_shortcuts();
    void apply_shortcut_colors();
    void release_unused_fonts(lv_font_t* preview_font = nullptr);
    static void help_backdrop_cb(lv_event_t* event);
    FieldRow create_field_row(std::size_t index, const char* caption);

    model::PreviewModel& model_;
    app::AssetManager& assets_;
    app::SoundPlayer& sound_player_;
    lv_obj_t* root_{nullptr};
    lv_obj_t* sidebar_{nullptr};
    lv_obj_t* preview_panel_{nullptr};
    lv_obj_t* sample_{nullptr};
    std::array<FieldRow, model::PreviewModel::kFieldCount> rows_{};
    lv_font_t* ui_regular_{nullptr};
    lv_font_t* ui_small_{nullptr};
    lv_font_t* ui_bold_{nullptr};
    lv_font_t* ui_message_{nullptr};
    std::array<lv_font_t*, 5> shortcut_fonts_{};
    lv_obj_t* help_{nullptr};
    lv_obj_t* help_panel_{nullptr};
    lv_obj_t* help_title_{nullptr};
    lv_obj_t* help_badge_{nullptr};
    lv_obj_t* help_footer_{nullptr};
    std::array<FieldRow, 3> help_rows_{};
    lv_obj_t* exit_hint_{nullptr};
    lv_obj_t* exit_hint_label_{nullptr};
    lv_timer_t* size_repeat_timer_{nullptr};
    uint32_t repeating_key_{0};
    bool size_repeat_has_fired_{false};
    int preview_scroll_y_{0};
};

} // namespace view

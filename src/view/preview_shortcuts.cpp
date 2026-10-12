/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "preview_screen.h"
#include "asset_manager.h"
#include <algorithm>

namespace view {
namespace {
void make_plain(lv_obj_t* object) {
    lv_obj_remove_style_all(object);
    lv_obj_remove_flag(object, LV_OBJ_FLAG_SCROLLABLE);
}
}

void PreviewScreen::build_shortcuts() {
    // Same geometry, font sizes, and palette as CardputerZero/Template 477fa6d.
    // Keep using the Noto CJK faces supplied by the system image.
    shortcut_fonts_ = {
        assets_.load_font("NotoSansCJK-Bold.ttc", 15, false, 2),
        assets_.load_font("NotoSansCJK-Bold.ttc", 11, false, 2),
        assets_.load_font("NotoSansCJK-Regular.ttc", 12, false, 2),
        ui_bold_,
        assets_.load_font("NotoSansCJK-Bold.ttc", 12, false, 2),
    };
    for (auto& font : shortcut_fonts_) {
        if (!font) font = ui_regular_;
    }

    exit_hint_ = lv_obj_create(root_);
    make_plain(exit_hint_);
    lv_obj_remove_flag(exit_hint_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_radius(exit_hint_, 10, 0);
    lv_obj_set_style_bg_opa(exit_hint_, LV_OPA_90, 0);
    lv_obj_set_style_shadow_width(exit_hint_, 8, 0);
    lv_obj_set_style_shadow_opa(exit_hint_, LV_OPA_10, 0);
    lv_obj_set_style_shadow_color(exit_hint_, lv_color_black(), 0);
    exit_hint_label_ = lv_label_create(exit_hint_);
    lv_label_set_recolor(exit_hint_label_, true);
    lv_label_set_long_mode(exit_hint_label_, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_style_text_font(exit_hint_label_, shortcut_fonts_[4], 0);
    lv_obj_set_style_text_align(exit_hint_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_point_t text_size{};
    lv_text_get_size(&text_size, "Hold ESC to exit", shortcut_fonts_[4], 0, 0, 264, LV_TEXT_FLAG_NONE);
    lv_obj_set_size(exit_hint_, std::clamp<int>(text_size.x + 28, 96, 292),
                    std::max<int>(36, text_size.y + 16));
    lv_obj_set_width(exit_hint_label_, std::max<int>(1, text_size.x));
    lv_obj_align(exit_hint_, LV_ALIGN_BOTTOM_MID, 0, -38);
    hide_exit_hint();

    help_ = lv_obj_create(root_);
    make_plain(help_);
    lv_obj_set_size(help_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(help_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(help_, LV_OPA_50, 0);
    lv_obj_add_flag(help_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(help_, help_backdrop_cb, LV_EVENT_CLICKED, this);

    help_panel_ = lv_obj_create(help_);
    make_plain(help_panel_);
    lv_obj_set_size(help_panel_, 292, 146);
    lv_obj_center(help_panel_);
    lv_obj_set_style_radius(help_panel_, 6, 0);
    lv_obj_set_style_bg_opa(help_panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(help_panel_, 1, 0);
    lv_obj_set_style_shadow_width(help_panel_, 14, 0);
    lv_obj_set_style_shadow_opa(help_panel_, LV_OPA_20, 0);
    lv_obj_set_style_shadow_color(help_panel_, lv_color_black(), 0);
    lv_obj_add_flag(help_panel_, LV_OBJ_FLAG_CLICKABLE);

    help_title_ = lv_label_create(help_panel_);
    lv_label_set_text(help_title_, "Keyboard help");
    lv_obj_set_style_text_font(help_title_, shortcut_fonts_[0], 0);
    lv_obj_align(help_title_, LV_ALIGN_TOP_LEFT, 12, 9);

    help_badge_ = lv_label_create(help_panel_);
    lv_label_set_text(help_badge_, "FontPreview");
    lv_obj_set_style_text_font(help_badge_, shortcut_fonts_[1], 0);
    lv_obj_set_style_radius(help_badge_, 3, 0);
    lv_obj_set_style_pad_hor(help_badge_, 6, 0);
    lv_obj_set_style_pad_ver(help_badge_, 2, 0);
    lv_obj_set_style_bg_opa(help_badge_, LV_OPA_20, 0);
    lv_obj_align(help_badge_, LV_ALIGN_TOP_RIGHT, -10, 7);

    auto* rows = lv_obj_create(help_panel_);
    make_plain(rows);
    lv_obj_set_size(rows, 268, 88);
    lv_obj_align(rows, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rows, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(rows, 1, 0);
    constexpr const char* keys[] = {"F / X", "Z / C", "L / M"};
    constexpr const char* actions[] = {"Switch fields", "Change options", "Scroll page content"};
    for (std::size_t i = 0; i < help_rows_.size(); ++i) {
        auto& row = help_rows_[i];
        row.container = lv_obj_create(rows);
        make_plain(row.container);
        // Noto CJK has a taller line box than Template's Latin UI face.
        lv_obj_set_size(row.container, LV_PCT(100), 24);
        row.caption = lv_label_create(row.container);
        lv_label_set_text(row.caption, keys[i]);
        lv_obj_set_width(row.caption, 82);
        lv_obj_set_style_text_font(row.caption, shortcut_fonts_[1], 0);
        lv_obj_align(row.caption, LV_ALIGN_LEFT_MID, 0, 0);
        row.value = lv_label_create(row.container);
        lv_label_set_text(row.value, actions[i]);
        lv_obj_set_width(row.value, 176);
        lv_obj_set_style_text_font(row.value, shortcut_fonts_[2], 0);
        lv_obj_align(row.value, LV_ALIGN_RIGHT_MID, 0, 0);
    }

    help_footer_ = lv_label_create(help_panel_);
    lv_label_set_text(help_footer_, "FN+H / ESC  Close");
    lv_label_set_long_mode(help_footer_, LV_LABEL_LONG_MODE_CLIP);
    lv_obj_set_style_text_font(help_footer_, shortcut_fonts_[3], 0);
    lv_obj_set_size(help_footer_, LV_SIZE_CONTENT, 12);
    lv_obj_align(help_footer_, LV_ALIGN_BOTTOM_LEFT, 12, -7);
    set_help_visible(false);
}

void PreviewScreen::set_help_visible(bool visible) {
    if (!help_) return;
    stop_size_repeat();
    hide_exit_hint();
    if (visible) {
        apply_shortcut_colors();
        lv_obj_move_foreground(help_);
        lv_obj_remove_flag(help_, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(help_, LV_OBJ_FLAG_HIDDEN);
    }
}

bool PreviewScreen::help_visible() const {
    return help_ && !lv_obj_has_flag(help_, LV_OBJ_FLAG_HIDDEN);
}

void PreviewScreen::show_exit_hint() {
    if (!exit_hint_) return;
    stop_size_repeat();
    apply_shortcut_colors();
    lv_obj_move_foreground(exit_hint_);
    lv_obj_remove_flag(exit_hint_, LV_OBJ_FLAG_HIDDEN);
}

void PreviewScreen::hide_exit_hint() {
    if (exit_hint_) lv_obj_add_flag(exit_hint_, LV_OBJ_FLAG_HIDDEN);
}

void PreviewScreen::apply_shortcut_colors() {
    if (!help_panel_) return;
    const bool dark = lv_color_brightness(lv_obj_get_style_bg_color(root_, LV_PART_MAIN)) < 128;
    const auto text = lv_color_hex(dark ? 0xe6e6e6 : 0x1f2225);
    const auto primary = lv_color_hex(dark ? 0x63e2b7 : 0x18a058);
    lv_obj_set_style_bg_color(help_panel_, lv_color_hex(dark ? 0x18181c : 0xffffff), 0);
    lv_obj_set_style_border_color(help_panel_, lv_color_hex(dark ? 0x3d3d3d : 0xe0e0e6), 0);
    lv_obj_set_style_text_color(help_title_, text, 0);
    lv_obj_set_style_bg_color(help_badge_, primary, 0);
    lv_obj_set_style_text_color(help_badge_, primary, 0);
    lv_obj_set_style_text_color(help_footer_, lv_color_hex(dark ? 0x616161 : 0xc2c2c2), 0);
    for (auto& row : help_rows_) {
        lv_obj_set_style_text_color(row.caption, primary, 0);
        lv_obj_set_style_text_color(row.value, text, 0);
    }
    lv_obj_set_style_bg_color(exit_hint_, lv_color_hex(dark ? 0x141414 : 0xf5f5f5), 0);
    lv_obj_set_style_text_color(exit_hint_label_, text, 0);
    lv_label_set_text(exit_hint_label_, dark ? "Hold #f2c97d ESC# to exit" : "Hold #f0a020 ESC# to exit");
    lv_obj_update_layout(exit_hint_label_);
    lv_obj_center(exit_hint_label_);
}

void PreviewScreen::help_backdrop_cb(lv_event_t* event) {
    auto* screen = static_cast<PreviewScreen*>(lv_event_get_user_data(event));
    if (lv_event_get_target(event) == screen->help_) screen->set_help_visible(false);
}

} // namespace view

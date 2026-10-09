/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "preview_screen.h"

#include "asset_manager.h"
#include "logger.h"
#include "sound_player.h"

#include <algorithm>
#include <array>

namespace view {
namespace {

constexpr int kRowTop = 1;
constexpr int kRowHeight = 28;
constexpr int kRowGap = 0;
constexpr int kPreviewInsetX = 8;
constexpr int kPreviewInsetY = 7;
constexpr int kPreviewScrollStep = 24;
constexpr uint32_t kSizeRepeatDelayMs = 420;
constexpr uint32_t kSizeRepeatPeriodMs = 70;

struct ColorScheme {
    uint32_t background;
    uint32_t foreground;
    uint32_t muted;
};

constexpr std::array<ColorScheme, 8> kColorSchemes = {{
    {0x000000, 0xffffff, 0xb8b8b8}, // Black/White
    {0xffffff, 0x111111, 0x666666}, // White/Black
    {0xf4ecd8, 0x3a2e24, 0x8c6f56}, // Sepia
    {0x0b0f0c, 0x7cfc8a, 0x3e8c4a}, // Terminal
    {0x002b36, 0x93a1a1, 0x586e75}, // Solarized
    {0x0b1f33, 0xe6f1ff, 0x7aa2c7}, // Navy
    {0x1b1407, 0xffc857, 0xb58a3a}, // Amber
    {0xe7edf3, 0x17202a, 0x667788}, // Slate
}};

void make_plain(lv_obj_t* object) {
    lv_obj_remove_style_all(object);
    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
}

bool is_previous_value_key(uint32_t key) {
    return key == 'z' || key == 'Z' || key == LV_KEY_LEFT;
}

bool is_next_value_key(uint32_t key) {
    return key == 'c' || key == 'C' || key == LV_KEY_RIGHT;
}

} // namespace

PreviewScreen::PreviewScreen(model::PreviewModel& model,
                             app::AssetManager& assets,
                             app::SoundPlayer& sound_player)
    : model_(model), assets_(assets), sound_player_(sound_player) {}

PreviewScreen::~PreviewScreen() {
    stop_size_repeat();
    if (root_ && lv_obj_is_valid(root_)) {
        lv_obj_delete(root_);
    }
}

bool PreviewScreen::build() {
    if (root_) return true;
    if (!load_ui_fonts()) return false;

    root_ = lv_obj_create(nullptr);
    make_plain(root_);
    lv_obj_set_size(root_, kScreenWidth, kScreenHeight);

    sidebar_ = lv_obj_create(root_);
    make_plain(sidebar_);
    lv_obj_set_pos(sidebar_, 0, 0);
    lv_obj_set_size(sidebar_, kSidebarWidth, kScreenHeight);

    preview_panel_ = lv_obj_create(root_);
    make_plain(preview_panel_);
    lv_obj_set_pos(preview_panel_, kSidebarWidth, 0);
    lv_obj_set_size(preview_panel_, kScreenWidth - kSidebarWidth, kScreenHeight);
    lv_obj_set_style_border_width(preview_panel_, 1, 0);
    lv_obj_set_style_border_side(preview_panel_, LV_BORDER_SIDE_LEFT, 0);

    constexpr std::array<const char*, model::PreviewModel::kFieldCount> kCaptions = {
        "FONT", "LANGUAGE", "TYPEFACE", "SIZE", "WEIGHT", "COLOR"
    };
    for (std::size_t i = 0; i < rows_.size(); ++i) {
        rows_[i] = create_field_row(i, kCaptions[i]);
    }

    sample_ = lv_label_create(preview_panel_);
    make_plain(sample_);
    lv_label_set_long_mode(sample_, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_pos(sample_, kPreviewInsetX, kPreviewInsetY);
    lv_obj_set_width(sample_, kScreenWidth - kSidebarWidth - kPreviewInsetX * 2);
    lv_obj_set_height(sample_, LV_SIZE_CONTENT);
    lv_obj_set_style_text_align(sample_, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_line_space(sample_, 4, 0);

    refresh();
    return true;
}

void PreviewScreen::handle_key(uint32_t key, bool pressed) {
    if (!pressed) {
        if (key == repeating_key_) stop_size_repeat();
        return;
    }

    switch (key) {
        case 'f':
        case 'F':
        case LV_KEY_UP:
            stop_size_repeat();
            model_.select_previous_field();
            sound_player_.play(app::SoundCue::Focus);
            break;
        case 'x':
        case 'X':
        case LV_KEY_DOWN:
            stop_size_repeat();
            model_.select_next_field();
            sound_player_.play(app::SoundCue::Focus);
            break;
        case 'z':
        case 'Z':
        case LV_KEY_LEFT:
            change_selected_value(-1, key);
            return;
        case 'c':
        case 'C':
        case LV_KEY_RIGHT:
            change_selected_value(1, key);
            return;
        case 'l':
        case 'L':
            scroll_preview(-kPreviewScrollStep);
            return;
        case 'm':
        case 'M':
            scroll_preview(kPreviewScrollStep);
            return;
        default:
            return;
    }

    refresh();
}

lv_obj_t* PreviewScreen::root() const {
    return root_;
}

void PreviewScreen::refresh() {
    refresh_values();
    apply_colors();

    if (!model_.has_font_face()) {
        lv_obj_set_style_text_font(sample_, ui_message_, 0);
        lv_label_set_text_fmt(sample_, "%s has no %s %s face.", model_.font_name(), model_.weight_name(),
                              model_.typeface_name());
        position_sample();
        assets_.release_unused_fonts({ui_small_, ui_regular_, ui_bold_, ui_message_});
        return;
    }

    auto* preview_font = assets_.load_font(
        model_.font_file_name(), model_.font_size(), model_.uses_synthetic_italic(), model_.font_face_index());
    if (!preview_font) {
        LOG_ERROR("failed to load preview font: {} at {}px", model_.font_file_name(), model_.font_size());
        lv_obj_set_style_text_font(sample_, ui_message_, 0);
        lv_label_set_text(sample_, "Font file missing.\nInstall the required Debian font package.");
        position_sample();
        assets_.release_unused_fonts({ui_small_, ui_regular_, ui_bold_, ui_message_});
        return;
    }

    lv_obj_set_style_text_font(sample_, preview_font, 0);
    lv_label_set_text(sample_, model_.sample_text());
    position_sample();
    assets_.release_unused_fonts({ui_small_, ui_regular_, ui_bold_, ui_message_, preview_font});
}

void PreviewScreen::apply_colors() {
    const auto& scheme = kColorSchemes[static_cast<std::size_t>(model_.color())];
    const auto background = lv_color_hex(scheme.background);
    const auto foreground = lv_color_hex(scheme.foreground);
    const auto muted = lv_color_hex(scheme.muted);

    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(root_, background, 0);
    lv_obj_set_style_bg_opa(sidebar_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(sidebar_, background, 0);
    lv_obj_set_style_bg_opa(preview_panel_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(preview_panel_, background, 0);
    lv_obj_set_style_border_color(preview_panel_, muted, 0);
    lv_obj_set_style_text_color(sample_, foreground, 0);

    const auto selected = static_cast<std::size_t>(model_.selected_field());
    for (std::size_t i = 0; i < rows_.size(); ++i) {
        const bool is_selected = i == selected;
        lv_obj_set_style_bg_opa(rows_[i].container, is_selected ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_color(rows_[i].container, foreground, 0);
        lv_obj_set_style_text_color(rows_[i].caption, is_selected ? background : muted, 0);
        lv_obj_set_style_text_color(rows_[i].value, is_selected ? background : foreground, 0);
    }
}

void PreviewScreen::refresh_values() {
    lv_label_set_text(rows_[0].value, model_.font_name());
    lv_label_set_text(rows_[1].value, model_.language_name());
    lv_label_set_text(rows_[2].value, model_.typeface_name());
    lv_label_set_text_fmt(rows_[3].value, "%d px", model_.font_size());
    lv_label_set_text(rows_[4].value, model_.weight_name());
    lv_label_set_text(rows_[5].value, model_.color_name());
}

void PreviewScreen::position_sample() {
    lv_obj_update_layout(sample_);
    const int viewport_height = kScreenHeight - kPreviewInsetY * 2;
    const int maximum_scroll = std::max(0, static_cast<int>(lv_obj_get_height(sample_)) - viewport_height);
    preview_scroll_y_ = std::clamp(preview_scroll_y_, 0, maximum_scroll);
    lv_obj_set_pos(sample_, kPreviewInsetX, kPreviewInsetY - preview_scroll_y_);
}

void PreviewScreen::scroll_preview(int delta) {
    preview_scroll_y_ += delta;
    position_sample();
}

void PreviewScreen::change_selected_value(int direction, uint32_t key) {
    const auto field = model_.selected_field();
    const bool changed = direction < 0 ? model_.select_previous_value() : model_.select_next_value();
    preview_scroll_y_ = 0;

    if (!changed) {
        sound_player_.play(app::SoundCue::Blocked);
    }
    else if ((field == model::PreviewField::Font || field == model::PreviewField::Typeface) &&
             !model_.has_font_face()) {
        sound_player_.play(app::SoundCue::Blocked);
    }
    else {
        sound_player_.play(app::SoundCue::Select);
    }

    if (field == model::PreviewField::Size && changed) start_size_repeat(key);
    refresh();
}

void PreviewScreen::start_size_repeat(uint32_t key) {
    stop_size_repeat();
    repeating_key_ = key;
    size_repeat_has_fired_ = false;
    size_repeat_timer_ = lv_timer_create(size_repeat_cb, kSizeRepeatDelayMs, this);
}

void PreviewScreen::stop_size_repeat() {
    if (size_repeat_timer_) {
        lv_timer_delete(size_repeat_timer_);
        size_repeat_timer_ = nullptr;
    }
    repeating_key_ = 0;
    size_repeat_has_fired_ = false;
}

void PreviewScreen::size_repeat_cb(lv_timer_t* timer) {
    auto* screen = static_cast<PreviewScreen*>(lv_timer_get_user_data(timer));
    if (!screen || screen->model_.selected_field() != model::PreviewField::Size) {
        if (screen) screen->stop_size_repeat();
        return;
    }

    bool changed = false;
    if (is_previous_value_key(screen->repeating_key_)) {
        changed = screen->model_.select_previous_value();
    }
    else if (is_next_value_key(screen->repeating_key_)) {
        changed = screen->model_.select_next_value();
    }
    else {
        screen->stop_size_repeat();
        return;
    }

    if (!changed) {
        screen->sound_player_.play(app::SoundCue::Blocked);
        screen->stop_size_repeat();
        return;
    }
    if (!screen->size_repeat_has_fired_) {
        screen->size_repeat_has_fired_ = true;
        screen->sound_player_.play(app::SoundCue::LongPress);
    }

    screen->preview_scroll_y_ = 0;
    screen->refresh();
    if (screen->size_repeat_timer_) {
        lv_timer_set_period(screen->size_repeat_timer_, kSizeRepeatPeriodMs);
    }
}

bool PreviewScreen::load_ui_fonts() {
    constexpr uint32_t kUiFaceIndex = 2; // Noto Sans CJK SC
    ui_small_ = assets_.load_font("NotoSansCJK-Regular.ttc", 8, false, kUiFaceIndex);
    ui_regular_ = assets_.load_font("NotoSansCJK-Regular.ttc", 9, false, kUiFaceIndex);
    ui_bold_ = assets_.load_font("NotoSansCJK-Bold.ttc", 10, false, kUiFaceIndex);
    ui_message_ = assets_.load_font("NotoSansCJK-Regular.ttc", 16, false, kUiFaceIndex);
    if (!ui_small_ || !ui_regular_ || !ui_bold_ || !ui_message_) {
        LOG_ERROR("Noto CJK UI fonts are missing or unreadable");
        return false;
    }
    return true;
}

PreviewScreen::FieldRow PreviewScreen::create_field_row(std::size_t index, const char* caption) {
    FieldRow row;
    row.container = lv_obj_create(sidebar_);
    make_plain(row.container);
    lv_obj_set_pos(row.container, 1, kRowTop + static_cast<int>(index) * (kRowHeight + kRowGap));
    lv_obj_set_size(row.container, kSidebarWidth - 2, kRowHeight - 2);
    lv_obj_set_style_radius(row.container, 2, 0);

    row.caption = lv_label_create(row.container);
    make_plain(row.caption);
    lv_label_set_text(row.caption, caption);
    lv_obj_set_style_text_font(row.caption, ui_small_, 0);
    lv_obj_set_pos(row.caption, 2, 1);

    row.value = lv_label_create(row.container);
    make_plain(row.value);
    lv_label_set_long_mode(row.value, LV_LABEL_LONG_MODE_CLIP);
    lv_obj_set_width(row.value, kSidebarWidth - 4);
    lv_obj_set_style_text_font(row.value, ui_regular_, 0);
    lv_obj_set_pos(row.value, 2, 10);
    return row;
}

} // namespace view

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 * SPDX-License-Identifier: MIT
 */

#include "preview_model.h"

#include <algorithm>
#include <array>

namespace model {
namespace {

struct LanguageSpec {
    const char* name;
    const char* sample;
};

constexpr std::array<LanguageSpec, 10> kLanguages = {{
    {"Chinese (SC)",
     "清晨六点半，城市还没有完全醒来。\n"
     "路口的面包店已经亮起暖黄色的灯，第一班公交车缓缓驶过，车窗里映着匆忙整理围巾的人。\n"
     "有人低头查看消息，也有人捧着热豆浆，站在风里等朋友。\n"
     "屏幕上的文字有长有短，标点、数字与不同结构交错出现，正好可以观察字距、行距和换行是否自然。\n"
     "今天是周三，气温 18°C；下午可能有雨，别忘了带伞。"},
    {"Chinese (TC)",
     "清晨六點半，城市還沒有完全醒來。\n"
     "路口的麵包店已經亮起暖黃色的燈，第一班公車緩緩駛過，車窗裡映著匆忙整理圍巾的人。\n"
     "有人低頭查看訊息，也有人捧著熱豆漿，站在風裡等朋友。\n"
     "螢幕上的文字有長有短，標點、數字與不同結構交錯出現，正好可以觀察字距、行距和換行是否自然。\n"
     "今天是週三，氣溫 18°C；下午可能有雨，別忘了帶傘。"},
    {"Japanese",
     "朝の駅で、カフェのシャッターがゆっくり開きました。\n"
     "ショーケースにはサンドイッチ、コーヒー、ヨーグルトが並びます。\n"
     "発車ベルが鳴るたび、人の流れは少しずつ変わります。\n"
     "新しいスマートフォンでニュースを読み、メモを書き、メールを送る。\n"
     "ひらがな、カタカナ、漢字、数字の 2026、そして「！」や「？」を一緒に表示して、文字の形と間隔を確かめます。"},
    {"Korean",
     "아침 일곱 시, 골목의 작은 카페가 문을 열었습니다.\n"
     "따뜻한 커피 향이 퍼지는 동안 버스는 천천히 정류장에 도착했고, 사람들은 휴대전화로 오늘의 일정과 날씨를 확인했습니다.\n"
     "짧은 문장도 있고, 화면 끝까지 이어지는 긴 문장도 있습니다.\n"
     "한글의 자음과 모음, 숫자 2026, 괄호와 쉼표를 함께 표시해 글자 간격과 줄바꿈이 자연스러운지 살펴봅니다.\n"
     "키스의 고유 조건은 입술끼리 만나야 하고 특별한 기술은 필요치 않다."},
    {"English",
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n"
     "The quick brown fox jumps over the lazy dog.\nThe five boxing wizards jump quickly.\n"
     "Sphinx of black quartz, judge my vow."},
    {"Portuguese",
     "Á À Â Ã Ç É Ê Í Ó Ò Ô Õ Ú\ná à â ã ç é ê í ó ò ô õ ú\n"
     "Gazeta publica hoje breve nota de faxina na quermesse.\n"
     "Vejo galã sexy pôr quinze kiwis à força em baú achatado."},
    {"Czech",
     "Á Č Ď É Ě Í Ň Ó Ř Š Ť Ú Ů Ý Ž\ná č ď é ě í ň ó ř š ť ú ů ý ž\n"
     "Nechť již hříšné saxofony ďáblů rozezvučí síň úděsnými tóny waltzu, tanga a quickstepu.\n"
     "Příliš žluťoučký kůň úpěl ďábelské ódy."},
    {"Greek",
     "Α Β Γ Δ Ε Ζ Η Θ Ι Κ Λ Μ Ν Ξ Ο Π Ρ Σ Τ Υ Φ Χ Ψ Ω\n"
     "ά έ ή ί ΐ ό ύ ΰ ώ ς Ϊ Ϋ\n"
     "Γαζίες και μυρτιές δεν θα βρω πια στο χρυσαφί ξέφωτο.\n"
     "Ξεσκεπάζω την ψυχοφθόρα βδελυγμία."},
    {"Russian",
     "А Б В Г Д Е Ё Ж З И Й К Л М Н О П Р С Т У Ф Х Ц Ч Ш Щ Ъ Ы Ь Э Ю Я\n"
     "а б в г д е ё ж з и й к л м н о п р с т у ф х ц ч ш щ ъ ы ь э ю я\n"
     "Съешь ещё этих мягких французских булок, да выпей же чаю.\n"
     "В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!"},
    {"Code",
     R"CODE($ echo 0123456789
0x00 0x7F 0xDEADBEEF
I l | 1 i !
O 0 o Q D  S 5 s
`~!@#$%^&*()_+-=[]{}\
/\|`'".,;:?  == != <= >=

$ git status --short
 M src/main.cpp

#include <cstdint>
#include <string>

int main(int argc, char** argv) {
    constexpr auto port = 8080;
    std::string url = "https://127.0.0.1/api?q=font";
    if (argc > 1 && argv[1] != nullptr) {
        printf("[%02d] %s\n", port, url.c_str());
    }
    return 0;
}

if (ready && count >= 42) {
    value = index < count ? data[index++] : 0;
})CODE"},
}};

template <typename Enum>
Enum wrap_enum(Enum value, int direction, std::size_t count) {
    const auto current = static_cast<int>(value);
    const auto length = static_cast<int>(count);
    return static_cast<Enum>((current + direction + length) % length);
}

} // namespace

PreviewField PreviewModel::selected_field() const {
    return selected_field_;
}

PreviewFont PreviewModel::font() const {
    return font_;
}

PreviewLanguage PreviewModel::language() const {
    return language_;
}

PreviewTypeface PreviewModel::typeface() const {
    return typeface_;
}

PreviewWeight PreviewModel::weight() const {
    return weight_;
}

PreviewColor PreviewModel::color() const {
    return color_;
}

int PreviewModel::font_size() const {
    return font_size_;
}

void PreviewModel::select_previous_field() {
    selected_field_ = wrap_enum(selected_field_, -1, kFieldCount);
}

void PreviewModel::select_next_field() {
    selected_field_ = wrap_enum(selected_field_, 1, kFieldCount);
}

bool PreviewModel::select_previous_value() {
    return change_value(-1);
}

bool PreviewModel::select_next_value() {
    return change_value(1);
}

const char* PreviewModel::font_name() const {
    constexpr std::array<const char*, 5> kNames = {
        "Noto CJK", "Go", "Inter", "DejaVu", "JetBrains Mono"
    };
    return kNames[static_cast<std::size_t>(font_)];
}

const char* PreviewModel::language_name() const {
    return kLanguages[static_cast<std::size_t>(language_)].name;
}

const char* PreviewModel::typeface_name() const {
    switch (typeface_) {
        case PreviewTypeface::Sans:
            return "Sans";
        case PreviewTypeface::SansItalic:
            return "Sans Italic";
        case PreviewTypeface::Serif:
            return "Serif";
        case PreviewTypeface::SerifItalic:
            return "Serif Italic";
        case PreviewTypeface::Mono:
            return "Mono";
        case PreviewTypeface::MonoItalic:
            return "Mono Italic";
    }
    return "Sans";
}

const char* PreviewModel::weight_name() const {
    switch (weight_) {
        case PreviewWeight::Light:
            return "Light";
        case PreviewWeight::Regular:
            return "Regular";
        case PreviewWeight::Bold:
            return "Bold";
    }
    return "Regular";
}

const char* PreviewModel::color_name() const {
    constexpr std::array<const char*, 8> kNames = {
        "Black/White", "White/Black", "Sepia", "Terminal", "Solarized", "Navy", "Amber", "Slate"
    };
    return kNames[static_cast<std::size_t>(color_)];
}

const char* PreviewModel::sample_text() const {
    return kLanguages[static_cast<std::size_t>(language_)].sample;
}

const char* PreviewModel::font_file_name() const {
    const bool light = weight_ == PreviewWeight::Light;
    const bool bold = weight_ == PreviewWeight::Bold;
    const bool italic = typeface_ == PreviewTypeface::SansItalic ||
                        typeface_ == PreviewTypeface::SerifItalic ||
                        typeface_ == PreviewTypeface::MonoItalic;

    switch (font_) {
        case PreviewFont::NotoCjk:
            switch (typeface_) {
                case PreviewTypeface::Sans:
                case PreviewTypeface::SansItalic:
                    if (light) return "NotoSansCJK-Light.ttc";
                    return bold ? "NotoSansCJK-Bold.ttc" : "NotoSansCJK-Regular.ttc";
                case PreviewTypeface::Serif:
                case PreviewTypeface::SerifItalic:
                    if (light) return "NotoSerifCJK-Light.ttc";
                    return bold ? "NotoSerifCJK-Bold.ttc" : "NotoSerifCJK-Regular.ttc";
                case PreviewTypeface::Mono:
                case PreviewTypeface::MonoItalic:
                    // The Light collection has only five proportional faces.
                    if (light) return "";
                    return bold ? "NotoSansCJK-Bold.ttc" : "NotoSansCJK-Regular.ttc";
            }
            break;
        case PreviewFont::Go:
            if (light) return "";
            if (typeface_ == PreviewTypeface::Sans || typeface_ == PreviewTypeface::SansItalic) {
                if (bold && italic) return "/usr/share/fonts/fonts-go/Go-Bold-Italic.ttf";
                if (bold) return "/usr/share/fonts/fonts-go/Go-Bold.ttf";
                if (italic) return "/usr/share/fonts/fonts-go/Go-Italic.ttf";
                return "/usr/share/fonts/fonts-go/Go-Regular.ttf";
            }
            if (typeface_ == PreviewTypeface::Mono || typeface_ == PreviewTypeface::MonoItalic) {
                if (bold && italic) return "/usr/share/fonts/fonts-go/Go-Mono-Bold-Italic.ttf";
                if (bold) return "/usr/share/fonts/fonts-go/Go-Mono-Bold.ttf";
                if (italic) return "/usr/share/fonts/fonts-go/Go-Mono-Italic.ttf";
                return "/usr/share/fonts/fonts-go/Go-Mono.ttf";
            }
            return "";
        case PreviewFont::Inter:
            if (typeface_ != PreviewTypeface::Sans && typeface_ != PreviewTypeface::SansItalic) return "";
            if (light && italic) return "/usr/share/fonts/opentype/inter/Inter-LightItalic.otf";
            if (light) return "/usr/share/fonts/opentype/inter/Inter-Light.otf";
            if (bold && italic) return "/usr/share/fonts/opentype/inter/Inter-BoldItalic.otf";
            if (bold) return "/usr/share/fonts/opentype/inter/Inter-Bold.otf";
            if (italic) return "/usr/share/fonts/opentype/inter/Inter-Italic.otf";
            return "/usr/share/fonts/opentype/inter/Inter-Regular.otf";
        case PreviewFont::DejaVu:
            if (light) {
                return typeface_ == PreviewTypeface::Sans
                    ? "/usr/share/fonts/truetype/dejavu/DejaVuSans-ExtraLight.ttf" : "";
            }
            switch (typeface_) {
                case PreviewTypeface::Sans:
                case PreviewTypeface::SansItalic:
                    if (bold && italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSans-BoldOblique.ttf";
                    if (bold) return "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
                    if (italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSans-Oblique.ttf";
                    return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
                case PreviewTypeface::Serif:
                case PreviewTypeface::SerifItalic:
                    if (bold && italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSerif-BoldItalic.ttf";
                    if (bold) return "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf";
                    if (italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Italic.ttf";
                    return "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf";
                case PreviewTypeface::Mono:
                case PreviewTypeface::MonoItalic:
                    if (bold && italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-BoldOblique.ttf";
                    if (bold) return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf";
                    if (italic) return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Oblique.ttf";
                    return "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
            }
            break;
        case PreviewFont::JetBrainsMono:
            if (typeface_ != PreviewTypeface::Mono && typeface_ != PreviewTypeface::MonoItalic) return "";
            if (light && italic) {
                return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-LightItalic.ttf";
            }
            if (light) return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Light.ttf";
            if (bold && italic) {
                return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-BoldItalic.ttf";
            }
            if (bold) return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Bold.ttf";
            if (italic) return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Italic.ttf";
            return "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf";
    }
    return "";
}

uint32_t PreviewModel::font_face_index() const {
    if (font_ != PreviewFont::NotoCjk) return 0;

    uint32_t regional_face = 2; // SC is the neutral default for non-CJK samples in this application.
    switch (language_) {
        case PreviewLanguage::SimplifiedChinese:
            regional_face = 2;
            break;
        case PreviewLanguage::TraditionalChinese:
            regional_face = 3;
            break;
        case PreviewLanguage::Japanese:
            regional_face = 0;
            break;
        case PreviewLanguage::Korean:
            regional_face = 1;
            break;
        default:
            break;
    }

    if (typeface_ == PreviewTypeface::Mono || typeface_ == PreviewTypeface::MonoItalic) {
        return regional_face + 5;
    }
    return regional_face;
}

bool PreviewModel::has_font_face() const {
    return font_file_name()[0] != '\0';
}

bool PreviewModel::uses_synthetic_italic() const {
    if (font_ != PreviewFont::NotoCjk) return false;
    return typeface_ == PreviewTypeface::SansItalic || typeface_ == PreviewTypeface::SerifItalic ||
           typeface_ == PreviewTypeface::MonoItalic;
}

bool PreviewModel::change_value(int direction) {
    switch (selected_field_) {
        case PreviewField::Font:
            font_ = wrap_enum(font_, direction, 5);
            return true;
        case PreviewField::Language:
            language_ = wrap_enum(language_, direction, kLanguages.size());
            return true;
        case PreviewField::Typeface:
            typeface_ = wrap_enum(typeface_, direction, 6);
            return true;
        case PreviewField::Size: {
            const auto previous_size = font_size_;
            font_size_ = std::clamp(font_size_ + direction, kMinimumFontSize, kMaximumFontSize);
            return previous_size != font_size_;
        }
        case PreviewField::Weight:
            weight_ = wrap_enum(weight_, direction, 3);
            return true;
        case PreviewField::Color:
            color_ = wrap_enum(color_, direction, 8);
            return true;
    }
    return false;
}

} // namespace model

#include "asset_manager.h"
#include "preview_model.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

struct TemporaryFonts {
    std::filesystem::path root = std::filesystem::temp_directory_path() /
        ("fontpreview-fonts-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    TemporaryFonts() {
        std::filesystem::create_directories(root / "standard");
        std::filesystem::create_directories(root / "launcher");
        std::filesystem::create_directories(root / "factory");
        root = std::filesystem::canonical(root);
    }
    ~TemporaryFonts() { std::filesystem::remove_all(root); }
    void touch(const std::filesystem::path& path) { std::ofstream(root / path) << "fixture"; }
};

void set_field(model::PreviewModel& model, model::PreviewField field) {
    while (model.selected_field() != field) model.select_next_field();
}

void check_resolution() {
    TemporaryFonts fixture;
    fixture.touch("launcher/JetBrainsMono-Regular.ttf");
    fixture.touch("launcher/JetBrainsMono-Bold.ttf");
    fixture.touch("factory/inter-regular.ttf");
    fixture.touch("factory/inter-bold.ttf");
    fixture.touch("standard/NotoSansCJK-Light.ttc");
    app::AssetManager assets("fontpreview-test", {},
        {fixture.root / "standard", fixture.root / "launcher", fixture.root / "factory"});
    const auto missing = fixture.root / "missing";
    require(assets.resolve_font(missing / "JetBrainsMono-Regular.ttf") == fixture.root / "launcher/JetBrainsMono-Regular.ttf",
            "missing Debian JetBrains path must resolve to Launcher font");
    require(assets.resolve_font(missing / "JetBrainsMono-Bold.ttf") == fixture.root / "launcher/JetBrainsMono-Bold.ttf",
            "JetBrains Bold must preserve its weight");
    require(assets.resolve_font(missing / "Inter-Regular.otf") == fixture.root / "factory/inter-regular.ttf",
            "Inter OTF request must resolve to factory Regular TTF");
    require(assets.resolve_font(missing / "Inter-Bold.otf") == fixture.root / "factory/inter-bold.ttf",
            "Inter Bold must preserve its weight");
    require(assets.resolve_font("NotoSansCJK-Light.ttc") == fixture.root / "standard/NotoSansCJK-Light.ttc",
            "relative CJK filename must still resolve");
    for (const auto* name : {"Inter-Light.otf", "Inter-Italic.otf", "Inter-BoldItalic.otf",
                             "JetBrainsMono-Italic.ttf", "JetBrainsMono-Light.ttf", "Go-Regular.ttf"}) {
        require(assets.resolve_font(missing / name).empty(), std::string("must not substitute missing face: ") + name);
    }
    require(assets.resolve_font({}).empty(), "empty face must not resolve to a directory");
    std::filesystem::create_directory(fixture.root / "standard/Inter-Light.otf");
    require(assets.resolve_font(missing / "Inter-Light.otf").empty(), "directories are not fonts");
    fixture.touch("standard/Inter-Regular.otf");
    require(assets.resolve_font(missing / "Inter-Regular.otf") == fixture.root / "standard/Inter-Regular.otf",
            "exact font filenames must take priority over factory aliases");
    fixture.touch("standard/JetBrainsMono-Regular.ttf");
    require(assets.resolve_font(fixture.root / "launcher/JetBrainsMono-Regular.ttf") == fixture.root / "launcher/JetBrainsMono-Regular.ttf",
            "an existing absolute path must take priority");
}

void check_system_fonts(bool factory_image) {
    app::AssetManager assets;
    if (factory_image) {
        require(assets.resolve_font("/usr/share/fonts/opentype/inter/Inter-Regular.otf") == "/usr/share/factory_test/fonts/inter-regular.ttf",
                "factory Inter must be discovered");
        require(assets.resolve_font("/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf") == "/usr/share/APPLaunch/share/font/JetBrainsMono-Regular.ttf",
                "factory JetBrains Mono must be discovered");
    }
    int loaded = 0;
    model::PreviewModel model;
    for (int family = 0; family < 5; ++family) {
        for (int language = 0; language < 10; ++language) {
            for (int typeface = 0; typeface < 6; ++typeface) {
                for (int weight = 0; weight < 3; ++weight) {
                    const bool light = model.weight() == model::PreviewWeight::Light;
                    const bool mono = model.typeface() == model::PreviewTypeface::Mono ||
                                      model.typeface() == model::PreviewTypeface::MonoItalic;
                    if (model.font() == model::PreviewFont::NotoCjk && light) {
                        require(model.has_font_face() == !mono, "CJK Light must support proportional faces only");
                    }
                    if (model.font() == model::PreviewFont::DejaVu && light) {
                        require(model.has_font_face() == (model.typeface() == model::PreviewTypeface::Sans),
                                "DejaVu ExtraLight must support upright Sans only");
                    }
                    if (model.has_font_face() && !assets.resolve_font(model.font_file_name()).empty()) {
                        auto* font = assets.load_font(model.font_file_name(), 20, model.uses_synthetic_italic(), model.font_face_index());
                        require(font != nullptr, std::string("font must load: ") + model.font_file_name());
                        lv_font_glyph_dsc_t glyph{};
                        require(lv_font_get_glyph_dsc(font, &glyph, 'A', 0), "loaded face must contain a Latin test glyph");
                        ++loaded;
                        assets.release_unused_fonts({});
                    }
                    set_field(model, model::PreviewField::Weight);
                    model.select_next_value();
                }
                set_field(model, model::PreviewField::Typeface);
                model.select_next_value();
            }
            set_field(model, model::PreviewField::Language);
            model.select_next_value();
        }
        set_field(model, model::PreviewField::Font);
        model.select_next_value();
    }
    if (factory_image) require(loaded == 330, "factory image must load all 330 available family/language/style/weight combinations");
    std::cout << "Font resolution checks passed; " << loaded << " system font combinations loaded.\n";
}
}

int main(int argc, char** argv) {
    try {
        lv_init();
        check_resolution();
        check_system_fonts(argc > 1 && std::string(argv[1]) == "--factory-image");
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

# FontPreview

A 320×170 typography preview application for CardputerZero, built with C++17, LVGL 9.5, SDL2, and FreeType.

The screen has a compact parameter panel on the left and a scrollable, full-height text preview on the right. Interface labels use Noto CJK; on CardputerZero all five font families are loaded from Debian system packages.

## Preview options

- Preview content: Simplified Chinese, Traditional Chinese, Japanese, Korean, English, Portuguese, Czech, Greek, Russian, and Code
- Fonts: Noto CJK, Go, Inter, DejaVu, and JetBrains Mono
- Typefaces: Sans, Sans Italic, Serif, Serif Italic, Mono, and Mono Italic
- Sizes: 8–24 px in 1 px steps, stopping at either boundary; hold Left/Right for rapid adjustment
- Weights: Light, Regular, and Bold. Light is available for Inter and JetBrains Mono.
- Colors: Black/White, White/Black, Sepia, Terminal, Solarized, Navy, Amber, and Slate

## Controls

| CardputerZero | Desktop | Action |
| --- | --- | --- |
| `F` | Up arrow or `F` | Select the previous parameter |
| `X` | Down arrow or `X` | Select the next parameter |
| `Z` | Left arrow or `Z` | Previous value |
| `C` | Right arrow or `C` | Next value |
| `L` | `L` | Scroll preview up |
| `M` | `M` | Scroll preview down |
| Esc | Esc | Exit |

## Interface sounds

FontPreview uses four short cues from the CC0 [UI SFX](https://uisfx.com/) Arcade pack. `focus` marks Up/Down movement between parameter fields, while `select` confirms a successful Left/Right option change. `blocked` marks a size boundary or unsupported typeface or weight, and `long-press` announces entry into rapid size adjustment. Ordinary preview scrolling and every repeated size step stay silent to avoid noisy high-frequency feedback.

Arcade feedback is always enabled when the audio device is available. The PCM WAV cues use SDL2's audio queue directly; SDL_mixer, extra codecs, and MIDI soundfonts are not needed. Audio reinforces the existing visual response and is never the only status signal. The bundled audio is dedicated to the public domain under CC0-1.0; see `assets/audio/LICENSE-UISFX-AUDIO.txt`.

## Fonts

On CardputerZero, the application loads the complete Regular and Bold Noto CJK faces from the system `fonts-noto-cjk` package:

- `/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc`
- `/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc`
- `/usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc`
- `/usr/share/fonts/opentype/noto/NotoSerifCJK-Bold.ttc`

The repository and Debian package do not include font files. For a desktop preview, provide locally installed Noto CJK files under an external `fonts/` directory and configure with `-DAPP_ASSETS_ROOT=/path/to/local/assets`. Fonts are never copied into the CardputerZero package.

The application selects the correct regional face inside each collection: JP for Japanese, KR for Korean, SC for Simplified Chinese, and TC for Traditional Chinese. Mono uses the matching regional Noto Sans Mono face already contained in the Sans TTC files, so separate Mono OTF copies are unnecessary.

The fonts are licensed under the SIL Open Font License 1.1. Installed CardputerZero fonts and their copyright information are managed by Debian's `fonts-noto-cjk` package.

Go, Inter, DejaVu, and JetBrains Mono are also loaded from their standard Debian system paths. Only `fonts-noto-cjk` is a required font dependency because it supplies the interface and default preview. Go, Inter, DejaVu, and JetBrains Mono are optional: existing system files are used, and a missing face is reported in the preview without blocking installation. The package suggests `fonts-go`, `fonts-inter`, `fonts-dejavu-core`, `fonts-dejavu-extra`, and `fonts-jetbrains-mono`; it does not request their installation automatically. DejaVu Mono is included in `fonts-dejavu-core` on Debian Bookworm, and pulled in by that package on Trixie. There is no unconditional dependency on `fonts-dejavu-mono`, which is unavailable on Bookworm. Inter and JetBrains Mono provide native Light and Light Italic files; the other configured families do not provide a Light face. Unsupported combinations are reported directly in the preview—for example, Inter does not provide Serif or Mono faces. Noto CJK italic selections use FreeType's synthetic italic rendering because Noto CJK does not ship native italic faces.

## Character coverage

The Latin, Greek, and Cyrillic previews begin with explicit locale character rows based on Unicode CLDR, followed by natural text or pangrams. The application intentionally does not substitute a fallback font: a missing-glyph box is part of the test result. Character coverage therefore varies with the selected family, making it possible to compare the broad DejaVu coverage with the more specialized Inter, Go, JetBrains Mono, and Noto CJK families.

## Build

Configure and build the macOS desktop preview:

```shell
cmake --preset darwin-arm64
cmake --build --preset darwin-arm64-dbg
./build/darwin-arm64/Debug/fontpreview
```

Build the CardputerZero ARM64 Debian package:

```shell
cmake --preset cp0-cross
cmake --build --preset cp0-cross-rel
cpack --preset cp0-cross-deb
```

The package is written to `dist/fontpreview_0.4.4-m5stack1_arm64.deb` and uses the system font packages listed above without installing duplicate font files.

## Installation and publishing

The package has no maintainer scripts or background services. It does not run
font downloads, update font caches, or start the application during installation.
Its required libraries and Noto CJK are resolved by the system package manager.
Go, Inter, and extra font faces can be installed separately when wanted.

If a store installation stalls while downloading or resolving dependencies,
check the device's network, configured APT repositories, available storage, and
package-manager output. From a device terminal, installing a copied package with
`sudo apt install ./fontpreview_0.4.4-m5stack1_arm64.deb` exposes the actual error.
Do not run a second installation while another package-manager process is active.
The application itself should be launched as the normal user.

The store metadata follows the [CardputerZero application development specification](https://cardputer.cc/#/documents/cp0-dev).
The existing share code `FONT` is retained. To submit the rebuilt package, run
this command from the project root:

```shell
~/M5Stack_Repos/AppBuilder/czdev publish --deb ./dist/fontpreview_0.4.4-m5stack1_arm64.deb
```

## 0.4.4 changes

- Complete store metadata with the current category, seven permissions, author, and existing share code.
- Keep system fonts and make nonessential font packages optional to reduce installation dependencies.
- Play the same four WAV cues through SDL2 without SDL_mixer or its codec/soundfont dependencies.
- Release preview fonts after switching faces or sizes; keep the interface fonts and active preview in memory.
- Use the standard lowercase Debian filename and document the external fonts needed for a desktop preview.

## Regression checks

On a desktop build, enable the audio and font lifetime checks with a locally
installed test font (any supported TTF/OTF file; it is not packaged):

```shell
cmake --preset darwin-arm64 -DFONTPREVIEW_BUILD_TESTS=ON -DFONTPREVIEW_TEST_FONT=/path/to/local/font.ttf
cmake --build --preset darwin-arm64-dbg
ctest --test-dir build/darwin-arm64 -C Debug --output-on-failure
```

The checks use SDL's dummy audio device and cover repeated playback, malformed
WAV files, unsupported PCM formats, unavailable audio, repeated font switches,
and CJK regional face selection. If no test font is supplied, the font lifetime
check is skipped.

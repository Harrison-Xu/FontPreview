# CardputerZero 出厂字体路径清单

采集日期：2026-11-12（北京时间）。设备：`CardputerZero-2ae40ccf`，Debian GNU/Linux 13.7（trixie），aarch64，内核 `6.18.39+rpt-rpi-v8`。本清单来自用户确认的全新出厂镜像，采集期间未安装、下载或替换字体。它记录这台设备的镜像内容，其他镜像版本应重新核对。

## 采集范围与原始清单

扫描 `/usr`、`/opt`、`/home/pi` 下的 TTF、OTF、TTC、Type 1（`.t1`、`.pfb`），共 **210 个文件路径**，用 `fc-scan` 读取字体家族、样式及索引。不将 `.afm` 字体度量文件、X11 编码表算作字体文件；不同目录下的副本分别保留，不代表 210 种不同字体。

- [全部文件绝对路径](device-fonts-2026-11-12/font-files.txt)
- [文件、face index、家族和样式 TSV](device-fonts-2026-11-12/font-faces.tsv)
- [系统与字体包状态](device-fonts-2026-11-12/system.txt)（只有 `install ok installed` 才表示已安装）

`fc-list` 主要反映 Fontconfig 已配置目录；启动器和工厂应用私有目录里的字体不一定出现在其中。开发应用时应检查文件本身是否存在、可读，并按实际文件加载。

## FontPreview 当前相关字体

| 字体 | 出厂文件路径 | 实际可用形态 |
| --- | --- | --- |
| Noto Sans CJK | `/usr/share/fonts/opentype/noto/NotoSansCJK-{Light,Regular,Bold}.ttc` | Sans 三种字重；Regular/Bold 另含 Mono。完整字重见后文 |
| Noto Serif CJK | `/usr/share/fonts/opentype/noto/NotoSerifCJK-{Light,Regular,Bold}.ttc` | Serif 三种字重；完整字重见后文 |
| Inter 18pt | `/usr/share/factory_test/fonts/inter-{regular,medium,semibold,bold}.ttf` | 正体 Regular、Medium、SemiBold、Bold；无 Light、Italic 文件 |
| JetBrains Mono | `/usr/share/APPLaunch/share/font/JetBrainsMono-{Regular,Bold}.ttf` | 等宽正体 Regular、Bold；无 Light、Italic 文件 |
| DejaVu | `/usr/share/fonts/truetype/dejavu/` | Sans、Serif、Mono 的 Regular/Bold/Italic/Bold Italic，以及 Sans ExtraLight 正体 |
| Go | 本次扫描未找到 | 应提示不可用 |

表中 `{…}` 为文件名展开写法。FontPreview 的 Light 选项对 DejaVu Sans 使用真实的 `DejaVuSans-ExtraLight.ttf`；Inter 的 Medium/SemiBold 尚无对应的应用字重选项。应用不把缺失的 Light、Italic 替换成 Regular。

### Noto CJK 集合索引

| 区域 | 比例字体 face index | 等宽字体 face index |
| --- | --- | --- |
| 日文 JP | 0 | 5 |
| 韩文 KR | 1 | 6 |
| 简体中文 SC | 2 | 7 |
| 繁体中文 TC | 3 | 8 |
| 香港 HK | 4 | 9 |

**仅 `NotoSansCJK-Regular.ttc` 和 `NotoSansCJK-Bold.ttc` 包含 5–9 的等宽字体。** 本机其他 Sans 字重和全部 Serif 集合只有 0–4。请求不存在的 face index 会加载失败。Noto CJK 没有原生斜体文件；FontPreview 的 Noto 斜体由 FreeType 合成。

### 出厂应用目录的使用限制

`/usr/share/APPLaunch/share/font/` 属于启动器，`/usr/share/factory_test/fonts/` 属于工厂测试应用。这些是本镜像实测路径，可能随应用更新或移除而变化，不能视为所有镜像通用的字体接口。复用前需核对字体许可证和所依赖应用的安装状态。FontPreview 优先使用原有 Debian 路径，再查出厂应用目录；不安装字体，也不将字体打入安装包。

## 其他出厂字体

- **Alibaba PuHuiTi 3.0**：启动器提供 Thin、Regular；工厂测试提供 Regular。
- **Liberation**：Sans、Serif、Mono 及 Sans Narrow，在 `/usr/share/fonts/truetype/liberation/`。
- **GNU FreeFont**：FreeSans、FreeSerif、FreeMono，在 `/usr/share/fonts/truetype/freefont/`。
- **Noto Mono / Noto Sans Mono**：在 `/usr/share/fonts/truetype/noto/`。
- **Cantarell**：`/usr/share/fonts/opentype/cantarell/Cantarell-VF.otf`，可变字体。
- **Nunito Sans**：`/usr/share/fonts/truetype/nunito-sans/`，正体和斜体可变字体。
- **GFS Didot Classic**：`/usr/share/fonts/opentype/didot-classic/GFSDidotClassic.otf`。
- **Droid Sans Fallback**：`/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf` 和 `/usr/share/fonts-droid-fallback/truetype/DroidSansFallback.ttf`。
- **URW Base35**：OpenType 位于 `/usr/share/fonts/opentype/urw-base35/`，另有 Type 1 格式副本。
- **Montserrat Bold**：`/usr/share/APPLaunch/share/font/Montserrat-Bold.ttf`。
- 图标字体、ReportLab Vera、Firefox Twemoji 的路径见下表。这些字体不一定适用于普通文本，彩色 Emoji 还取决于渲染器支持。

可变字体的 `fc-scan` 索引可能大于 65535，含命名实例信息；不能直接当成普通 TTC 的 0–9 索引使用。当前 FontPreview 未增加可变字体轴或这些额外字体家族的选择。

## 完整逐文件清单

每个小节的目录与文件名相接即为完整绝对路径。TTC 的各区域及可变字体的命名实例索引可在 TSV 中逐行查询。

### `/usr/lib/firefox/fonts`（1 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `TwemojiMozilla.ttf` | Twemoji Mozilla | Regular |

### `/usr/lib/python3/dist-packages/reportlab/fonts`（6 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `DarkGardenMK.pfb` | Dark Garden | Regular |
| `Vera.ttf` | Bitstream Vera Sans | Roman |
| `VeraBI.ttf` | Bitstream Vera Sans | Bold Oblique |
| `VeraBd.ttf` | Bitstream Vera Sans | Bold |
| `VeraIt.ttf` | Bitstream Vera Sans | Oblique |
| `callig15.pfb` | CALLIG15 | Regular |

### `/usr/share/APPLaunch/share/font`（12 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `AlibabaPuHuiTi-3-35-Thin.ttf` | Alibaba PuHuiTi 3.0 | 35 Thin |
| `AlibabaPuHuiTi-3-55-Regular.ttf` | Alibaba PuHuiTi 3.0 | 55 Regular |
| `DejaVuSans.ttf` | DejaVu Sans | Book |
| `DejaVuSansMono.ttf` | DejaVu Sans Mono | Book |
| `DejaVuSerif.ttf` | DejaVu Serif | Book |
| `JetBrainsMono-Bold.ttf` | JetBrains Mono | Bold |
| `JetBrainsMono-Regular.ttf` | JetBrains Mono | Regular |
| `LiberationMono-Regular.ttf` | Liberation Mono | Regular |
| `Montserrat-Bold.ttf` | Montserrat | Bold |
| `NotoSansCJK-Regular.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK, Noto Sans Mono CJK JP, Noto Sans Mono CJK KR, Noto Sans Mono CJK SC, Noto Sans Mono CJK TC, Noto Sans Mono CJK HK | Regular |
| `NotoSerifCJK-Regular.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Regular |
| `svgfont.ttf` | svgfont | Regular |

### `/usr/share/CameraApp/assets/fonts`（2 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `camera_icons.ttf` | camera-icons | Regular |
| `kenney_input_keyboard_and_mouse.ttf` | Kenney Input Keyboard & Mouse | Regular |

### `/usr/share/camera_app/fonts`（2 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `camera_icons.ttf` | camera-icons | Regular |
| `kenney_input_keyboard_and_mouse.ttf` | Kenney Input Keyboard & Mouse | Regular |

### `/usr/share/factory_test/fonts`（9 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `Phosphor-Fill.ttf` | Phosphor-Fill | Regular |
| `alibaba-puhui-regular.ttf` | Alibaba PuHuiTi 3.0 | 55 Regular |
| `camera_icons.ttf` | camera-icons | Regular |
| `inter-bold.ttf` | Inter 18pt | Bold |
| `inter-medium.ttf` | Inter 18pt | Medium |
| `inter-regular.ttf` | Inter 18pt | Regular |
| `inter-semibold.ttf` | Inter 18pt | SemiBold |
| `kenney_input_keyboard_and_mouse.ttf` | Kenney Input Keyboard & Mouse | Regular |
| `kenny_keyboard_extra.ttf` | kenny_extra | Regular |

### `/usr/share/fonts-droid-fallback/truetype`（1 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `DroidSansFallback.ttf` | Droid Sans Fallback | Regular |

### `/usr/share/fonts/X11/Type1`（35 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `C059-BdIta.pfb` | C059 | Bold Italic |
| `C059-Bold.pfb` | C059 | Bold |
| `C059-Italic.pfb` | C059 | Italic |
| `C059-Roman.pfb` | C059 | Roman |
| `D050000L.pfb` | D050000L | Regular |
| `NimbusMonoPS-Bold.pfb` | Nimbus Mono PS | Bold |
| `NimbusMonoPS-BoldItalic.pfb` | Nimbus Mono PS | Bold Italic |
| `NimbusMonoPS-Italic.pfb` | Nimbus Mono PS | Italic |
| `NimbusMonoPS-Regular.pfb` | Nimbus Mono PS | Regular |
| `NimbusRoman-Bold.pfb` | Nimbus Roman | Bold |
| `NimbusRoman-BoldItalic.pfb` | Nimbus Roman | Bold Italic |
| `NimbusRoman-Italic.pfb` | Nimbus Roman | Italic |
| `NimbusRoman-Regular.pfb` | Nimbus Roman | Regular |
| `NimbusSans-Bold.pfb` | Nimbus Sans | Bold |
| `NimbusSans-BoldItalic.pfb` | Nimbus Sans | Bold Italic |
| `NimbusSans-Italic.pfb` | Nimbus Sans | Italic |
| `NimbusSans-Regular.pfb` | Nimbus Sans | Regular |
| `NimbusSansNarrow-Bold.pfb` | Nimbus Sans Narrow | Bold |
| `NimbusSansNarrow-BoldOblique.pfb` | Nimbus Sans Narrow | Bold Oblique |
| `NimbusSansNarrow-Oblique.pfb` | Nimbus Sans Narrow | Oblique |
| `NimbusSansNarrow-Regular.pfb` | Nimbus Sans Narrow | Regular |
| `P052-Bold.pfb` | P052 | Bold |
| `P052-BoldItalic.pfb` | P052 | Bold Italic |
| `P052-Italic.pfb` | P052 | Italic |
| `P052-Roman.pfb` | P052 | Roman |
| `StandardSymbolsPS.pfb` | Standard Symbols PS | Regular |
| `URWBookman-Demi.pfb` | URW Bookman | Demi |
| `URWBookman-DemiItalic.pfb` | URW Bookman | Demi Italic |
| `URWBookman-Light.pfb` | URW Bookman | Light |
| `URWBookman-LightItalic.pfb` | URW Bookman | Light Italic |
| `URWGothic-Book.pfb` | URW Gothic | Book |
| `URWGothic-BookOblique.pfb` | URW Gothic | Book Oblique |
| `URWGothic-Demi.pfb` | URW Gothic | Demi |
| `URWGothic-DemiOblique.pfb` | URW Gothic | Demi Oblique |
| `Z003-MediumItalic.pfb` | Z003 | Medium Italic |

### `/usr/share/fonts/opentype/cantarell`（1 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `Cantarell-VF.otf` | Cantarell | Regular, Light, Bold, Extra Bold, Thin |

### `/usr/share/fonts/opentype/didot-classic`（1 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `GFSDidotClassic.otf` | GFS Didot Classic | Regular |

### `/usr/share/fonts/opentype/noto`（14 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `NotoSansCJK-Black.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK | Black |
| `NotoSansCJK-Bold.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK, Noto Sans Mono CJK JP, Noto Sans Mono CJK KR, Noto Sans Mono CJK SC, Noto Sans Mono CJK TC, Noto Sans Mono CJK HK | Bold |
| `NotoSansCJK-DemiLight.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK | DemiLight |
| `NotoSansCJK-Light.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK | Light |
| `NotoSansCJK-Medium.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK | Medium |
| `NotoSansCJK-Regular.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK, Noto Sans Mono CJK JP, Noto Sans Mono CJK KR, Noto Sans Mono CJK SC, Noto Sans Mono CJK TC, Noto Sans Mono CJK HK | Regular |
| `NotoSansCJK-Thin.ttc` | Noto Sans CJK JP, Noto Sans CJK KR, Noto Sans CJK SC, Noto Sans CJK TC, Noto Sans CJK HK | Thin |
| `NotoSerifCJK-Black.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Black |
| `NotoSerifCJK-Bold.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Bold |
| `NotoSerifCJK-ExtraLight.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | ExtraLight |
| `NotoSerifCJK-Light.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Light |
| `NotoSerifCJK-Medium.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Medium |
| `NotoSerifCJK-Regular.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | Regular |
| `NotoSerifCJK-SemiBold.ttc` | Noto Serif CJK JP, Noto Serif CJK KR, Noto Serif CJK SC, Noto Serif CJK TC, Noto Serif CJK HK | SemiBold |

### `/usr/share/fonts/opentype/urw-base35`（35 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `C059-BdIta.otf` | C059 | Bold Italic |
| `C059-Bold.otf` | C059 | Bold |
| `C059-Italic.otf` | C059 | Italic |
| `C059-Roman.otf` | C059 | Roman |
| `D050000L.otf` | D050000L | Regular |
| `NimbusMonoPS-Bold.otf` | Nimbus Mono PS | Bold |
| `NimbusMonoPS-BoldItalic.otf` | Nimbus Mono PS | Bold Italic |
| `NimbusMonoPS-Italic.otf` | Nimbus Mono PS | Italic |
| `NimbusMonoPS-Regular.otf` | Nimbus Mono PS | Regular |
| `NimbusRoman-Bold.otf` | Nimbus Roman | Bold |
| `NimbusRoman-BoldItalic.otf` | Nimbus Roman | Bold Italic |
| `NimbusRoman-Italic.otf` | Nimbus Roman | Italic |
| `NimbusRoman-Regular.otf` | Nimbus Roman | Regular |
| `NimbusSans-Bold.otf` | Nimbus Sans | Bold |
| `NimbusSans-BoldItalic.otf` | Nimbus Sans | Bold Italic |
| `NimbusSans-Italic.otf` | Nimbus Sans | Italic |
| `NimbusSans-Regular.otf` | Nimbus Sans | Regular |
| `NimbusSansNarrow-Bold.otf` | Nimbus Sans Narrow | Bold |
| `NimbusSansNarrow-BoldOblique.otf` | Nimbus Sans Narrow | Bold Oblique |
| `NimbusSansNarrow-Oblique.otf` | Nimbus Sans Narrow | Oblique |
| `NimbusSansNarrow-Regular.otf` | Nimbus Sans Narrow | Regular |
| `P052-Bold.otf` | P052 | Bold |
| `P052-BoldItalic.otf` | P052 | Bold Italic |
| `P052-Italic.otf` | P052 | Italic |
| `P052-Roman.otf` | P052 | Roman |
| `StandardSymbolsPS.otf` | Standard Symbols PS | Regular |
| `URWBookman-Demi.otf` | URW Bookman | Demi |
| `URWBookman-DemiItalic.otf` | URW Bookman | Demi Italic |
| `URWBookman-Light.otf` | URW Bookman | Light |
| `URWBookman-LightItalic.otf` | URW Bookman | Light Italic |
| `URWGothic-Book.otf` | URW Gothic | Book |
| `URWGothic-BookOblique.otf` | URW Gothic | Book Oblique |
| `URWGothic-Demi.otf` | URW Gothic | Demi |
| `URWGothic-DemiOblique.otf` | URW Gothic | Demi Oblique |
| `Z003-MediumItalic.otf` | Z003 | Medium Italic |

### `/usr/share/fonts/truetype/dejavu`（22 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `DejaVuMathTeXGyre.ttf` | DejaVu Math TeX Gyre | Regular |
| `DejaVuSans-Bold.ttf` | DejaVu Sans | Bold |
| `DejaVuSans-BoldOblique.ttf` | DejaVu Sans | Bold Oblique |
| `DejaVuSans-ExtraLight.ttf` | DejaVu Sans | ExtraLight |
| `DejaVuSans-Oblique.ttf` | DejaVu Sans | Oblique |
| `DejaVuSans.ttf` | DejaVu Sans | Book |
| `DejaVuSansCondensed-Bold.ttf` | DejaVu Sans | Condensed Bold |
| `DejaVuSansCondensed-BoldOblique.ttf` | DejaVu Sans | Condensed Bold Oblique |
| `DejaVuSansCondensed-Oblique.ttf` | DejaVu Sans | Condensed Oblique |
| `DejaVuSansCondensed.ttf` | DejaVu Sans | Condensed |
| `DejaVuSansMono-Bold.ttf` | DejaVu Sans Mono | Bold |
| `DejaVuSansMono-BoldOblique.ttf` | DejaVu Sans Mono | Bold Oblique |
| `DejaVuSansMono-Oblique.ttf` | DejaVu Sans Mono | Oblique |
| `DejaVuSansMono.ttf` | DejaVu Sans Mono | Book |
| `DejaVuSerif-Bold.ttf` | DejaVu Serif | Bold |
| `DejaVuSerif-BoldItalic.ttf` | DejaVu Serif | Bold Italic |
| `DejaVuSerif-Italic.ttf` | DejaVu Serif | Italic |
| `DejaVuSerif.ttf` | DejaVu Serif | Book |
| `DejaVuSerifCondensed-Bold.ttf` | DejaVu Serif | Condensed Bold |
| `DejaVuSerifCondensed-BoldItalic.ttf` | DejaVu Serif | Condensed Bold Italic |
| `DejaVuSerifCondensed-Italic.ttf` | DejaVu Serif | Condensed Italic |
| `DejaVuSerifCondensed.ttf` | DejaVu Serif | Condensed |

### `/usr/share/fonts/truetype/droid`（1 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `DroidSansFallbackFull.ttf` | Droid Sans Fallback | Regular |

### `/usr/share/fonts/truetype/freefont`（12 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `FreeMono.ttf` | FreeMono | Regular |
| `FreeMonoBold.ttf` | FreeMono | Bold |
| `FreeMonoBoldOblique.ttf` | FreeMono | Bold Oblique |
| `FreeMonoOblique.ttf` | FreeMono | Oblique |
| `FreeSans.ttf` | FreeSans | Regular |
| `FreeSansBold.ttf` | FreeSans | Bold |
| `FreeSansBoldOblique.ttf` | FreeSans | Bold Oblique |
| `FreeSansOblique.ttf` | FreeSans | Oblique |
| `FreeSerif.ttf` | FreeSerif | Regular |
| `FreeSerifBold.ttf` | FreeSerif | Bold |
| `FreeSerifBoldItalic.ttf` | FreeSerif | Bold Italic |
| `FreeSerifItalic.ttf` | FreeSerif | Italic |

### `/usr/share/fonts/truetype/liberation`（16 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `LiberationMono-Bold.ttf` | Liberation Mono | Bold |
| `LiberationMono-BoldItalic.ttf` | Liberation Mono | Bold Italic |
| `LiberationMono-Italic.ttf` | Liberation Mono | Italic |
| `LiberationMono-Regular.ttf` | Liberation Mono | Regular |
| `LiberationSans-Bold.ttf` | Liberation Sans | Bold |
| `LiberationSans-BoldItalic.ttf` | Liberation Sans | Bold Italic |
| `LiberationSans-Italic.ttf` | Liberation Sans | Italic |
| `LiberationSans-Regular.ttf` | Liberation Sans | Regular |
| `LiberationSansNarrow-Bold.ttf` | Liberation Sans Narrow | Bold |
| `LiberationSansNarrow-BoldItalic.ttf` | Liberation Sans Narrow | Bold Italic |
| `LiberationSansNarrow-Italic.ttf` | Liberation Sans Narrow | Italic |
| `LiberationSansNarrow-Regular.ttf` | Liberation Sans Narrow | Regular |
| `LiberationSerif-Bold.ttf` | Liberation Serif | Bold |
| `LiberationSerif-BoldItalic.ttf` | Liberation Serif | Bold Italic |
| `LiberationSerif-Italic.ttf` | Liberation Serif | Italic |
| `LiberationSerif-Regular.ttf` | Liberation Serif | Regular |

### `/usr/share/fonts/truetype/noto`（3 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `NotoMono-Regular.ttf` | Noto Mono | Regular |
| `NotoSansMono-Bold.ttf` | Noto Sans Mono | Bold |
| `NotoSansMono-Regular.ttf` | Noto Sans Mono | Regular |

### `/usr/share/fonts/truetype/nunito-sans`（2 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `NunitoSans-Italic-VariableFont_YTLC,opsz,wdth,wght.ttf` | Nunito Sans | 12pt ExtraLight Italic, Light Italic, Italic, Medium Italic, SemiBold Italic, Bold Italic, ExtraBold Italic, Black Italic |
| `NunitoSans-VariableFont_YTLC,opsz,wdth,wght.ttf` | Nunito Sans | 12pt ExtraLight, Light, Regular, Medium, SemiBold, Bold, ExtraBold, Black |

### `/usr/share/fonts/type1/urw-base35`（35 个文件）

| 文件名 | 字体家族 | 样式 |
| --- | --- | --- |
| `C059-BdIta.t1` | C059 | Bold Italic |
| `C059-Bold.t1` | C059 | Bold |
| `C059-Italic.t1` | C059 | Italic |
| `C059-Roman.t1` | C059 | Roman |
| `D050000L.t1` | D050000L | Regular |
| `NimbusMonoPS-Bold.t1` | Nimbus Mono PS | Bold |
| `NimbusMonoPS-BoldItalic.t1` | Nimbus Mono PS | Bold Italic |
| `NimbusMonoPS-Italic.t1` | Nimbus Mono PS | Italic |
| `NimbusMonoPS-Regular.t1` | Nimbus Mono PS | Regular |
| `NimbusRoman-Bold.t1` | Nimbus Roman | Bold |
| `NimbusRoman-BoldItalic.t1` | Nimbus Roman | Bold Italic |
| `NimbusRoman-Italic.t1` | Nimbus Roman | Italic |
| `NimbusRoman-Regular.t1` | Nimbus Roman | Regular |
| `NimbusSans-Bold.t1` | Nimbus Sans | Bold |
| `NimbusSans-BoldItalic.t1` | Nimbus Sans | Bold Italic |
| `NimbusSans-Italic.t1` | Nimbus Sans | Italic |
| `NimbusSans-Regular.t1` | Nimbus Sans | Regular |
| `NimbusSansNarrow-Bold.t1` | Nimbus Sans Narrow | Bold |
| `NimbusSansNarrow-BoldOblique.t1` | Nimbus Sans Narrow | Bold Oblique |
| `NimbusSansNarrow-Oblique.t1` | Nimbus Sans Narrow | Oblique |
| `NimbusSansNarrow-Regular.t1` | Nimbus Sans Narrow | Regular |
| `P052-Bold.t1` | P052 | Bold |
| `P052-BoldItalic.t1` | P052 | Bold Italic |
| `P052-Italic.t1` | P052 | Italic |
| `P052-Roman.t1` | P052 | Roman |
| `StandardSymbolsPS.t1` | Standard Symbols PS | Regular |
| `URWBookman-Demi.t1` | URW Bookman | Demi |
| `URWBookman-DemiItalic.t1` | URW Bookman | Demi Italic |
| `URWBookman-Light.t1` | URW Bookman | Light |
| `URWBookman-LightItalic.t1` | URW Bookman | Light Italic |
| `URWGothic-Book.t1` | URW Gothic | Book |
| `URWGothic-BookOblique.t1` | URW Gothic | Book Oblique |
| `URWGothic-Demi.t1` | URW Gothic | Demi |
| `URWGothic-DemiOblique.t1` | URW Gothic | Demi Oblique |
| `Z003-MediumItalic.t1` | Z003 | Medium Italic |


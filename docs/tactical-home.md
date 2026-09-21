<p align="right"><a href="tactical-home.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Tactical identity layout (2.4.0)

The homepage follows the user's supplied 240 × 320 HTML composition: a solid
accent-color banner with a diagonal cut, a large left portrait, stacked
identity details on the right, and a filled/outlined alias opposite a
multiline signature. The shared status bar and four bottom shortcuts remain.
The actual stored badge theme determines every color; yellow is only a demo.
No Tailwind, external image or remote font service is needed on the device.

In 2.4.2 the banner cut moves 28px left, from (165,79)–(153,89) to
(137,79)–(125,89), just before the name column at x=145. Browser and device
share this geometry; stored badge images do not need to be regenerated.

In 2.4.3 the QR page reuses the homepage status bar, brand banner and bottom
frame. The middle becomes a neutral black/white 176px QR at (32,100): 32px
side margins, 11px below the banner and 12px before the lowest frame rise.
The original 192px stored code is unchanged. A binary nearest-center display
buffer avoids blurred edges and LVGL transform allocations. Browser preview
uses the same sampling, and upload validation decodes both sizes. The footer
shows the existing return action; no portrait or identity text sits behind
the code. Empty codes show an upload hint inside the same shell.

The optional `alias` field appears only as the large footer lettering in 2.4.4. When blank, footer lettering uses the existing name. Names wrap
to two lines, department/title to two lines, and signatures to three lines;
oversized text is fitted or ellipsized without changing stored metadata.
Orbitron/Caveat remain the same offline fonts as [2.3.0](portrait-home.md).
Chinese signatures use the browser's available KaiTi or Chinese fallback.

Photo fades are separate display overlays. The stored photo remains unfaded,
so reading and saving a badge repeatedly does not progressively darken it.
Browser preview and LVGL both use a 140 × 180 displayed photo and the same
horizontal/vertical fade regions. Fade objects have no animation or timer.

## V4 data layout

### Typography refinement (2.4.1)

Text blocks now flow according to their wrapped line count. The name owns the
first 32 pixel rows for a clean crop in the badge switcher. The alias has a
4-pixel gap; other visible blocks have 7–12 pixels between their line boxes.
Empty fields do not reserve a blank block. Names use 14–16px Orbitron with
Chinese sans-serif fallback, departments use 10–12px semibold sans-serif,
and titles use 10–11px sans-serif. IDs use 10–11px monospace instead of the
wide display face. Names/departments use the theme text color, ID/title use
its muted color, and aliases/signatures use its accent.

Body leading adapts from 13–17px to the available height. Signatures have
at least 3px leading above their font size and are vertically centered in
the 40px footer. Latin words wrap as units when they fit; oversized tokens
fall back to grapheme wrapping. The existing font files and V4 storage
format remain unchanged. Previously saved text images need to be regenerated
and saved to adopt the new typography; installing the application alone
does not overwrite them.

Partitions and alternating 0x18000-byte profile banks are unchanged. The
91,648-byte payload contains:

| Asset | Bytes | Interpretation |
| --- | ---: | --- |
| Card atlas | 43,264 | 84 × 148 RGB565 side panel, then 208 × 40 footer, then padding |
| Avatar | 32,256 | 112 × 144 RGB565 original cropped photo |
| Brand | 9,472 | 148 × 32 RGB565 |
| Logo | 2,048 | 32 × 32 RGB565 |
| QR | 4,608 | 192 × 192, one bit per pixel |

The card transport dimensions are 208 × 104; the atlas is not a single
displayed rectangle. Asset offsets are selected by each badge's version.
V1–V3 continue to load and use their existing display paths. The protocol
remains 2 and advertises `maxProfileVersion: 4`. V4 JSON adds a string `alias`
of up to 128 UTF-8 bytes, disallowing control characters. All prior fields
retain their validation. Unsupported versions fail before any bank erase.

The editor reads actual per-badge image dimensions before loading saved
photos. A V4 save upgrades only the selected badge with its revision guard.
Firmware updates write only the application partition. NVS, Wi-Fi, audio
resources and unselected cards remain intact. Original card files and
private migration snapshots stay outside the source archive.

## Validation

Persistence tests exercise mixed V2/V3/V4 slots, every asset boundary,
interrupted transfers and interrupted commits. Browser checks exercise the
real V4 save handler, alias, Unicode wrapping, native photo roundtrips and
exact 91,648-byte packing. The LVGL renderer completed 200 home/list switches
using a 32 KiB pool (16,856 bytes peak in the native test). Hardware validation
also checks the application flash digest, all uploaded image bytes, preserved
profile fields, active card and Wi-Fi, plus persistence after restart.

## Shared bottom actions (2.4.4)

All mini apps and system settings use a single 10px action row at y=300,
with 8px side insets and a 224px text area. `main/badge_footer.h` owns the
shared style and hints; long-press labels identify holding Up, Down or OK. Muyu, voice lists/volume, radio and its settings, Xiaozhi states,
brightness, hotspot/profile setup and the return chooser share this row.
Button hints were removed from page subtitles and body messages. State,
network instructions and content stay above it. Existing button behavior
is unchanged. The small font generator collects the hint glyphs; native
LVGL checks verify glyph coverage, one-line width and the rendered pages.

### Header ID and signature placement

In 2.4.4, employee ID is a 168 x 12 raster at (60,31), right-aligned inside
the brand banner. The side panel contains only name, department and title.
The footer alias remains at y=250; the signature is separately drawn at
y=244, six pixels higher.

The V4 payload remains 91,648 bytes. A new atlas uses 84 x 124 pixels for
the side panel (offset 0), 168 x 12 for the header ID (offset 20,832), and
the unchanged footer at 24,864. The ASCII marker `IDH1` at offset 41,504
identifies this layout. Existing V4 atlases without the marker retain their
original 148px side panel. The device advertises `maxBadgeLayout: 2`; the
editor requires it before saving, preventing uploads to an older renderer.
Profile JSON, photo, logo, QR and partition sizes are unchanged. Existing
cards need their text atlases regenerated to move the ID and remove the
inline alias.

## Fixed identity zones and AI shortcut (2.4.5)

Name, department and title keep fixed 32/34/32px zones at panel offsets
0/44/90. Each holds two lines; one-line text is vertically centered and
empty fields keep their space. The atlas and all asset sizes remain unchanged.
Existing cards need their text images refreshed to adopt the spacing.

The terminal order is Mini Apps, My QR Code, Edit Profile, Settings, Home.
On configured Home, holding Up directly opens Xiaozhi; a short Up still opens
the app list. The runtime resolves the stable `xiaozhi` registry ID rather
than a hardcoded list index. A directly launched app returns to Home through
Previous; list-launched apps return to the list. Both use the existing worker
stop and UI teardown flow. All five Home shortcut hints share the bottom row.
Navigation tests cover long/short gestures, missing apps, empty setup,
repeated launches and both return paths. Browser checks compare fixed zone
coordinates for short, long and empty fields.

## Badge switcher name bounds (2.4.6)

Five badge rows now start at y=100 with a 38px pitch and 36px height. The
32px name strip is inset 2px vertically in a 128px-wide clipping container,
separate from the row number and active marker. Both lines remain visible;
V3 names retain their existing scale and are centered in the same container.
The final row ends at y=288, above the shared footer. This display-only fix
needs no profile image regeneration and preserves all stored badge data.

Rows use the page background with no extra fill; borders and a left marker
indicate selection. Saved RGB565 name strips become 16-level alpha text in
the current theme's text color, removing their original background colors.
Conversion buffers exist only on the badge switcher and are freed on exit.

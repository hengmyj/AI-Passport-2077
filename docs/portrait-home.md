<p align="right"><a href="portrait-home.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Portrait homepage (2.3.0)

The 240 × 320 homepage uses a centered 112 × 136 portrait, compact brand row,
full-width name with up to two lines, paired department/title, identifier and
signature. Each badge retains its own palette and custom logo. The shared
status bar and four button hints keep their existing actions.

The offline USB/hotspot editor embeds Orbitron 700 for Latin headings and
Caveat 600 for Latin signatures; Chinese uses the browser's available Chinese
font. The browser rasterizes text into RGB565 before upload so the device
does not need a large multilingual font library. Long metadata fields use
an ellipsis; names wrap to two lines before falling back to an ellipsis.

## Storage and compatibility

V3 payload order: information strip (208 × 76, 31,616 bytes), portrait
(112 × 136, 30,464), brand (148 × 32, 9,472), logo (32 × 32, 2,048), QR
(192 × 192, one bit per pixel, 4,608). Total: 78,208 bytes. Partition locations
and 0x18000-byte alternating banks are unchanged; the header commits last.
V1/V2 remain readable and writable. The protocol stays at version 2 and
advertises `maxProfileVersion: 3`; dimensions describe the requested badge's
stored format. New uploads explicitly request format 3.

Old badges can be displayed immediately using legacy crops. Reading and
saving them in the new editor upgrades that badge. Old 72 × 88 photos are
resampled; upload the original photograph again for native portrait detail.
The previous full-flash backup remains a rollback option. Application-only
updates preserve NVS, profile banks and voice partitions.

## Font sources

- [Orbitron](https://github.com/google/fonts/tree/main/ofl/orbitron), SIL OFL,
  `assets/fonts/Orbitron.ttf` and `Orbitron-OFL.txt`.
- [Caveat](https://github.com/google/fonts/tree/main/ofl/caveat), SIL OFL,
  `assets/fonts/Caveat.ttf` and `Caveat-OFL.txt`.

The `*-latin.woff2` subsets include U+0020–00FF and U+2010–2027, generated
with fontTools `pyftsubset INPUT --unicodes=U+0020-00FF,U+2010-2027
--flavor=woff2 --output-file=OUTPUT`. `tools/build_config_page.py` embeds them
as data URLs; no internet connection is needed while configuring the badge.

## Validation

Persistence tests cover mixed V2/V3 badges, all asset offsets and bounds,
interrupted uploads/commits and unchanged neighboring badges. Browser checks
cover fonts, exact payload size, metadata and QR preservation. The actual
LVGL homepage renderer survives 200 home/list transitions in a 32 KiB pool
(18,128 bytes maximum used in the native renderer).

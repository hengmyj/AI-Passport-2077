<p align="right"><a href="storage-captions.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Storage and caption fonts — 2.6.8

The application partition grows from 0x2C0000 to 0x2E0000 bytes. `voice_data`
moves from 0x2D0000 to 0x2F0000 and shrinks by 128 KiB. `voice_tail`, NVS,
PHY and both profile partitions retain their original offsets and sizes. All
726 Opus clips are retained byte-for-byte. The logical voice stream is repacked
at the new split point, leaving 50,800 bytes after its header and payload.
Existing-device installation requires a verified full backup and segmented
migration; an app-only update cannot perform this layout change. The installer
checks an allowlist of layouts, verifies resources, writes the partition table
last, and checks user-data bytes before boot. Never write the merged image for
a preserving update.

Three 14px UI fonts share one bitmap and descriptor table. The legacy character
set is retained in `assets/fonts/ui14-characters.txt`; generated wrappers retain
their public font names. The regression fixture checks every legacy glyph's
pixel bytes, advance, bounds and offsets. Run any of the old 14px generators or
`tools/generate_shared_fonts.py` with the existing Noto Sans SC TTF to regenerate
the shared fonts. Pillow is required.

The XiaoZhi caption font retains GB2312 coverage and adds common name/UI
characters, punctuation and symbols. Its 7,783 glyphs are checked against the
source TTF cmap; `tools/generate_xiaozhi_font.py` also requires fontTools.
It is not a complete Unicode font. Incoming STT, TTS and alert display strings
are normalized once outside the LVGL refresh loop. Common emoji become named
Chinese labels, other pictographic emoji receive a generic emoji label, and unsupported
characters retain their identity as `[U+XXXX]` (or longer for supplementary
codepoints). The literal Han character U+53E3 remains unchanged. Variation selectors, joiners and
skin-tone modifiers do not become missing-glyph boxes. Invalid UTF-8 becomes an
explicit encoding-error label. Bounded output never splits a UTF-8 character or
a replacement token. Audio and the server's conversation are unchanged.

## Validation

The release app is 2,674,896 bytes, with 339,760 bytes free (331.8 KiB).
Compared with 2.6.7, app bytes decrease by 21,216 and headroom grows by 152,288.
This is flash headroom, not additional runtime RAM.

Host checks cover strict UTF-8, output bounds, emoji/unknown characters, all old
UI glyph pixels, all GB2312 caption characters, the voice bundle CRC/split and
allowed migration layouts. A native LVGL check resolves all 7,783 caption glyphs
without placeholder descriptors and renders mixed captions. Compilation and
host rendering alone do not prove the user's particular cloud response or
hardware transport. Device installation results must be reported separately.

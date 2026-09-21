<p align="right"><a href="muyu-validation.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Personnel Badge 1.7.1 validation

Date: 2026-09-15. ESP-IDF 5.5.3 / ESP32-C3 / 8 MB flash / LVGL 9.5.0.

| Field | Result |
| --- | --- |
| Build | PASS — native Windows build, merged image and partition verification |
| Host tests | PASS — eight C programs, 31 Python tests, repository checks, actionlint, Node web and QR tests |
| Device tests | PASS — 1.7.1 segmented installation, startup, unchanged five-profile catalog/active card/Wi-Fi; physical game appearance remains unverified |
| Unverified | Physical controls, physical icon appearance and reconnect transitions, WeChat scanning the physical display, audio, brightness, power removal and long runs |

## Firmware

- Application: 1,511,664 bytes at 0x10000; factory capacity: 6,815,744 bytes.
- Bootloader: 21,024 bytes; partition table: 3,072 bytes; merged image: 1,577,200 bytes.
- Merged image SHA-256: `5668e8537962d8208eb79a7b660e80929ed41bdb2e1bb2ab311a6e0a3a6a4c8f`.
- Version 1.6.0 hardware record: NVS, PHY and original profile addresses retained; four extra profiles occupy the old application partition tail, with a private pre-install backup. Segmented flashing verifies each segment and preserves NVS, PHY and `badge_user`. Startup succeeded; profile storage and USB service reported ESP_OK.
- Static gate ran in WSL; corresponding build, merge and image verification ran natively on Windows. This is not a claim that the single POSIX complete gate invocation passed in this environment.

## Data and protocol (1.3.0)

- Host tests cover legacy reads, v1-to-v2 upgrade, the complete 93,920-byte layout, bounds, CRC, incomplete uploads, partial commits, reboot recovery, QR removal and sequence wraparound.
- Uploaded non-personal test branding, palette, fields, icon and QR to hardware; all 93,920 bytes and metadata matched on complete readback.
- A competing writer session was rejected. Hardware restart during a later incomplete upload retained the previous committed profile and QR.
- Original data was backed up and restored after testing. Private backups and raw test logs remain in the work directory and are not distributed in source archives.

## Hotspot (1.3.0)

- The device started a WPA2 setup hotspot with a random password; USB remained responsive.
- An idle host wireless adapter joined the real hotspot and downloaded the embedded compressed HTML and offline QR libraries.
- HTTP metadata matched USB. Read the existing profile fully and saved it unchanged through HTTP.
- Missing header tokens and foreign Origin requests returned 403. Host regression tests cover IPv4 and mapped IPv6 AP-interface matching and reject non-AP addresses.
- Invalid SSID/password bounds were rejected. Testing closed the hotspot, disconnected temporary Wi-Fi and removed the newly created WLAN profile.
- No real user Wi-Fi password was used to test router association. NVS credential storage and boot connection are implemented; actual network acceptance remains to be checked.

## UI and QR (1.3.0)

- Browser checks exercised Chinese brand/caption edits, live cyan theme changes, and QR file selection, decoding, regeneration and validation feedback.
- Node tests cover RGB565 roundtrip, input validation, QR encode/decode identity, monochrome packing and capacity limits.
- Real LVGL code rendered badge, terminal, library, settings, QR, network and Muyu pages. The 192 × 192 QR render decoded with jsQR to the public test URL without changing content.
- Fixed missing glyphs in network labels and inspected the final network page. Native browser serial-picker and complete mobile-browser interactions were not automated; hardware USB/HTTP protocols were tested.
- The 40 KB LVGL pool had 23,016 bytes free after 1,000 strikes, peak usage 15,976 bytes, fragmentation 3%. After 100 cycles including new pages, free space was 22,952 bytes, equal to the warmed baseline. Synchronous active-screen replacement emits LVGL warnings; the cycles completed. These are host LVGL measurements, not long-term whole-device heap measurements.

## 1.3.1 quick QR

- DOWN on home opens QR immediately. DOWN, OK and long OK return to home. Opening QR from the terminal returns to that terminal and preserves selection. Games remain accessible from the terminal.
- Repeated Windows build/merge/image checks, WSL static checks, Node web tests, LVGL rendering and 100 page cycles. Navigation regressions cover all three close inputs, origin-aware return and the game entry path.
- Inspected final home/QR button hints. Version 1.3.1 was installed and booted successfully; physical button acceptance remains.

## 1.3.2 nearby Wi-Fi list

- USB and hotspot configuration scan automatically after connection. Manual refresh, strongest-first ordering, SSID/security deduplication and up to 16 entries are supported. Selecting an entry fills SSID; hidden networks remain editable manually.
- Host tests cover deduplication, signal ordering, capacity and hidden entries. Node tests cover selection, password drafts, open networks, unsupported security, disconnected controls and literal SSID text rendering.
- Windows build/merge checks, the WSL static gate (eight C programs, 31 Python tests and supporting checks) and both Node suites passed. Version 1.3.2 was installed through COM6 with segment hash verification and booted successfully.
- Real USB scanning found 3 networks; the start request returned in approximately 0.2 seconds. Profile reads stayed responsive, and profile and saved network settings matched before and after scanning.
- An idle wireless adapter joined the real badge hotspot, fetched the updated offline page and found 5 networks through HTTP scanning. The hotspot and configuration API remained available, and the profile revision stayed unchanged. Cleanup removed the temporary WLAN profile, closed the hotspot opened by this test and released serial.
- Counts reflect separate scans in the local environment. Router association with user credentials and the complete mobile-browser selection flow were not exercised.

## 1.3.3 header Wi-Fi status icon

- Badge pages reserve a slot before the battery and use the configured text color. The icon appears after the station obtains an IP, hides on disconnect and updates about once per second. Setup AP alone does not indicate station connectivity. The Muyu game retains its separate UI.
- Production LVGL rendering tests exercise connected, disconnected and AP-only states. Pixel assertions confine changes to the icon slot and verify exact restoration after disconnect. Title and battery stay unobstructed; the rendered image was inspected.
- Windows build/merge verification, WSL static checks and real LVGL rendering passed. Free memory after 100 page cycles matched the baseline. Previous Node web-test results remain applicable to the unchanged configuration page.
- Installed 1.3.3 through COM6 with segment hash verification. Startup and profile storage succeeded. USB readback confirmed connection using the saved network (connected=true); no network password or profile data was written. Serial was released.
- Physical icon appearance has not been checked by camera or a person, and the user's network was not interrupted to test reconnect behavior. Connected status does not verify Internet reachability.

## 1.3.4 compact header

- Removed the BAT prefix: battery is a right-aligned percentage, or -- when unavailable. The Wi-Fi icon is approximately 8 pixels high to match the small header text and sits beside the battery. The configuration preview also removes BAT.
- Windows build/image verification, WSL static checks and Node web tests passed. Production LVGL rendering verified icon visibility, reserved bounds and 100 page cycles. The render was visually inspected; physical screen appearance remains for user confirmation.
- Installed 1.3.4 through COM6 with segment hash verification. Startup and profile storage succeeded. Post-install USB readback confirmed Wi-Fi connectivity and the same profile revision as before installation. Serial was released.

## 1.4.0 home design and AI Passport heading

- Retains the saved five-color palette, adding an orbital emblem, portrait framing, side scales and an engraved classification strip. The three footer actions are spaced separately. Every badge-shell page uses AI Passport at top left, with a matching configuration preview.
- Profile image format, dimensions, position and storage protocol remain compatible. Decorations occupy existing gutters without resaving or replacing the avatar, text, palette or QR. Host renders use synthetic example identity data; user profile data is not distributed.
- Native Windows build/merge verification, WSL static checks and Node web tests passed. Production LVGL renders of default and configured home pages were inspected; Wi-Fi state transitions and bounds passed. The 40 KB LVGL pool retained 23,032 bytes after 1,000 strikes, with peak usage of 16,592 bytes. After 100 page cycles, free memory was 22,992 bytes, matching the warmed baseline.
- Installed 1.4.0 through COM6 with segment hash verification; startup and profile storage succeeded. USB readback confirmed Wi-Fi connectivity and the same profile revision as before installation. Serial was released. Physical screen appearance and controls remain for user confirmation.

## 1.5.0 asymmetric portrait layout

- Rebuilt the home composition: upper-left brand, upper-right emblem, right portrait enlarged from 72 x 88 to 90 x 110, left-aligned name/department/title and a separate bottom ID/signature strip. Saved colors and compact header status remain; the configuration preview matches.
- Read-only image views reuse regions of the existing stored card without writing profile data. Views retain source stride; the signature uses the complete source row width so the final row and buffer stay within the original card allocation.
- Initial rendering caught image-view sizes that did not satisfy LVGL full-stride requirements; corrected them before installation. Final production LVGL rendering passed exact text-pixel comparisons, region bounds, Wi-Fi states and 100 page cycles. Only the portrait is scaled; all other profile-region pixels exactly match their source.
- Windows build/merge verification, WSL static checks and Node web tests passed. The 40 KB LVGL pool retained 23,016 bytes after 1,000 strikes, with peak usage of 18,016 bytes. After 100 page cycles, free memory was 22,952 bytes, matching the warmed baseline. Render examples use synthetic profiles.
- Installed 1.5.0 with segmented flashing and hash verification. Startup, profile storage and Wi-Fi readback succeeded. Profile revision matched its pre-install value; serial was released. Physical appearance and button operation remain for user confirmation.

## 1.5.1 remove DIVISION heading

- Removed DIVISION from the home page and configuration preview according to the final request. Actual department content, existing font metrics, palette and layout remain. The earlier spacing proposal was not shipped.
- Windows build/merge verification, WSL static checks and Node web checks passed. Production LVGL rendering confirmed heading removal and exact profile-region pixels. Free memory after 100 page cycles matched baseline; after 1,000 strikes, 23,016 bytes remained with peak usage of 17,656 bytes.
- After serial became available, installed 1.5.1 using segmented flashing with hash verification. Startup, profile storage and USB Wi-Fi readback succeeded. Serial was released after testing; physical appearance remains for user confirmation.

## 1.6.0 five independent badges

- Each card has an independent pair of committed banks. Slot 1 keeps the original data address; slots 2–5 use the new partition. Host tests cover isolation, switching barriers, an interrupted inactive write, partial commit, clearing one slot and reload. Navigation tests cover home double-OK entry, selection, wrap, empty-slot rejection and return, while retaining the QR and game paths.
- Three Node suites cover profile/QR conversion, Wi-Fi selection and multi-badge editing. Multi-badge tests verify literal catalog names, independent editing/display selection, explicit write targets and stale-revision protection.
- Real LVGL rendering includes all five list rows and current/selected indicators. Header Wi-Fi bounds and exact profile-field pixels pass. The 40 KB pool retains 23,016 bytes after 1,000 strikes, with peak usage 18,016 bytes; after 100 cycles including the badge picker, free space is 22,952 bytes, identical to the warmed baseline.
- Segmented COM6 installation verified hashes. The existing profile metadata and revision matched the private pre-install backup. A temporary fifth card was uploaded without changing the active card; all 93,920 payload bytes matched on readback. Incomplete upload cancellation, switching during upload, implicit legacy writes and stale clear requests were tested.
- Selecting card 5 survived a hardware reset, including its QR. A spare wireless adapter joined the real setup hotspot, downloaded the five-card editor, read specific cards/QR and switched the displayed card through HTTP; USB saw the same selection. Missing token and foreign Origin requests were rejected.
- Cleanup restored card 1, cleared the temporary card, closed the test hotspot, removed only the temporary WLAN profile and released COM6. All original profile assets matched the pre-install backup byte-for-byte; saved Wi-Fi settings remained unchanged and connected. Private raw backups/logs are excluded from deliverables.
- Physical double-click timing and button feel, real screen appearance and phone WeChat scanning remain manual checks. The native browser serial picker and complete mobile browser form were not automated; shared USB/HTTP endpoints and host editor logic were tested.

## 1.6.1 compact switching hint

- Home renders the switching hint and active/total number on one 10 px line. Header badge numbering is removed from all shell pages. The browser preview matches. Four Chinese glyphs are added to the 10 px font; existing ASCII advances are preserved.
- Windows build/merge/image checks, the WSL static gate, Node profile/QR checks and real LVGL renders passed. Profile pixels and Wi-Fi bounds remain intact; 100 page cycles retain the same free memory as the warmed baseline. This change does not modify storage or controls.
- Installed 1.6.1 through COM6 using segmented flashing; all three image hashes verified. Startup reported ready, with buttons, battery and storage available. Readback of all five profiles, their revisions, current selection and saved Wi-Fi settings matched the pre-install snapshot. Saved Wi-Fi connected successfully. COM6 was released. Physical appearance remains unverified.

## 1.7.0 built-in game theme

- `badge_theme` is the shared five-color source for shell and games. Active profile reload updates it under the LVGL lock; Muyu applies palette revisions during its existing UI refresh, including while mounted. No counters, timing, audio or storage behavior changes. New games have a documented theme integration contract.
- Muyu uses original vector percussion artwork and theme-colored striker, feedback, labels and footer. Its AI Passport header and battery use the shell's 10 px font. The old orange bitmap is no longer embedded.
- Production LVGL renders default red, cyan, amber and a custom light palette. Switching themes on a mounted game verifies exact background/panel colors, text and percussion accent pixels, and unchanged count. Existing profile-image and Wi-Fi bounds regressions pass. After 1,000 strikes the 40 KB pool has 22,232 bytes free, peak usage 17,656 bytes; after 100 theme/page cycles it retains 22,216 bytes, equal to its warmed baseline.
- Windows firmware build/merge/partition verification and the complete WSL static gate pass. Screenshot comparisons were visually inspected. This feature shipped with 1.7.1 below. Physical gameplay/theme appearance and audio remain unverified.

## 1.7.1 name wrapping and plain card numbers

- The editor rasterizes names into a clipped 112 x 32 two-row region, preferring word boundaries and reducing the font for longer names. Very long names retain their complete metadata and use an ellipsis only after exhausting both rows at the minimum font. Existing saved images require one read/save in the updated editor. Default firmware text enables wrapping. Home/preview counters use 1/5 and picker rows use 1–5.
- Node tests cover English words, CJK, long unbroken strings, repeated spaces, supplementary Unicode and the two-row limit. A real local browser rendered FOLO / MAKER and a long Chinese name across two rows; its updated help text was checked. No device writes were made by that test browser.
- Real LVGL screenshots use a synthetic two-row fixture, verifying unchanged image-region bounds and the 1/5 counter. The full WSL static gate, Windows build/merge/image checks and existing live-game-theme/page-cycle regressions passed.
- COM6 segmented installation verified all three image hashes and booted 1.7.1. Before/after USB readback matched every slot's profile/revision, active selection and Wi-Fi settings; saved Wi-Fi connected. Serial and the temporary local preview service were released. Actual display appearance, resaving an existing personal name and physical game operation remain unverified.

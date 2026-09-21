<p align="right"><a href="setup-navigation.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# First setup and return navigation — 1.9.0

Current routing (2.2.0): Home UP opens Mini Apps, including from the first-setup
screen. The mini-app list remembers Home or Terminal as its parent. Edit Profile
now displays its hotspot guide directly, with OK to toggle the hotspot in place.
The sections below record the original 1.9.0 behavior and validation.

When no saved profile exists in any of the five slots, Home shows the setup guide and requests the protected configuration hotspot automatically. A configured device continues to boot into its badge. Clearing the final configured profile activates the guide again without deleting any other settings.

The guide displays the hotspot name/password, `http://192.168.4.1`, the steps to save and display a badge, and a reminder to remain connected when the phone reports no Internet. Down opens the terminal and Up opens settings. OK starts the hotspot if it is closed or startup failed. During initial setup the ten-minute idle shutdown is suspended; manual closure is respected. Saving the first profile restores normal idle shutdown with a fresh ten-minute period, preserving the current configuration session.

Opening the network guide from Settings or Edit Profile also starts the hotspot if necessary. Reopening an active hotspot does not rotate its password or disconnect clients. Save the desired profile in the browser; if editing an inactive slot, select **Display this badge** afterward.

## Return choices

Long OK on a non-home page opens a shared chooser:

1. Previous page
2. Home
3. Continue current page

Up/down selects; OK confirms; another long OK cancels. Settings remembers whether Home or the terminal opened it. The network guide remembers whether Settings or Edit Profile opened it. Quick QR still supports its existing short-press return. The chooser also works inside both games: Previous honors the voice game's internal category/volume navigation, while Home stops the game worker before destroying its UI. A stop timeout retains the game for retry.

## Badge spacing

The name region now starts at screen `(20,116)`, department at `(20,158)`, and title at `(20,184)`. The information panel starts at `(14,102)`, providing 14 pixels of top space before the name image and 6 pixels on the left. Existing RGB565 crops and storage formats stay unchanged; saved badges immediately use the new spacing. The browser preview uses exactly the same region table, checked by a Node test.

## Validation — 2026-09-15

| Field | Result |
| --- | --- |
| Build | PASS — native ESP-IDF 5.5.3 build, all five images and resource checks; application 1,755,008 bytes |
| Host tests | PASS — repository/static gate, setup/idle policy, five-slot emptiness and actual parent navigation, chooser confirm/cancel and game stop, Node configuration suites, production LVGL render |
| Device tests | PASS — segmented installation and normal startup, five profiles/active slot/Wi-Fi readback unchanged, real WPA2 hotspot serves the exact updated configuration page and profile readback |
| Unverified | Empty-device first-boot sequence and physical button operation; existing profiles were retained throughout hardware testing |

The partition table and both audio images are byte-identical to 1.8.0; installation only updated program images. UI tests preserve every source pixel in the repositioned profile regions. Under the same 40 KB LVGL pool, 100 voice/Muyu/return-chooser cycles left 17,000 bytes free versus 16,984 before, with a 22,680-byte peak. This is a UI heap test, not an audio or total-device heap measurement.

The static gate ran in WSL; native Windows ran the firmware build/merge/verification. The temporary WLAN profile and hotspot test connection were removed afterward, and COM6 was released. No full flash erase or profile clearing was performed.

As of 2.4.5, configured Home also supports holding Up to open Xiaozhi directly. Previous returns to Home for this route; ordinary app-list entry still returns to the list. My QR Code precedes Edit Profile in the terminal.

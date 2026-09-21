<p align="right">
  <a href="screen-capture.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# Device screen capture

The 2.6.7 candidate adds the USB command `FAP_SCREENSHOT_V1\n`. It exports the
current device-rendered screen as a header followed by exactly 153,600 RGB565LE
bytes. The image is 240 by 320 pixels (portrait 3:4). This is a digital screen
capture, not a photograph of the enclosure or a desktop rendering.

Capture uses the existing partial display buffer after corner masking and
before panel byte swapping. It forces one full redraw while holding the LVGL
lock and streams strips in top-to-bottom order. No additional framebuffer is
allocated. The USB sink has a total three-second timeout. UI updates pause
during capture; prefer idle pages, not active audio or conversations.

The command does not navigate, wake the display, reset the board, or change
saved settings. A sleeping screen returns `FAP_SCREENSHOT_ERROR
screen_unavailable`. Wake it with a physical key first. Only the USB command
accepts screenshot requests; the hotspot configuration API does not.

Open the serial port with DTR and RTS deasserted **before** opening it. Close
the configuration page's serial connection before capture. The host must
validate the header, exact byte length and dimensions, and discard incomplete
transfers. A timeout must not become an apparently successful image.

For release assets, save the original PNG at native resolution. Inspect all
images for personal information and secrets before copying to public assets.
Use the current homepage as the cover; select app pages using normal device
controls. Never relabel host previews as device screenshots. Firmware update
and capture are separate operations; capture itself never flashes or resets.

## Candidate validation

The host strip test checks pixel byte order, stride padding, partial frames,
invalid/reordered strips, transport failure and a fresh retry. Device capture,
USB-disconnect recovery and impact on active audio require on-device testing
after an explicitly authorized installation. No device screenshots are claimed
merely because the candidate compiles.

<p align="right"><a href="xiaozhi-background.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Xiaozhi AI background wake

Version 2.6.1 targets ESP32-C3, 8 MiB Flash, no PSRAM. Xiaozhi moves from Mini Apps
to **Settings → Xiaozhi AI**. Home long-UP still starts conversation.

## Controls

- Start conversation automatically connects using the existing half-duplex UI.
- Background wake persists in `xiaozhi/wake_enabled`, initially off. Disabling
  waits for the microphone worker to stop before completing.
- Reply volume reuses `xiaozhi/volume`, including mute; other app volumes stay independent.
- Wake phrase/status shows the actual model phrase. The bundled model recognizes
  Ni Hao Xiao Zhi. It is not an editable display alias.
- Wi-Fi, hotspot, brightness, profiles and theme remain shared system settings.

## Lifecycle and boundaries

On quiet Home/settings/QR pages, enabled detection records locally without
uploading ambient audio. Detection releases its model and PCM memory, wakes the
display, then starts cloud conversation. Wait for the listening indication before
speaking the command: capture during connection is not retained. Account binding
still requires the user when the cloud service requests activation.

With background wake enabled, only paused or normally ended sessions return to
the origin page after 60 idle seconds. Active listening is never closed based on
low PCM energy. Errors remain visible for retry; manual exit resumes local waiting.
See [stability changes](firmware-optimization.md) for classified faults and bounded recovery.

The navigation task serializes audio ownership. It joins the detector before any
mini app, conversation or reminder starts, then joins that audio owner before
restarting detection. Wooden fish and radio pause detection for their entire
lifetime. Since 2.6.3 the idle soundboard releases its player and resumes detection;
queued or playing clips pause it again. A 750 ms guard reduces playback-tail triggers.
There is no wake during radio playback, echo cancellation or spoken interruption
of replies in this release.

The existing 13 tool names are unchanged; 2.6.3 adds a random-sound tool. Authorization uses a live AI
session, not a mini-app index. Navigation tools, including badge/QR changes,
still end the cloud session after the deferred action is committed. Local waiting
can resume afterward; retaining cloud conversation across hidden pages is not
implemented. Stale commands are not replayed on re-entry.

## Screen sleep and power

After 60 idle seconds in local wake, the backlight is zero, the panel sleeps and
LVGL stops. Microphone RX and inference continue with CPU performance and
no-light-sleep locks held. This mode cannot also stop the codec or run at 40 MHz.
Disabling background wake releases its audio resources and restores the prior
policy. Actual standby current and battery duration require instrument measurement.

## Models and customization

ESP-SR is pinned to **2.1.3**. Direct WakeNet9s receives 16 kHz mono signed 16-bit
PCM, 512 samples per frame. The packaged bundled model is **125,946 bytes**.
Partitions and all 726 sound clips are preserved.

A custom phrase requires a trained **WakeNet9s / ESP32-C3 / ESP-SR 2.1.3** compatible
model. Text editing does not change recognition. See [model import](../assets/xiaozhi/wake/README.md)
for build-time replacement. This is not a browser text setting, runtime upload or
model trainer. The requested phrase remains unspecified; no external training
request or paid order has been submitted.

The exact bundled weights use normal mode with threshold 0.67. Other models keep
their normal-mode default until separately calibrated. Metadata/size checks do
not prove neural compatibility; use trusted compatible assets and test on-device.

## Validation scope

The standalone detector measured approximately 28,448 bytes of runtime allocation
and 10.9 ms inference per 32 ms frame, excluding the rest of the application.
Seven fixed synthetic samples were decoded on the actual device. Three positive
voices and four negative phrases passed at 0.67. Lower thresholds falsely accepted
a similar phrase; 0.70 missed positives. This small set is a regression check,
not population accuracy or a false-trigger-rate measurement.

The opt-in `BADGE_WAKE_DEVICE_PROBE` checks real microphone frames, screen-off RX,
synthetic wake navigation/brightness, mini-app exclusion/resume, toggle persistence,
unchanged volume and repeated teardown. Cloud auto-connect is suppressed in that
build. It does not validate acoustic wake-to-cloud latency. Release builds must
disable all probes.

Host tests cover navigation, returns, volume bounds, model-byte preservation and
invalid metadata rejection. Native LVGL checks use the configured 32 KiB pool.
Actual microphone recognition across speakers/noise, custom models, standby current
and end-to-end wake conversation remain user/device acceptance work.

## Primary references

- [WakeNet support](https://docs.espressif.com/projects/esp-sr/en/latest/esp32/wake_word_engine/README.html)
- [Direct WakeNet example](https://github.com/espressif/esp-skainet/tree/master/examples/wake_word_detection)
- [Custom model process](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/wake_word_engine/ESP_Wake_Words_Customization.html)

## 2.6.42 startup recovery and resource handoff

Code review found that a detector exiting because of its memory gate or a
microphone error left the running flag and error state latched, preventing
future ticks from restarting it. The updated owner joins the completed task,
synchronously frees its 8 KiB stack, and retries after three seconds. Semaphore
and task creation failures use the same backoff without changing the saved
wake preference.

Before task creation, if free memory is below 64 KiB or the largest contiguous
block is below 40 KiB, navigation releases the idle cloud transport. Existing
reuse remains available without memory pressure. The detector still requires
48 KiB free and a 32 KiB largest block; model admission thresholds are not
lowered. New logs report the preference, memory gate, microphone failure and
detection without exposing microphone recordings. The bundled wake phrase
remains Ni Hao Xiao Zhi and its detection threshold is unchanged.

Host tests run the actual lifecycle against memory-gate, task-creation and
microphone-read failures, plus disable/join transitions, checking automatic
retry, resource recovery and preference preservation.

Device lifecycle checks passed: automatic recovery after one injected startup
failure, increasing real microphone frame counts, continued capture after 60
seconds of screen-off waiting, and restored brightness/navigation after an
injected trigger. Three sound-library playback/wake handoffs passed, and listening
resumed after leaving each of the four mini apps. All five enable/disable cycles
returned to 116,312 free bytes. Heap comparison waits for geolocation to finish
and pauses new requests to exclude its temporary network allocations; the first
unisolated check stopped and was rerun. These injected triggers do not establish
real spoken wake-word recognition.

A separate cloud regression completed five synthetic-voice turns with 303/303
audio packets decoded, a 22-second listening interval, and recovery from one
injected disconnect. The codec arena was allocated once; local listening resumed
after exit with volume preserved. The test uploaded no microphone recordings.
All probes are disabled in the production firmware.

After installing production 2.6.42, the user spoke the bundled wake phrase on
the home screen and confirmed wake-up and a normal reply. Serial diagnostics
recorded a real Wake detected event followed by Voice wake AI start: ESP_OK,
without a crash or allocation error. This single-user acceptance does not
measure recognition rates across speakers or noise conditions.

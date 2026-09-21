<p align="right"><a href="power-recovery.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Power recovery in 2.6.35

This release adapts the audio recovery and BSP lifetime protections from
[FoloToy/ai-passport 31759c4](https://github.com/FoloToy/ai-passport/tree/31759c4d63dd0d0d6580d6f74639ed5315bcd2c3).
It preserves this application's audio ownership, volume, display capture,
ten-row display buffer, background wake-word detector and deep-sleep button wake.

## Audio

- Control/data wrappers retain errors swallowed by codec-dev, including TX errors
  otherwise hidden by successful RX operations.
- Sleep deletes codec objects before applying and checking the full ES8311 sleep
  sequence, then stops I2S. Shared I2C and I2S resources remain allocated for wake.
- Wake and format changes rebuild the codec through public APIs, restore the
  saved format and volume, and revoke PCM access on failure. Failed opens return
  to stopped I2S and forced codec sleep. Releasing an uncertain codec reference
  fails closed until reboot.
- `bsp_audio_init()` retains the product contract: an initialized, sleeping codec
  is awakened. `bsp_audio_is_sleeping()` reports verified sleep, whereas
  `bsp_audio_needs_wake()` also covers partial open/suspend failures.
- A failed sleep remains an error and can be retried. Status flags used across
  tasks are atomic; lifecycle and PCM operations still require one joined owner.

## Audio ownership and retries

`badge_power_leave()` returns the real suspend error but releases ownership.
Workers signal completion without indefinitely waiting for broken hardware.
The shell retries unowned audio sleep at most once per second, with a mutex
covering both that check/operation and owner entry/exit. A new worker owns audio
before it initializes or resumes its stream. Main-thread setup remains serialized
with shell maintenance. Reminder chimes also use this ownership protocol.
Audio performance locks remain held until sleep has actually succeeded.

Only the shell changes screen state. If a failed panel suspend also fails to roll
back, the shell records the actual suspended state and retries waking it. Existing
brightness fallback remains. Power-management setup failures release acquired
locks so a later setup attempt can succeed.

## Resource lifetime and shutdown

Button cleanup retains ADC/calibration resources while any button deletion fails.
LVGL initialization retains a successfully initialized port for display retries;
partial port initialization requires reboot instead of racing asynchronous deinit.
Display, rounding callback and task-discovery timer registration occur under the
same lock; failed registration never publishes a partially ready display.

System shutdown, including the Xiaozhi shutdown tool, preserves the existing
CW2017 → ES8311 → I2S → shared I2C → LCD → deep-sleep ordering and now logs errors
from each pin/display shutdown stage. It still attempts remaining peripheral
stops after an individual failure. Failure to establish wake or display locking
retains the existing restart-to-recover behavior.

CW2017 mode/profile timing and sleep readback continue to match upstream.
Deep-sleep wake activates the gauge again; the transient `0.00%` emitted while
it rebuilds SOC is no longer accepted as a real reading. Firmware waits for a
valid SOC, retains the latest valid runtime value, and retries initialization
after a temporary failure. Normal screen-off does not suspend the gauge.
Wake-word listening keeps microphone/I2S processing active;
screen-off is not whole-device deep sleep. The board has no independent PA
control pin, so codec sleep is not a claim of physically removing amplifier power.

## Verification

Host tests exercise audio control/data faults, 64 format/sleep/wake cycles with
volume retention, failed release, ADC retention, LVGL initialization and wake
rollback, unowned retry and new-owner exclusion, PM initialization failures, wake
detector and reminder/display behavior. The existing broader host suite passes.

`BADGE_BSP_POWER_PROBE` is an opt-in boot test for twelve real codec/display sleep
cycles with 8/16 kHz format changes, silent playback, discarded microphone samples
and volume/brightness checks. It must be OFF in release artifacts. This test does
not transmit or save microphone data and does not measure current.

At preparation time the USB device was unavailable. Device testing and actual
current measurements remain pending; no battery-life improvement is quantified.
Partitions and persistent configuration formats are unchanged.

<p align="right"><a href="firmware-optimization.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Firmware size and conversation stability — 2.6.1

## Flash and RAM are different budgets

The 2.6.0 application occupied 2,869,952 bytes of a 2,883,584-byte factory
partition, leaving 13,632 bytes. That limit concerns stored firmware, not free
runtime heap. A nearly full application partition does not establish that a
conversation disconnected because of RAM exhaustion.

The application was still built with `CONFIG_COMPILER_OPTIMIZATION_DEBUG`.
Use `CONFIG_COMPILER_OPTIMIZATION_SIZE=y` for release builds. The initial size-only
build occupied 2,689,312 bytes, saving 180,640 bytes and leaving 194,272 bytes.
The final image also contains the connection diagnostics/recovery changes, so its
size must be read from `verify_firmware.py` rather than this intermediate result.
No partitions, image quality, font coverage, wake models or sound clips were removed.
Assertions and TLS certificate verification remain enabled.

Size optimization also exposed a compiler warning in radio display-name assembly.
The city prefix now reserves room for the frequency suffix and ends on a UTF-8
boundary before bounded copies; compiler warnings were not globally disabled.

## Unwanted exit and disconnect handling

2.6.0 considered 20 seconds of low PCM amplitude while listening to be inactivity.
When background wake was enabled, navigation could close an active conversation
on that basis. The new policy only returns paused or normally ended sessions to
local wake after 60 seconds. Listening/thinking/speaking and displayed errors do
not qualify. This is a confirmed code issue, not proof that every reported drop
had that cause.

Classified faults distinguish transport, timeout, allocation, message/PCM queue,
protocol, audio and a service `goodbye`. The first failure logs numeric state,
free heap, largest allocatable block and minimum heap; no credentials or transcript
are added. MQTT errors include numeric TLS/socket details. Intentional teardown
does not produce another failure. A service goodbye shows a normally ended
conversation instead of reporting every closure as a network fault.

An active conversation interrupted by transport/response timeout can retry at
1/2/4 seconds, at most three attempts. A completed answer resets the retry budget;
merely connecting does not. User exit stops recovery. Memory/protocol/audio/queue
errors are kept visible for explicit retry rather than repeatedly reconnecting.
Activation still requires user action. Reconnection starts a new session and
does not replay a partial utterance or pending device-control commands.

## Validation

Host tests exercise active silence, the paused/ended timeout, visible errors,
retry bounds and non-retryable faults. Existing navigation and resource gates
remain required. `BADGE_XIAOZHI_STABILITY_PROBE` uses a fixed synthetic greeting,
not microphone recordings. It exercises five cloud replies, a 22-second silent
listening interval and one injected transport fault followed by reconnection.
The probe records packet counts, heap, largest block and worker stack margin.
Always disable it in release firmware.

Finite bench runs cannot prove arbitrary Wi-Fi/server reliability, long-term heap
stability or acoustic performance. The user's original disconnection was not
captured by the initial passive log; do not label it a confirmed memory failure.

## Measured results

The release app is 2,689,072 bytes, leaving 194,512 bytes (about 190 KiB), a reduction of 180,880 bytes from 2.6.0.

Five device replies decoded 49/49, 36/36, 49/49, 62/62 and 49/49 packets. The conversation survived 22 seconds of silence; the first reconnect recovered the injected fault and replies continued. Reply-time free heap was 36,660–36,872 bytes, largest block 17,408 bytes, historical minimum at injection 11,276 bytes and worker stack margin 3,008 bytes. No actual allocation failure occurred; finite samples do not establish that all workloads are safe from memory pressure.

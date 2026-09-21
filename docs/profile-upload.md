<p align="right"><a href="profile-upload.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# Profile upload recovery — 2.6.24

Profile uploads use an inactive flash bank. After commit, the navigation task
must refresh all UI references before the former bank can be reused. Previously,
screen sleep skipped that refresh indefinitely, leaving subsequent uploads rejected
with `ESP_ERR_INVALID_STATE`, including otherwise valid QR uploads.

The navigation task now requests a wake when a committed revision is awaiting
display acknowledgement. Configuration writes also request activity through the
existing power owner; protocol tasks never invoke LVGL directly. The flash reuse
barrier remains enforced.

Protocol errors distinguish `display_pending`, `profile_changed`, and `upload_busy`.
USB and hotspot clients preserve these reasons. The configuration page retries only
`display_pending`, up to 15 times at 250 ms intervals. It does not replace a stale
base revision or retry another client's upload. Failed preparation preserves the
browser draft; an abort is sent only after this client successfully began an upload.
`info` additionally reports `profileReady` and `refreshPending` for diagnostics.

Regression tests: `tests/test_profile_upload.cjs`, `tests/test_badges_ui.cjs`, and
`tests/test_profile_store.c`. Device testing must include upload after display idle,
QR readback, an immediately following upload, and preservation of other profile data.

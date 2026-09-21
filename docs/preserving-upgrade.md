[简体中文](preserving-upgrade.zh_CN.md) · **English**

# Upgrade an existing badge while keeping its data

Use the separate preserving upgrade ZIP for an existing installation. The
community merged firmware is for a fresh installation: writing it at 0x0
overwrites NVS and additional badge slots, even without a full-chip erase.
Installing a new application cannot recover bytes the installer has erased.
No change to the community website installer is included in this package.

## Compatibility

The locally archived 2.6.7 image matches the original community submission hash.
Its application partition was 0x2C0000 bytes and voice data began at 0x2D0000.
The current application partition is 0x3C0000 bytes and voice data begins at
0x3D0000. NVS/PHY (0x9000–0x10000), four extra badges (0x690000–0x750000), and
the first badge (0x7D0000–0x800000) retain their addresses. Profile formats 1–4
remain readable. Preserving those regions retains profiles and their images,
QR codes, themes, active badge and persisted Wi-Fi/app preferences. New features
use their defaults; built-in sounds are replaced by the current catalog.

Only recognized partition layouts are accepted. Unknown layouts stop before
writing; do not bypass that check. These guarantees apply to this upgrade tool,
not an arbitrary third-party or community web installer.

## Install from the ZIP

1. Extract the entire ZIP into a local folder. Install Python 3.10 or newer.
2. In that folder run `python -m pip install -r requirements.txt` once.
3. Connect the badge by USB and disconnect it in any web configuration page.
4. On Windows double-click `upgrade-windows.cmd`, then enter the badge's COM
   port. Alternatively run `python tools/upgrade_badge.py --port COM6` (replace
   COM6 with the actual port; on Linux/macOS use the actual device path).
5. Keep USB connected until the tool reports successful verification.

The tool verifies package hashes, backs up the full 8 MiB flash privately, and
validates the old layout before writing. It writes and verifies only the
bootloader, application and bundled sound resources, then commits the partition
table last. It checks NVS/PHY and all badge storage byte-for-byte against the
backup before resetting. It never flashes a merged image or erases the chip.

The default backup folder on Windows is
`%LOCALAPPDATA%/AI-Passport/Backups/<timestamp>/`. On other systems it is
`~/AI-Passport/Backups/<timestamp>/`. Backups contain personal data and credentials:
keep them private. `--backup-dir` can choose another new private folder. Existing
backups are never overwritten. `--check-only` verifies the package without USB.

## Failure and recovery

If backup or compatibility checks fail, no write starts. If power is lost during
writing, the device may not boot; this single-application layout is not an atomic
or power-loss-proof update. Keep the complete `flash-before.bin` and its SHA-256
file. The backup can restore the previous firmware and data on the same badge.
Do not select a different device's backup or erase flash to troubleshoot.

From the extracted folder, after verifying the backup SHA-256 matches
`backup-sha256.txt`, a same-device full rollback is:

```text
python -m esptool --chip esp32c3 --port COM6 --after no_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 "PATH/flash-before.bin"
python -m esptool --chip esp32c3 --port COM6 --after hard_reset verify_flash 0x0 "PATH/flash-before.bin"
```

This is a deliberate full rollback. Use the actual port and original private
backup path. It replaces the entire device state with the saved state.

<p align="right">
  <a href="pipeline-guide.zh_CN.md">简体中文</a> · <strong>English</strong>
</p>

# AI Agent Guide: Version Update, Firmware Build, and Flashing Pipeline

This document defines the unified pipeline for firmware version bumping, compilation, 0x0 merged binary generation, layout verification, and device flashing.

## 1. Automated Pipeline Tool: `tools/pipeline.py`

The repository provides a unified script (`tools/pipeline.py`) that chains the entire workflow, supporting version bumping, building, flashing, and full **backup/restore** of user data.

### Usage Examples

1. **Standard Safe Flash (Fastest, preserves NVS & profiles in-place)**:
   ```bash
   python tools/pipeline.py --bump patch --flash
   ```

2. **Flash with explicit Backup & Restore (Full safety net)**:
   ```bash
   python tools/pipeline.py --bump patch --flash --backup --restore
   ```

3. **Backup only (Save Wi-Fi, profiles & QR codes to disk)**:
   ```bash
   python tools/pipeline.py --backup-only --backup-dir ./backups/my_backup
   ```

4. **Restore only (Write saved user data back to device)**:
   ```bash
   python tools/pipeline.py --restore-only --backup-dir ./backups/my_backup
   ```

5. **Build and package 0x0 binary without flashing**:
   ```bash
   python tools/pipeline.py --bump patch
   ```

## 2. Core Execution Stages

### Stage 1: Update Version
- Location: `main/badge_profile.h`
- Macro: `#define BADGE_VERSION "X.Y.Z"`

### Stage 2: Activate ESP-IDF 5.5.3 Environment
- Toolchain path: `D:\esp\esp-idf`
- Activation in Windows PowerShell:
  ```powershell
  [Environment]::SetEnvironmentVariable('MSYSTEM', '')
  Set-Location D:\esp\esp-idf
  . .\export.ps1
  Set-Location <repo_root>
  ```

### Stage 3: Build & Merge Binary (0x0)
- Compile application: `idf.py build`
- Merge full image: `idf.py merge-bin -o FoloToy-AI-Passport-full.bin`
- Verify firmware layout: `python tools/verify_firmware.py build`

### Stage 4: Flash Device (Preserve User Data vs Full Erase)

> **⚠️ Critical Safety Rule**:
> - **Incremental / Feature Update**: **DO NOT write 0x0 full image directly!** Writing at 0x0 overwrites `0x9000` (NVS, Wi-Fi credentials) and `0x690000`/`0x7d0000` (custom profiles & QR codes).
> - **Preserving Flash (Recommended)**: Only flash application image to `0x10000`. Finishes in seconds and preserves all user configuration and credentials.

- **Safe Flashing (Preserves all user data)**:
  ```bash
  esptool --port <PORT> --chip esp32c3 -b 460800 write-flash 0x10000 build/FoloToy-AI-Passport.bin
  ```
  Or via the pipeline script:
  ```bash
  python tools/pipeline.py --flash
  ```

- **Factory Reset Flash (Erases all data)**:
  Use only when initializing a blank chip or requested by the user:
  ```bash
  python tools/pipeline.py --erase-all
  ```

## 3. Troubleshooting

- **Port Disappears After Flashing**: Press the UP (wake) key or replug the USB cable to re-enumerate the USB CDC/JTAG serial port.
- **Compressed Size vs Flash Size**: `esptool` compresses the 8 MB flash payload during transport. The decompressed 8 MB image is written completely.

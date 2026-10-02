<p align="right">
  <strong>简体中文</strong> · <a href="pipeline-guide.md">English</a>
</p>

# AI Agent 指南：版本更新、固件编译与设备烧录自动化流程

本文档为 AI Agent 提供固件版本号更新、编译、打包验证及烧录的标准工作流。

## 1. 自动化流水线工具：`tools/pipeline.py`

仓库内置了专用的端到端流水线脚本 `tools/pipeline.py`，串联了版本递增、IDF 编译、0x0 全固件合并验证、**可选设备数据备份、安全刷写与数据恢复**。

### 常用命令示例

1. **常规安全刷写（最快，原地保留 NVS、Wi-Fi 凭据和工牌数据）**：
   ```bash
   python tools/pipeline.py --bump patch --flash
   ```

2. **双保险刷写（先自动备份数据 ➔ 刷入固件 ➔ 自动恢复并校验数据）**：
   ```bash
   python tools/pipeline.py --bump patch --flash --backup --restore
   ```

3. **仅执行数据备份（将设备上的 Wi-Fi、工牌、二维码保存到本地目录）**：
   ```bash
   python tools/pipeline.py --backup-only --backup-dir ./backups/my_backup
   ```

4. **仅执行数据恢复（将本地保存的资料写回设备）**：
   ```bash
   python tools/pipeline.py --restore-only --backup-dir ./backups/my_backup
   ```

5. **仅编译打包（生成 0x0 完整镜像，不烧录）**：
   ```bash
   python tools/pipeline.py --bump patch
   ```

## 2. 核心执行阶段

### 阶段 1：更新固件版本号
- 位置：`main/badge_profile.h`
- 宏定义：`#define BADGE_VERSION "X.Y.Z"`

### 阶段 2：激活 ESP-IDF 5.5.3 环境
- 工具链路径：`D:\esp\esp-idf`
- Windows PowerShell 环境激活：
  ```powershell
  [Environment]::SetEnvironmentVariable('MSYSTEM', '')
  Set-Location D:\esp\esp-idf
  . .\export.ps1
  Set-Location <项目根目录>
  ```

### 阶段 3：构建与 0x0 合并固件打包
- 编译应用：`idf.py build`
- 合并完整固件：`idf.py merge-bin -o FoloToy-AI-Passport-full.bin`
- 校验固件布局：`python tools/verify_firmware.py build`

### 阶段 4：烧录写入设备（严格区分数据保护与全刷）

> **⚠️ 核心安全原则**：
> - **日常开发 / 功能升级**：**严禁直接刷写 0x0 全镜像**！0x0 写入会擦除 `0x9000`（NVS 配置、Wi-Fi 账号密码）和 `0x690000`/`0x7d0000`（工牌个人资料、二维码）。
> - **增量保护刷写（推荐）**：只将应用固件写入 `0x10000`，几秒完成，不碰任何用户数据。

- **安全刷写命令（保留所有数据）**：
  ```bash
  esptool --port <PORT> --chip esp32c3 -b 460800 write-flash 0x10000 build/FoloToy-AI-Passport.bin
  ```
  或者使用流水线：
  ```bash
  python tools/pipeline.py --flash
  ```

- **全盘出厂恢复刷写（清除所有数据）**：
  仅当设备为空片或用户明确要求重置时使用：
  ```bash
  python tools/pipeline.py --erase-all
  ```

## 3. 常见问题排查

- **烧录后串口端口消失**：按一下设备正面的 UP/唤醒按键，或重新插拔 USB 数据线以重新枚举串口。
- **压缩大小与实际 Flash 大小**：`esptool` 在串口传输过程中采用了压缩算法，片上解压后物理写入的依然是完整的 8MB 空间。

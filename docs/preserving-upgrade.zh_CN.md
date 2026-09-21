**简体中文** · [English](preserving-upgrade.md)

# 老用户保留数据升级

已有资料的工牌请使用独立的“保留数据升级包”。社区整包固件适用于全新安装：
即使没有勾选整片擦除，从 0x0 写入整包也会覆盖系统设置和额外工牌存储区。
新固件启动后无法找回已经被安装器擦掉的数据。本升级包没有修改社区网站安装器。

## 兼容范围

本地归档的 2.6.7 固件与当时社区提交记录的哈希一致。旧应用分区为 0x2C0000
字节，音效起点为 0x2D0000；当前应用分区为 0x3C0000 字节，音效起点改为
0x3D0000。以下用户数据区域位置没有变化：系统设置及校准 0x9000–0x10000，
四张额外工牌 0x690000–0x750000，首张工牌 0x7D0000–0x800000。
新版仍可读取 1–4 版工牌资料格式。

保留这些区域，就能保留工牌文字、图片、二维码、主题、当前工牌、Wi-Fi 及已保存的
应用偏好。新增功能采用默认值；内置音效会更新为新版目录。

工具只接受已识别的分区布局。发现未知布局会在写入前停止，不要绕过检查。
此保留数据流程适用于本升级工具，不代表任意第三方或社区网页安装器都能保留资料。

## 使用升级包

1. 完整解压 ZIP，安装 Python 3.10 或更新版本。
2. 在解压目录打开终端，首次执行：`python -m pip install -r requirements.txt`。
3. USB 连接工牌，在配置网页中点击“断开连接”，释放串口。
4. Windows 双击 `upgrade-windows.cmd`，按提示输入工牌实际 COM 端口。
   也可以执行 `python tools/upgrade_badge.py --port COM6`，把 COM6 换成实际端口。
   Linux/macOS 使用对应设备路径。
5. 保持 USB 连接，直到提示校验成功。

流程：校验升级包 → 完整备份 8 MiB Flash → 识别旧分区 → 分段更新启动程序、应用
和音效并校验 → 最后写入分区表 → 将系统设置及全部工牌区域与备份逐字节校验 → 重启。
工具不会刷写整包固件，也不会擦除整片 Flash。

Windows 默认备份到 `%LOCALAPPDATA%/AI-Passport/Backups/<时间>/`，其他系统默认
备份到 `~/AI-Passport/Backups/<时间>/`。备份含个人资料和凭据，请私下保存。
可用 `--backup-dir` 指定另一个新的私有目录；不会覆盖已有备份。
`--check-only` 只检查升级包，不连接设备。

## 失败与恢复

备份失败或布局不兼容时，不开始写入。写入中断电可能导致暂时无法启动；当前只有
一个应用分区，不是断电仍可自动回退的升级方案。请保留完整 `flash-before.bin`
和 SHA-256 文件，可在同一块工牌上恢复原固件和资料。不要混用其他人的备份，
也不要为了排错清空 Flash。

先确认备份文件 SHA-256 与 `backup-sha256.txt` 一致。确需整机回退时，在升级包目录执行：

```text
python -m esptool --chip esp32c3 --port COM6 --after no_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 8MB 0x0 "备份路径/flash-before.bin"
python -m esptool --chip esp32c3 --port COM6 --after hard_reset verify_flash 0x0 "备份路径/flash-before.bin"
```

这里必须使用实际端口和这块工牌原先的私有备份路径。此操作会将整机状态恢复到备份时。

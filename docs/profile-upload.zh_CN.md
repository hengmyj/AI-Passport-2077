<p align="right"><strong>简体中文</strong> · <a href="profile-upload.md">English</a></p>

# 工牌上传恢复 — 2.6.24

工牌资料上传到备用 Flash 存储区。提交后，导航任务必须更新全部界面引用，
才能再次使用旧存储区。旧流程在息屏时跳过这一步，导致后续上传一直返回
`ESP_ERR_INVALID_STATE`，有效的二维码上传也会被拒绝。

现在，已提交资料尚未获得界面确认时，导航任务会请求唤醒屏幕。配置写入也会
通过原有电源管理任务请求保持唤醒；协议任务不直接操作 LVGL。存储区安全复用
检查继续保留。

协议错误区分 `display_pending`（等待界面刷新）、`profile_changed`（资料版本
冲突）和 `upload_busy`（其他配置端正在上传）。USB 与热点配置页均保留原因。
页面仅对等待刷新自动重试，间隔 250 毫秒，最多重试 15 次；不会替换旧版本号，
也不会重试其他配置端的上传。准备失败时保留浏览器中的编辑，只在本页面成功
开始上传后才发送中止请求。`info` 新增 `profileReady` 和 `refreshPending` 诊断字段。

回归测试：`tests/test_profile_upload.cjs`、`tests/test_badges_ui.cjs`、
`tests/test_profile_store.c`。实机测试应覆盖息屏后上传、二维码回读、立即再次上传，
以及其他工牌资料保持不变。

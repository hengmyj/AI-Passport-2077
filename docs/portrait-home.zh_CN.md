<p align="right"><strong>简体中文</strong> · <a href="portrait-home.md">English</a></p>

# 竖版肖像首页（2.3.0）

240 × 320 首页采用居中的 112 × 136 肖像、紧凑品牌区、最多两行的通栏姓名、
并排部门与职位、编号和签名。每张工牌继续使用自己的配色和自定义图标；
共用状态栏及底部四项按键提示保留原有功能。

USB／热点离线配置页内置 Orbitron 700 拉丁标题字体和 Caveat 600 拉丁签名字体，
中文使用浏览器可用的中文字体。网页把文字栅格化为 RGB565 再上传，硬件无需
保存庞大的多语言字库。过长资料显示省略号；姓名优先换行，最多两行。

## 存储与兼容

V3 数据依次为：信息区（208 × 76，31,616 字节）、肖像（112 × 136，30,464）、
品牌区（148 × 32，9,472）、图标（32 × 32，2,048）、二维码（192 × 192，
每像素一位，4,608）。合计 78,208 字节。分区位置及每个 0x18000 字节的双存储区
保持不变，最后写入头部才完成提交。继续支持读写 V1／V2。
通信协议仍为 2，通过 `maxProfileVersion: 3` 声明支持的新格式；返回的尺寸对应
所请求工牌的实际存储格式，新上传明确指定格式 3。

旧工牌可直接使用兼容裁剪显示，在新版配置页读取并保存后，该工牌会升级格式。
旧版 72 × 88 头像会重采样，重新上传原始照片可获得新尺寸的清晰度。
原来的完整 Flash 备份仍可用于回退。仅更新程序分区会保留 NVS、工牌和音效分区。

## 字体来源

- [Orbitron](https://github.com/google/fonts/tree/main/ofl/orbitron)，SIL OFL，
  文件为 `assets/fonts/Orbitron.ttf` 与 `Orbitron-OFL.txt`。
- [Caveat](https://github.com/google/fonts/tree/main/ofl/caveat)，SIL OFL，
  文件为 `assets/fonts/Caveat.ttf` 与 `Caveat-OFL.txt`。

`*-latin.woff2` 子集保留 U+0020–00FF、U+2010–2027，使用 fontTools 命令
`pyftsubset INPUT --unicodes=U+0020-00FF,U+2010-2027 --flavor=woff2 --output-file=OUTPUT`
生成。`tools/build_config_page.py` 将其以内联数据嵌入配置页，配置时无需互联网。

## 验证

存储测试覆盖 V2／V3 混用、全部素材偏移及边界、上传或提交中断、相邻工牌保持不变。
浏览器检查覆盖字体、数据包长度、资料及二维码保持不变。实际 LVGL 绘制代码在
32 KiB 内存池中完成 200 次首页／列表切换，原生渲染器记录的最大占用为 18,128 字节。

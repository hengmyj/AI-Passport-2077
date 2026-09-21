<p align="right"><strong>简体中文</strong> · <a href="README.md">English</a></p>

# WakeNet9s 模型资源

`wn9s_nihaoxiaozhi` 来自乐鑫 ESP-SR **2.1.3**，注册表记录的提交为
`53ff709c3cf65fcd21291c38bad5914c9639b2ae`，识别「你好小智」。三个模型及元数据文件
保持原样，原 Espressif MIT 许可证保留在 `LICENSE`，限定用于乐鑫产品。

固件固定 ESP-SR 2.1.3。`tools/build_wake_model.py` 将一个模型打包进应用固件，并从
模型元数据读取实际显示的唤醒词。因此默认模型无需更改分区。

## 自定义词模型

先取得兼容 **WakeNet9s / ESP32-C3 / ESP-SR 2.1.3** 的训练模型。
任意文本、WakeNet9/S3 模型和 MultiNet 资源不能互换。将模型来源及许可证随资源保存，
把 `_MODEL_INFO_`、`wn9_data`、`wn9_index` 放入名为 `wn9s_<模型名>` 的目录，再配置：

```sh
idf.py -D BADGE_WAKE_MODEL_DIR=/absolute/path/wn9s_custom build
```

安装前验证固件容量，并用正确唤醒词及相似的错误词进行实机测试。元数据检查不能证明
神经网络兼容性，只接入来源可信的训练模型。不能把默认模型的元数据改名，冒充它已能
识别新词。选择自定义模型会替换默认模型，不会同时运行两套检测器。
当前更换词模型需要重新编译固件，不支持只修改文字或在网页上传。

可参考[官方定制流程](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/wake_word_engine/ESP_Wake_Words_Customization.html)
及[社区 TTS 模型申请](https://github.com/espressif/esp-sr/issues/88)。
尚未提交自定义训练申请，也未订购任何付费服务。

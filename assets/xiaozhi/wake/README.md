<p align="right"><a href="README.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# WakeNet9s model asset

The `wn9s_nihaoxiaozhi` directory comes from Espressif ESP-SR **2.1.3**, registry
commit `53ff709c3cf65fcd21291c38bad5914c9639b2ae`. It recognizes Ni Hao Xiao Zhi.
The three coefficient/metadata files are unmodified. The upstream Espressif MIT
license is retained in `LICENSE` and restricts use to Espressif products.

The firmware pins ESP-SR 2.1.3. `tools/build_wake_model.py` packages one model into
the app image and derives the displayed phrase from its metadata. The firmware
therefore needs no partition changes for this default model.

## Custom trained phrases

Obtain a trained **WakeNet9s / ESP32-C3 / ESP-SR 2.1.3 compatible** model first.
An arbitrary text string, WakeNet9/S3 model or MultiNet asset is not interchangeable.
Keep its license and provenance with the asset. Put `_MODEL_INFO_`, `wn9_data` and
`wn9_index` under a directory named `wn9s_<model-name>` and configure:

```sh
idf.py -D BADGE_WAKE_MODEL_DIR=/absolute/path/wn9s_custom build
```

Validate the resulting image size and test positive and confusable negative speech
on the device before installing it. Metadata checks do not prove neural-network
compatibility; only use a trusted trained model. Never rename metadata to pretend
the default model recognizes a different phrase. Selecting a custom model replaces
the default; it does not run two detectors simultaneously. Phrase changes currently
require a firmware rebuild, not a text edit or browser upload.

[Official customization process](https://docs.espressif.com/projects/esp-sr/en/latest/esp32s3/wake_word_engine/ESP_Wake_Words_Customization.html)
and [community TTS-trained model requests](https://github.com/espressif/esp-sr/issues/88).
No custom training request has been submitted and no paid service has been ordered.
